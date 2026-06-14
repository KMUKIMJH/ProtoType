#include "StageManager.h"
#include "MapLoader.h"
#include "Tutorial.h"
#include "StageFactory.h"
#include "StageForest.h"
#include "StageCity.h"

#include "StageFactoryBoss.h"
#include "StageForestBoss.h"
#include "StageCityBoss.h"
#include "StageShop.h"
#include "../Game/FactoryBoss.h"
#include "../Game/Enemy.h"
#include "../Game/MeleeEnemy.h"
#include "../Game/ShieldEnemy.h"
#include "../Game/RangedEnemy.h"
#include "../Game/Scrap.h"
#include "../Game/StoryItem.h"
#include "../Game/States.h"
#include "../Game/Setting.h"
#include "../Game/Gravity.h"
#include "../Game/Platform.h"
#include "../Game/KeyBinding.h"
#include "../Engine/Engine.h"
#include "../Engine/ShowCollision.h"
#include "../Engine/Camera.h"
#include "../Engine/GameObjectManager.h"
#include "../Engine/logger.h"
#include "../Game/Particles.h"
#include "../Game/TwoWayPlatform.h"
#include "../Game/Zipline.h"
#include <algorithm>
#include <cstdio>
#include <GL/glew.h>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <random>
#include <iostream>
#include <cctype>
#include <set>
#include <utility>
#include <memory>

bool StageManager::isTransitioningToSettings = false;

static std::set<std::pair<int, int>> s_unlockedBackdoorCoords;
static std::vector<CS230::Backdoor*> s_menuBackdoors;

static std::vector<std::shared_ptr<CS230::Texture>> s_cachedMenuTextures;
static std::shared_ptr<CS230::Texture> s_cachedMenuTitle;
static int s_lastMenuSelection = -1;
static CS230::Backdoor* s_lastActiveBackdoor = nullptr;



static std::shared_ptr<CS230::Texture> s_backdoorImageTex;
static int s_lastBackdoorImageStage = -1;

namespace
{
    Math::ivec2 StageViewportOffset(int worldW, int worldH)
    {
        int offX = (worldW < 1920) ? (1920 - worldW) / 2 : 0;
        int offY = (worldH < 1080) ? (1080 - worldH) / 2 : 0;
        return { offX, offY };
    }
}

StageManager::StageManager() : timer_texture(nullptr), backgroundTex(nullptr)
{}

void StageManager::InitStage(int width, int height)
{
    worldWidthCur = width;
    worldHeightCur = height;
    hasPlayerSpawnOverride = false;
    playerSpawnOverride = { 0.0, 0.0 };

    if (generator)
    {
        delete generator;
        generator = nullptr;
    }

    for (auto* obj : staticObjects)
    {
        delete obj;
    }
    staticObjects.clear();
    backdoors.clear();

    for (auto& entity : enemies)
    {
        delete entity.go;
    }
    enemies.clear();

    for (auto* item : items)
    {
        delete item;
    }
    items.clear();

    texts.clear();

    if (leftPortal)
    {
        delete leftPortal;
        leftPortal = nullptr;
    }
    if (rightPortal)
    {
        delete rightPortal;
        rightPortal = nullptr;
    }

    handlers = {};
    backgroundTex = nullptr;
    isBackdoorMenuOpen = false;
}

Platform* StageManager::AddPlatform(Math::irect rect)
{
    auto* p = new Platform(rect);
    staticObjects.push_back(p);
    return p;
}

void StageManager::AddText(std::string text, Math::vec2 position, unsigned int color, double scale)
{
    auto tex = Engine::GetFont(0).PrintToTextureScaled(text, color, scale);
    texts.push_back({ tex, position });
}

ProtoPortal* StageManager::AddPortal(Math::ivec2 bottomLeft)
{
    const double w = CS230::Enemy::PortalWidth;
    const double h = CS230::Enemy::PortalHeight;
    Math::rect r{ { static_cast<double>(bottomLeft.x), static_cast<double>(bottomLeft.y) }, { static_cast<double>(bottomLeft.x) + w, static_cast<double>(bottomLeft.y) + h } };
    auto* p = new ProtoPortal(r, stageIndex);

    if (bottomLeft.x < worldWidthCur / 2)
    {
        if (!leftPortal)
        {
            leftPortal = p;
        }
        else
        {
            staticObjects.push_back(p);
        }
    }
    else
    {
        if (!rightPortal)
        {
            rightPortal = p;
        }
        else
        {
            staticObjects.push_back(p);
        }
    }
    return p;
}

CS230::GameObject* StageManager::AddEnemy(Math::vec2 pos, EnemyType type, bool dropsStoryItem)
{
    Math::vec2 spawnPos = pos;
    CS230::Enemy* logic = nullptr;
    CS230::GameObject* go = nullptr;

    switch (type)
    {
    case EnemyType::Melee:
    {
        auto* e = new CS230::MeleeEnemy();
        e->SetDropsStoryItem(dropsStoryItem);
        e->Load(spawnPos);
        logic = e;
        go = e;
        break;
    }
    case EnemyType::Shield:
    {
        auto* e = new CS230::ShieldEnemy();
        e->SetDropsStoryItem(dropsStoryItem);
        e->Load(spawnPos);
        logic = e;
        go = e;
        break;
    }
    case EnemyType::Ranged:
    default:
    {
        auto* e = new CS230::RangedEnemy();
        e->SetDropsStoryItem(dropsStoryItem);
        e->Load(spawnPos);
        logic = e;
        go = e;
        break;
    }
    }

    GameEntity ge;
    ge.go = go;
    ge.logic = logic;
    ge.updateFunc = [logic](const CS230::Player& p, double dt, bool s)
        {
            if (!logic)
            {
                return;
            }
            if (auto* m = dynamic_cast<CS230::MeleeEnemy*>(logic))
            {
                m->Update(p, dt, s);
            }
            else if (auto* sh = dynamic_cast<CS230::ShieldEnemy*>(logic))
            {
                sh->Update(p, dt, s);
            }
            else if (auto* rg = dynamic_cast<CS230::RangedEnemy*>(logic))
            {
                rg->Update(p, dt, s);
            }
        };

    enemies.push_back(ge);
    return go;
}

void StageManager::SetPortals(ProtoPortal* left, ProtoPortal* right)
{
    leftPortal = left;
    rightPortal = right;
}

void StageManager::NotifyEnemyKilled()
{
    if (currentSlotLoaded >= 0 && currentSlotLoaded < CS230::RunProgressManager::StageSlotCount)
    {
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            if (enemies[i].logic && enemies[i].logic->GetHealth() <= 0)
            {
                if (i < runProgress.enemyAliveStatus[currentSlotLoaded].size())
                {
                    runProgress.enemyAliveStatus[currentSlotLoaded][i] = false;
                }
            }
        }
    }
}

void StageManager::SetBackdoorUIOffset(int bdIndex, Math::vec2 offset)
{
    for (auto* bd : backdoors)
    {
        if (bd->GetIndex() == bdIndex)
        {
            bd->SetUIOffset(offset);
            break;
        }
    }
}

void StageManager::Reset()
{
    int slot;
    if (pendingSlot >= 0)
    {
        slot = pendingSlot;

        auto getThemeBySlot = [](int s) {
            if (s <= 0) return -1;
            return (s - 1) / 3;
            };

        if (getThemeBySlot(currentSlotLoaded) != getThemeBySlot(slot))
        {
            player.ResetStoryFragments();
            player.ClearStoryUI();
        }

        stageIndex = runProgress.GetStageConfigIdForSlot(slot);
        pendingSlot = -1;
    }
    else
    {
        slot = runProgress.GetSlotForStageIndex(stageIndex);
    }

    currentSlotLoaded = slot;
    BuildStage(slot);

    if (runProgress.enemyAliveStatus[slot].empty())
    {
        runProgress.enemyAliveStatus[slot].assign(enemies.size(), true);
    }
    else
    {
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            if (i < runProgress.enemyAliveStatus[slot].size() && !runProgress.enemyAliveStatus[slot][i])
            {
                delete enemies[i].go;
                enemies[i].go = nullptr;
                enemies[i].logic = nullptr;
                enemies[i].updateFunc = nullptr;
            }
        }
    }

    ApplyPortalTargetsForSlot(slot);

    bool isBossStage = (stageIndex == 3 || stageIndex == 4 || stageIndex == 5);
    int currentTheme = -1;
    if (stageIndex == 3) currentTheme = 0;
    else if (stageIndex == 4) currentTheme = 1;
    else if (stageIndex == 5) currentTheme = 2;

    player.combat.showMissingFragmentsUI = false;

    if (runProgress.IsStageCleared(slot))
    {
        ClearEnemyInstances();
        if (rightPortal) rightPortal->SetEnabled(true);
        if (leftPortal) leftPortal->SetEnabled(true);

        if (isBossStage)
        {
            if (player.GetStoryFragments() >= 3)
            {
                if (!(player.combat.viewedBossStories & (1 << currentTheme)))
                {
                    player.combat.showStoryUI = true;
                    player.combat.currentStoryId = currentTheme;
                    player.combat.viewedBossStories |= (1 << currentTheme);
                }
            }
            else
            {
                player.combat.showMissingFragmentsUI = true;
            }
        }
    }
    else
    {
        if (isBossStage)
        {
            if (rightPortal) rightPortal->SetEnabled(false);
            if (leftPortal) leftPortal->SetEnabled(false);
        }
        else
        {
            if (rightPortal) rightPortal->SetEnabled(true);
        }
    }

    double groundTop = 0.0;
    for (auto* obj : staticObjects)
    {
        if (obj->Type() == GameObjectTypes::Platform)
        {
            groundTop = obj->GetGOComponent<CS230::RectCollision>()->WorldBoundary().Top();
            break;
        }
    }

    const Math::ivec2 off = StageViewportOffset(worldWidthCur, worldHeightCur);
    Math::vec2 defaultSpawn{ 150.0 + off.x, groundTop + off.y };
    Math::vec2 spawnPos = defaultSpawn;

    switch (pendingRespawn)
    {
    case RespawnMode::ExitPortal:
        spawnPos = PortalSpawnBottomLeft(rightPortal, defaultSpawn);
        break;
    case RespawnMode::MidBossRoom:
        spawnPos = MidBossRespawnPoint(groundTop, off);
        break;
    case RespawnMode::EntryPortal:
    case RespawnMode::None:
    default:
        spawnPos = PortalSpawnBottomLeft(leftPortal, defaultSpawn);
        break;
    }

    if (hasPlayerSpawnOverride && pendingRespawn != RespawnMode::ExitPortal && pendingRespawn != RespawnMode::MidBossRoom)
    {
        spawnPos = playerSpawnOverride;
    }

    player.SetWeaponLocked(false);
    player.Load(player, spawnPos);
    player.SetBottomTo(spawnPos.y);

    pendingRespawn = RespawnMode::None;
    slowMotionTimerSec = 0.0;
    ignoreFloorTimer = 0;
    ignoreFloorTarget = nullptr;
    isBackdoorMenuOpen = false;

    ui.ResetGameOver();
    ui.SetPaused(false);

    if (auto cam = GetGSComponent<CS230::Camera>())
    {
        cam->Update(player.Center());
    }
}

void StageManager::Load()
{
    if (isTransitioningToSettings)
    {
        isTransitioningToSettings = false;
        return;
    }

    s_unlockedBackdoorCoords.clear();

    AddGSComponent(new CS230::GameObjectManager());
    AddGSComponent(new Gravity(2400.0));
#ifdef _DEBUG
    AddGSComponent(new CS230::ShowCollision());
#endif

    const double marginX = 1920.0 * 0.25;
    const double marginY = 1080.0 * 0.25;
    Math::rect followZone{ { marginX, marginY }, { 1920.0 - marginX, 1080.0 - marginY } };
    AddGSComponent(new CS230::Camera(followZone));
    AddGSComponent(new CS230::ParticleManager<Particles::Effect1>());

    runProgress.ResetRun();
    stageIndex = runProgress.GetStageConfigIdForSlot(0);

    player.combat = CS230::PlayerCombat();
    player.move = CS230::PlayerMovement();

    player.combat.health = 5;
    player.combat.maxHealth = 5;
    player.combat.scrap = 0;
    player.combat.savedAmmo = 5;
    player.SetWeapon(nullptr);

    Reset();
}

void StageManager::BuildStage(int index)
{
    if (handlers.Unload) handlers.Unload(*this);

    const int configId = runProgress.GetStageConfigIdForSlot(index);
    const char* configNames[] = {
        "StageFactory", "StageForest", "StageCity",
        "StageFactoryBoss", "StageForestBoss", "StageCityBoss",
        "StageShop", "StageTutorial"
    };

    const char* cfgName = (configId >= 0 && configId <= 7) ? configNames[configId] : "Unknown";

    std::ostringstream oss;
    oss << "[StageManager] BuildStage: slot=" << index << ", configId=" << configId << " (" << cfgName << ")";
    Engine::GetLogger().LogEvent(oss.str());

    switch (configId)
    {
    case 0: { StageFactory s; s.Setup(*this); break; }
    case 1: { StageForest s; s.Setup(*this); break; }
    case 2: { StageCity s; s.Setup(*this); break; }
    case 3: { StageFactoryBoss s; s.Setup(*this); break; }
    case 4: { StageForestBoss s; s.Setup(*this); break; }
    case 5: { StageCityBoss s; s.Setup(*this); break; }
    case 6: { StageShop s; s.Setup(*this); break; }
    case 7: { StageTutorial s; s.Setup(*this); break; }
    default: { StageTutorial s; s.Setup(*this); break; }
    }

    std::string newBGM;
    if (configId == 0 || configId == 1 || configId == 2 || configId == 7) newBGM = "Assets/Sound/BGM/nofireland.wav";
    else if (configId == 3 || configId == 4 || configId == 5) newBGM = "Assets/Sound/BGM/fireland_break.wav";
    else if (configId == 6) Engine::GetSFXManager().StopBGM();
    else newBGM = currentBGM;

    if (!newBGM.empty() && currentBGM != newBGM)
    {
        Engine::GetSFXManager().StopBGM();
        Engine::GetSFXManager().PlayBGM(newBGM, 1.0f, 80);
        currentBGM = newBGM;
    }
    else if (currentBGM.empty() && !newBGM.empty())
    {
        Engine::GetSFXManager().PlayBGM(newBGM, 1.0f, 80);
        currentBGM = newBGM;
    }

    if (auto cam = GetGSComponent<CS230::Camera>())
    {
        cam->SetZoom(1.3);
        int viewW = static_cast<int>(1920.0 / cam->GetZoom());
        int viewH = static_cast<int>(1080.0 / cam->GetZoom());
        int limW = std::max(0, static_cast<int>(worldWidthCur - viewW));
        int limH = std::max(0, static_cast<int>(worldHeightCur - viewH));

        cam->SetLimit(Math::irect{ { 0, 0 }, { limW, limH } });
        std::string bgFileName = std::string(cfgName) + ".png";
        backgroundTex = Engine::GetTextureManager().Load("Assets/Image/Stage/" + bgFileName);
    }
}

void StageManager::DrawSolids(const Math::TransformationMatrix& camM) const
{
    for (auto* obj : staticObjects)
    {
        if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
        {
            auto* rc = obj->GetGOComponent<CS230::RectCollision>();
            if (rc)
            {
                Math::rect r = rc->WorldBoundary();
                Math::vec2 sz = r.Size();
                Math::vec2 c = { r.Left() + sz.x * 0.5, r.Bottom() + sz.y * 0.5 };
                GAME200::RGBA color = (obj->Type() == GameObjectTypes::Platform) ? GAME200::LIGHTGRAY : GAME200::SKYBLUE;
                ::Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(c) * Math::ScaleMatrix(sz), color, color, 1.0);
            }
        }
        else
        {
            obj->Draw(camM);
        }
    }
}

void StageManager::Update(double dt)
{
    if (generator && generator->IsUIOpen())
    {
        generator->UpdateUI(&player);
        return;
    }

    if (player.ShouldShowStoryUI() || ui.IsGameOver() || ui.IsPaused() || ui.IsEnding())
    {
        ui.Update(this, player);
        return;
    }

    if (isBackdoorMenuOpen)
    {
        if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::W))
        {
            backdoorMenuSelection--;
            if (backdoorMenuSelection < 0) backdoorMenuSelection = static_cast<int>(s_menuBackdoors.size()) - 1;
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        }
        else if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::S))
        {
            backdoorMenuSelection++;
            if (backdoorMenuSelection >= static_cast<int>(s_menuBackdoors.size())) backdoorMenuSelection = 0;
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        }
        else if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter) || Engine::GetInput().KeyJustPressed(CS230::Input::Keys::E))
        {
            if (!s_menuBackdoors.empty())
            {
                CS230::Backdoor* target = s_menuBackdoors[backdoorMenuSelection];
                if (target != activeBackdoor)
                {
                    player.TeleportBottomLeft(target->GetSpawnPosition());
                    player.move.velocity = { 0.0, 0.0 };
                    Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/portal.wav");
                }
            }
            isBackdoorMenuOpen = false;
        }
        else if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape))
        {
            isBackdoorMenuOpen = false;
        }
        return;
    }

    if (ui.IsGameOver() || ui.IsPaused())
    {
        ui.Update(this, player);
        return;
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape))
    {
        ui.SetPaused(true);
        return;
    }

    double realDt = dt;
    player.combat.runTimer += realDt;

    if (player.ConsumeInvincibleHitTrigger())
    {
        freezeTimerSec = 0.3;
    }

    if (freezeTimerSec > 0.0)
    {
        freezeTimerSec -= realDt;
        if (freezeTimerSec < 0.0) freezeTimerSec = 0.0;
        dt *= 0.2;
    }

    UpdateGSComponents(dt);

    if (auto gom = GetGSComponent<CS230::GameObjectManager>()) gom->UpdateAll(dt);

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::R))
    {
        pendingRespawn = RespawnMode::EntryPortal;
        Reset();
        return;
    }

#ifdef _DEBUG
    auto tryWarpToConfig = [&](CS230::Input::Keys digitKey, int targetConfigId)
        {
            if (Engine::GetInput().KeyJustPressed(digitKey))
            {
                for (int i = 0; i < CS230::RunProgressManager::StageSlotCount; ++i)
                {
                    if (runProgress.GetStageConfigIdForSlot(i) == targetConfigId)
                    {
                        stageIndex = targetConfigId;
                        pendingRespawn = RespawnMode::EntryPortal;
                        Reset();
                        return true;
                    }
                }
            }
            return false;
        };

    if (tryWarpToConfig(CS230::Input::Keys::Digit1, 0) || tryWarpToConfig(CS230::Input::Keys::Digit2, 1) || tryWarpToConfig(CS230::Input::Keys::Digit3, 2) || tryWarpToConfig(CS230::Input::Keys::Digit4, 3) || tryWarpToConfig(CS230::Input::Keys::Digit5, 4) || tryWarpToConfig(CS230::Input::Keys::Digit6, 5) || tryWarpToConfig(CS230::Input::Keys::Digit7, 6) || tryWarpToConfig(CS230::Input::Keys::Digit8, 7))
        return;
#endif

    if (slowMotionTimerSec > 0.0)
    {
        slowMotionTimerSec -= dt;
        if (slowMotionTimerSec < 0.0) slowMotionTimerSec = 0.0;
        int hits = player.ConsumeEnemyHitEvents();
        if (hits > 0) slowMotionTimerSec += hits * 1.0;
    }

    bool hitStopActive = (slowMotionTimerSec > 0.0);
    player.SetSlowMotionActive(hitStopActive);
    player.SetSpeedMultiplier(player.IsSlowMoActive() ? 1.5 : 1.0);

    if (player.GetGroggyTimer() > 0) player.DecGroggyTimer(dt);

    if (ignoreFloorTimer > 0 && --ignoreFloorTimer == 0) ignoreFloorTarget = nullptr;

    Math::rect groundRect = { { 0, 0 }, { 0, 0 } };
    for (auto* obj : staticObjects)
    {
        if (obj->Type() == GameObjectTypes::Platform)
        {
            groundRect = obj->GetGOComponent<CS230::RectCollision>()->WorldBoundary();
            break;
        }
    }

    player.Update(groundRect, worldWidthCur, worldHeightCur, dt);

    if (leftPortal) leftPortal->Update(dt);
    if (rightPortal) rightPortal->Update(dt);

    for (auto* obj : staticObjects)
    {
        if (auto* p = dynamic_cast<ProtoPortal*>(obj)) p->Update(dt);
        if (auto* b = dynamic_cast<CS230::Backdoor*>(obj)) b->Update(dt);
    }

    if (stageIndex == CS230::RunProgressManager::TutorialConfigId && player.GetRect().Bottom() <= 0.0) player.SetHealth(0);
    if (player.GetRect().Bottom() <= 0.0) player.SetHealth(0);

    if (player.GetHealth() <= 0)
    {
        if (!ui.IsGameOver())
        {
            ui.TriggerGameOver();
            Engine::GetSFXManager().StopBGM();
            currentBGM = "";
        }
        return;
    }

    if (player.TryConsumeDropThroughRequest())
    {
        for (auto* obj : staticObjects)
        {
            if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
            {
                ignoreFloorTarget = obj;
                ignoreFloorTimer = 30;
                break;
            }
        }
    }

    for (auto& entity : enemies)
    {
        if (entity.logic && !entity.logic->IsActive()) continue;
        if (entity.updateFunc) entity.updateFunc(player, dt, hitStopActive);

        if (entity.logic && entity.go)
        {
            auto* eRc = entity.go->GetGOComponent<CS230::RectCollision>();
            if (eRc)
            {
                Math::rect eRect = eRc->WorldBoundary();
                for (auto* obj : staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
                    {
                        auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                        if (rc)
                        {
                            Math::rect r = rc->WorldBoundary();

                            if (Math::IntersectsRect(eRect, r))
                            {
                                entity.logic->ResolveCollision(GameObjectTypes::Platform, &r, false, dt);
                            }
                        }
                    }
                }
            }
            player.ResolveCollision(entity.go->Type(), entity.logic, false, dt);
        }
    }

    for (auto* obj : staticObjects)
    {
        if (obj->Type() == GameObjectTypes::Platform)
        {
            if (obj != ignoreFloorTarget) player.ResolveCollision(obj);
        }
        else if (obj->Type() == GameObjectTypes::TwoWayPlatform)
        {
            if (obj != ignoreFloorTarget && player.move.velocity.y <= 0)
            {
                Math::rect pRect = player.GetRect();
                Math::rect platRect = obj->GetGOComponent<CS230::RectCollision>()->WorldBoundary();

                double prevBottom = pRect.Bottom() - player.move.velocity.y * dt;

                if (prevBottom >= platRect.Top() - 0.5)
                {
                    player.ResolveCollision(obj);
                }
            }
        }
        else if (obj->Type() == GameObjectTypes::Zipline)
        {
            Math::rect zRect = obj->GetGOComponent<CS230::RectCollision>()->WorldBoundary();
            Math::rect pRect = player.GetRect();

            if (!(pRect.Right() < zRect.Left() || pRect.Left() > zRect.Right() || pRect.Top() < zRect.Bottom() || pRect.Bottom() > zRect.Top()))
            {
                if (CS230::KeyBinding::GetInstance().IsActionKeyJustPressed(CS230::Action::Interact))
                {
                    double targetX = zRect.Left() + zRect.Size().x * 0.5 - CS230::Player::DefaultWidth * 0.5;
                    player.StartZipline(targetX, zRect);
                }
            }
        }
    }

    for (int i = 0; i < (int)items.size(); ++i)
    {
        if (auto* scrap = dynamic_cast<CS230::Scrap*>(items[i]))
        {
            if (!scrap->IsConsumed())
            {
                scrap->Update(dt);
                scrap->ResolveCollision(&player);

                for (auto* obj : staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
                    {
                        scrap->ResolveCollision(obj);
                    }
                }
            }
        }
        else if (auto* story = dynamic_cast<CS230::StoryItem*>(items[i]))
        {
            if (!story->IsConsumed())
            {
                story->Update(dt);
                story->ResolveCollision(&player);

                for (auto* obj : staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
                    {
                        story->ResolveCollision(obj);
                    }
                }
            }
        }
    }

    bool isBossStage = (stageIndex == 3 || stageIndex == 4 || stageIndex == 5);

    if (isBossStage)
    {
        if (!runProgress.IsStageCleared(currentSlotLoaded) && AreEnemiesCleared())
        {
            runProgress.MarkStageCleared(currentSlotLoaded);
            if (rightPortal) rightPortal->SetEnabled(true);
            if (leftPortal) leftPortal->SetEnabled(true);

            int themeId = -1;
            if (stageIndex == 3) themeId = 0;
            else if (stageIndex == 4) themeId = 1;
            else if (stageIndex == 5) themeId = 2;

            if (player.GetStoryFragments() >= 3)
            {
                if (!(player.combat.viewedBossStories & (1 << themeId)))
                {
                    player.combat.showStoryUI = true;
                    player.combat.currentStoryId = themeId;
                    player.combat.viewedBossStories |= (1 << themeId);
                    player.combat.showMissingFragmentsUI = false;
                }
            }
            else
            {
                player.combat.showMissingFragmentsUI = true;
            }
        }
    }
    else
    {
        if (rightPortal && !rightPortal->IsEnabled())
        {
            rightPortal->SetEnabled(true);
        }
    }

    bool usePortal = Engine::GetInput().KeyJustPressed(CS230::Input::Keys::E);

    if (generator && usePortal && !isBackdoorMenuOpen)
    {
        Math::rect r = generator->GetRect();
        Math::rect plr = player.GetRect();
        if (!(plr.Right() < r.Left() || plr.Left() > r.Right() || plr.Top() < r.Bottom() || plr.Bottom() > r.Top()))
        {
            generator->ToggleUI();
            return;
        }
    }

    if (usePortal && !isBackdoorMenuOpen)
    {
        for (auto* obj : staticObjects)
        {
            if (obj->Type() == GameObjectTypes::HPStation)
            {
                auto* station = static_cast<CS230::HPStation*>(obj);
                Math::rect r = station->GetRect();
                Math::rect plr = player.GetRect();
                if (!(plr.Right() < r.Left() || plr.Left() > r.Right() || plr.Top() < r.Bottom() || plr.Bottom() > r.Top()))
                {
                    player.Heal(999);

                    if (player.GetWeapon() && player.GetWeapon()->GetType() == CS230::WeaponType::Arrow)
                    {
                        auto* arrowWep = static_cast<CS230::ArrowWeapon*>(player.GetWeapon());
                        arrowWep->SetAmmo(arrowWep->GetMaxAmmo()); 
                        player.combat.savedAmmo = arrowWep->GetAmmo(); 
                    }

                    Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/heal.wav");
                    return;
                }
            }
        }
    }

    if (usePortal)
    {
        for (auto* bd : backdoors)
        {
            Math::rect r = bd->GetRect();
            Math::rect plr = player.GetRect();

            if (!(plr.Right() <= r.Left() || plr.Left() >= r.Right() || plr.Top() <= r.Bottom() || plr.Bottom() >= r.Top()))
            {
                int bdX = static_cast<int>(r.Left());
                int bdY = static_cast<int>(r.Bottom());
                std::pair<int, int> currentCoord = { bdX, bdY };

                bool wasAlreadyUnlocked = (s_unlockedBackdoorCoords.count(currentCoord) > 0);

                if (!wasAlreadyUnlocked)
                {
                    bd->SetActivated(true);
                    s_unlockedBackdoorCoords.insert(currentCoord);
                    Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/heal.wav"); 
                }
                else
                {
                    s_menuBackdoors.clear();
                    for (auto* b : backdoors)
                    {
                        int bx = static_cast<int>(b->GetRect().Left());
                        int by = static_cast<int>(b->GetRect().Bottom());

                        if (s_unlockedBackdoorCoords.count({ bx, by }) > 0)
                        {
                            s_menuBackdoors.push_back(b);
                        }
                    }
                    isBackdoorMenuOpen = true;
                    activeBackdoor = bd;
                    backdoorMenuSelection = 0;
                    s_lastMenuSelection = -1;
                    for (size_t i = 0; i < s_menuBackdoors.size(); ++i)
                    {
                        if (s_menuBackdoors[i] == bd)
                        {
                            backdoorMenuSelection = static_cast<int>(i);
                            break;
                        }
                    }

                    Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
                }
                return;
            }
        }
    }
    
    auto checkPortal = [&](ProtoPortal* p, bool isLeftPortal)
    {
        if (p && usePortal)
        {
            if (!p->IsEnabled()) return false;
            Math::rect r = p->WorldRect();
            Math::rect plr = player.GetRect();
            if (!(plr.Right() <= r.Left() || plr.Left() >= r.Right() || plr.Top() <= r.Bottom() || plr.Bottom() >= r.Top()))
            {
                if (!isLeftPortal && currentSlotLoaded == 9)
                {
                    if (AreEnemiesCleared())
                    {
                        ui.TriggerEnding(); 
                        return true;
                    }
                }

                if (!isLeftPortal && (stageIndex == 3 || stageIndex == 4 || stageIndex == 5))
                {
                    player.Heal(999);
                    if (player.GetWeapon() && player.GetWeapon()->GetType() == CS230::WeaponType::Arrow)
                    {
                        auto* arrowWep = static_cast<CS230::ArrowWeapon*>(player.GetWeapon());
                        arrowWep->SetAmmo(arrowWep->GetMaxAmmo());
                    }
                }

                if (player.GetWeapon() && player.GetWeapon()->GetType() == CS230::WeaponType::Arrow)
                {
                    player.combat.savedAmmo = static_cast<CS230::ArrowWeapon*>(player.GetWeapon())->GetAmmo();
                }

                pendingSlot = p->TargetIndex();
                stageIndex = runProgress.GetStageConfigIdForSlot(pendingSlot);

                if (isLeftPortal) pendingRespawn = RespawnMode::ExitPortal;
                else pendingRespawn = RespawnMode::EntryPortal;

                Reset();
                return true;
            }
        }
        return false;
    };

    if (checkPortal(rightPortal, false)) return;
    if (checkPortal(leftPortal, true)) return;

    for (auto* obj : staticObjects)
    {
        if (auto* p = dynamic_cast<ProtoPortal*>(obj))
        {
            if (p != leftPortal && p != rightPortal)
            {
                if (checkPortal(p, false)) return;
            }
        }
    }

    if (auto cam = GetGSComponent<CS230::Camera>()) cam->Update(player.Center());
}

void StageManager::Draw()
{
    ::Engine::GetWindow().Clear(GAME200::GRAY);

    Math::TransformationMatrix camM = Math::TranslationMatrix(Math::vec2(0.0, 0.0));
    CS230::Camera* cam = nullptr;
    if (auto c = GetGSComponent<CS230::Camera>())
    {
        cam = c;
        camM = cam->GetMatrix();
    }

    if (handlers.Draw)
    {
        handlers.Draw(*this, camM);
        return;
    }

    auto RenderWorld = [&]()
        {
            if (backgroundTex)
            {
                Math::vec2 bgSize = { static_cast<double>(backgroundTex->GetSize().x), static_cast<double>(backgroundTex->GetSize().y) };
                if (bgSize.x > 0.0 && bgSize.y > 0.0)
                {
                    Math::vec2 scale = { worldWidthCur / bgSize.x, worldHeightCur / bgSize.y };
                    backgroundTex->Draw(camM * Math::ScaleMatrix(scale));
                }
                else
                {
                    backgroundTex->Draw(camM);
                }
            }

            DrawSolids(camM);

            if (leftPortal && leftPortal->IsEnabled()) leftPortal->Draw(camM);
            if (rightPortal && rightPortal->IsEnabled()) rightPortal->Draw(camM);
            if (generator) generator->Draw(camM);

            for (auto* obj : staticObjects)
            {
                if (auto* p = dynamic_cast<ProtoPortal*>(obj)) if (p->IsEnabled()) p->Draw(camM);
                if (auto* b = dynamic_cast<CS230::Backdoor*>(obj)) b->Draw(camM);
                if (auto* hps = dynamic_cast<CS230::HPStation*>(obj)) hps->Draw(camM);
            }

            for (auto textpair : texts)
            {
                textpair.first->Draw(camM * Math::TranslationMatrix(textpair.second));
            }

            for (auto& entity : enemies)
            {
                if (entity.logic && entity.logic->IsActive() && entity.go) entity.go->Draw(camM);
            }

            player.Draw(camM);

            for (auto* item : items)
            {
                if (auto* scrap = dynamic_cast<CS230::Scrap*>(item))
                {
                    if (!scrap->IsConsumed()) scrap->Draw(camM);
                }
                else if (auto* story = dynamic_cast<CS230::StoryItem*>(item))
                {
                    if (!story->IsConsumed()) story->Draw(camM);
                }
            }

            if (auto gom = GetGSComponent<CS230::GameObjectManager>()) gom->DrawAll(camM);
        };


    auto RenderBackdoorMenu = [&](Math::TransformationMatrix ortho)
        {
            int currentConfigId = runProgress.GetStageConfigIdForSlot(currentSlotLoaded);
            if (s_lastBackdoorImageStage != currentConfigId)
            {
                const char* configNames[] = {
                    "StageFactory", "StageForest", "StageCity",
                };
                std::string cfgName = (currentConfigId >= 0 && currentConfigId <= 7) ? configNames[currentConfigId] : "Unknown";
                std::string mapImgPath = "Assets/Maps/" + cfgName + ".png";
                s_backdoorImageTex = Engine::GetTextureManager().Load(mapImgPath);
                s_lastBackdoorImageStage = currentConfigId;
            }

            if (s_backdoorImageTex)
            {
                Math::ivec2 texSize = s_backdoorImageTex->GetSize();
                if (texSize.x > 0 && texSize.y > 0)
                {
                    double scaleX = 1920.0 / static_cast<double>(texSize.x);
                    double scaleY = 1080.0 / static_cast<double>(texSize.y);
                    s_backdoorImageTex->Draw(ortho * Math::ScaleMatrix(Math::vec2{ scaleX, scaleY }));
                }
            }

            double mapToWorldX = 1920.0 / static_cast<double>(worldWidthCur);
            double mapToWorldY = 1080.0 / static_cast<double>(worldHeightCur);

            if (s_lastMenuSelection != backdoorMenuSelection || s_lastActiveBackdoor != activeBackdoor)
            {
                s_cachedMenuTextures.clear();
                for (int i = 0; i < (int)s_menuBackdoors.size(); ++i)
                {
                    unsigned int color = (i == backdoorMenuSelection) ? GAME200::RED : GAME200::BLACK;

                    std::string text = "BackDoor " + std::to_string(s_menuBackdoors[i]->GetIndex());
                    s_cachedMenuTextures.push_back(Engine::GetFont(0).PrintToTextureScaled(text, color, 0.33));
                }
                s_lastMenuSelection = backdoorMenuSelection;
                s_lastActiveBackdoor = activeBackdoor;
            }

            for (int i = 0; i < (int)s_menuBackdoors.size(); ++i)
            {
                auto* bd = s_menuBackdoors[i];

                Math::vec2 worldPos = {
                    bd->GetRect().Left() + bd->GetUIOffset().x,
                    bd->GetRect().Bottom() + bd->GetUIOffset().y
                };

                Math::vec2 screenPos = { worldPos.x * mapToWorldX, worldPos.y * mapToWorldY };

                if (i < (int)s_cachedMenuTextures.size() && s_cachedMenuTextures[i])
                {
                    s_cachedMenuTextures[i]->Draw(ortho * Math::TranslationMatrix(screenPos));
                }
            }
        };

    RenderWorld();

#ifdef _DEBUG
    if (auto sc = GetGSComponent<CS230::ShowCollision>())
    {
        if (sc->Enabled())
        {
            for (auto* obj : staticObjects)
            {
                if (auto* rc = obj->GetGOComponent<CS230::RectCollision>()) rc->Draw(camM);
                if (auto* cc = obj->GetGOComponent<CS230::CircleCollision>()) cc->Draw(camM);
            }
        }
    }
#endif

    bool isBossStageUI = (stageIndex == 3 || stageIndex == 4 || stageIndex == 5);

    Math::TransformationMatrix ortho = Math::TranslationMatrix(Math::vec2{ 0.0, 0.0 });

    if (generator && generator->IsUIOpen()) generator->DrawUI(ortho, &player);

    if (!player.ShouldShowStoryUI() && player.combat.showMissingFragmentsUI)
    {
        static std::shared_ptr<CS230::Texture> s_missingTex;
        static int s_lastMissingFragments = -1;

        if (s_lastMissingFragments != player.GetStoryFragments())
        {
            std::string msg = "Story Items Collected : ( " + std::to_string(player.GetStoryFragments()) + " / 3 )";
            s_missingTex = Engine::GetFont(0).PrintToTextureScaled(msg, 0xFFFFFFFFu, 0.6);
            s_lastMissingFragments = player.GetStoryFragments();
        }

        if (s_missingTex)
        {
            Math::ivec2 ts = s_missingTex->GetSize();
            s_missingTex->Draw(Math::TranslationMatrix(Math::vec2{ 1920.0 / 2.0 - ts.x / 2.0, 1080.0 / 2.0 + 250.0 }));
        }
    }

    if (isBackdoorMenuOpen && !s_menuBackdoors.empty()) RenderBackdoorMenu(ortho);

    if (isBossStageUI)
    {
        ui.Draw(this, player, camM);
    }
    else
    {
        ui.Draw(player, camM);
    }
}

void StageManager::Unload()
{
    if (isTransitioningToSettings)
    {
        return;
    }

    Engine::GetSFXManager().StopBGM();
    currentBGM = "";

    if (handlers.Unload)
    {
        handlers.Unload(*this);
    }

    if (auto gom = GetGSComponent<CS230::GameObjectManager>())
    {
        gom->Unload();
    }

    InitStage(0, 0);
    ClearGSComponents();
}

void StageManager::ApplyPortalTargetsForSlot(int slot)
{
    int nextSlot = std::min(static_cast<int>(CS230::RunProgressManager::StageSlotCount - 1), static_cast<int>(slot + 1));
    int prevSlot = std::max(0, static_cast<int>(slot - 1));

    if (leftPortal)
    {
        leftPortal->SetTargetIndex(prevSlot);
        leftPortal->SetEnabled(true);

        if (slot == 0 || slot == 1 || slot == 4 || slot == 7)
        {
            leftPortal->SetEnabled(false);
        }
    }

    if (rightPortal)
    {
        rightPortal->SetTargetIndex(nextSlot);
    }

    for (auto* obj : staticObjects)
    {
        if (auto* p = dynamic_cast<ProtoPortal*>(obj))
        {
            if (p != leftPortal && p != rightPortal)
            {
                p->SetTargetIndex(nextSlot);
            }
        }
    }
}

void StageManager::ClearEnemyInstances()
{
    for (auto& entity : enemies)
    {
        delete entity.go;
    }
    enemies.clear();
}

bool StageManager::AreEnemiesCleared() const
{
    if (enemies.empty())
    {
        return true;
    }
    for (const auto& entity : enemies)
    {
        if (entity.logic && entity.logic->IsActive() && entity.logic->GetHealth() > 0)
        {
            return false;
        }
    }
    return true;
}

Math::vec2 StageManager::PortalSpawnBottomLeft(const ProtoPortal* portal, const Math::vec2& fallback) const
{
    if (portal == nullptr)
    {
        return fallback;
    }
    const Math::rect& pr = portal->WorldRect();
    Math::vec2 spawn = fallback;
    const double width = CS230::Player::DefaultWidth;
    const double centerX = pr.Left() + (pr.Right() - pr.Left()) * 0.5;
    spawn.x = centerX - width * 0.5;
    spawn.y = pr.Bottom();
    return spawn;
}

Math::vec2 StageManager::MidBossRespawnPoint(double groundTop, const Math::ivec2& offset) const
{
    Math::vec2 spawn;
    spawn.x = offset.x + worldWidthCur * 0.5 - CS230::Player::DefaultWidth * 0.5;
    spawn.y = groundTop + offset.y;
    return spawn;
}

void StageManager::HandlePlayerDeath()
{
    player.SetHealth(5);
    Engine::GetSFXManager().StopBGM();
    currentBGM = "";

    if (stageIndex == 3 || stageIndex == 4 || stageIndex == 5)
    {
        int shopSlot = currentSlotLoaded - 1;
        if (shopSlot >= 0 && runProgress.GetStageConfigIdForSlot(shopSlot) == 6)
        {
            pendingSlot = shopSlot;
            stageIndex = 6;
            pendingRespawn = RespawnMode::ExitPortal;
            Reset();
            return;
        }
    }

    pendingRespawn = RespawnMode::EntryPortal;
    Reset();
}