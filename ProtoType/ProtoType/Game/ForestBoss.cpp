#include "ForestBoss.h"
#include "Player.h"
#include "Scrap.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../Game/Gravity.h"
#include "../OpenGL/RGBA.h"
#include "../Stage/StageManager.h"
#include <cmath>
#include <GL/glew.h>
#include <algorithm>
#include <memory>

namespace CS230
{
    void ForestBoss::Load(const Math::vec2& position_, int initialHealth)
    {
        Math::vec2 fixedSize{ 600, 600 };
        rect.point_1 = position_;
        rect.point_2 = { position_.x + fixedSize.x, position_.y + fixedSize.y };
        health = initialHealth;
        maxHealth = initialHealth;

        AddGOComponent(new CS230::Sprite("Assets/image/Bosses/ForestBoss/ForestBoss.spt", this));
		this -> GetGOComponent<CS230::Sprite>()->PlayAnimation(0);

        currentState = FB2State::Idle;
        stateTimer = 5.0;

        laserActive = false;
        laserMovesRight = true;
        laserDamageApplied = false;
        laserP1 = { 0.0, 0.0 };
        laserP2 = { 0.0, 0.0 };

        for (int i = 0; i < 5; ++i)
        {
            orbs[i].active = false;
        }

        active = true;
        SetPosition({ rect.Left(), rect.Bottom() });
    }

    void ForestBoss::Unload()
    {
        ClearGOComponents();
        active = false;
        laserActive = false;
        laserMovesRight = true;
        laserDamageApplied = false;
        laserP1 = { 0.0, 0.0 };
        laserP2 = { 0.0, 0.0 };
        health = 0;
        for (int i = 0; i < 5; ++i)
        {
            orbs[i].active = false;
        }
        rect = Math::rect{ { -100000.0, -100000.0 }, { -99900.0, -99900.0 } };
    }

    void ForestBoss::Update(const CS230::Player& player, double dt, bool isSlow)
    {

        if (player.IsSlowMoActive()) isSlow = true;

        if (isSlow)
        {
            dt *= Enemy::SlowTimeScale;
        }

        if (!active)
        {
            return;
        }

        if (playerHitCooldownSec > 0.0)
        {
            playerHitCooldownSec -= dt;
            if (playerHitCooldownSec < 0.0)
            {
                playerHitCooldownSec = 0.0;
            }
        }

        if (hitCooldownSec > 0.0)
        {
            hitCooldownSec -= dt;
            if (hitCooldownSec < 0.0)
            {
                hitCooldownSec = 0.0;
            }
        }

        if (health <= 0)
        {
            if (active)
            {
                Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/enumhit.wav");
                auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState());
                if (stage)
                {
                    stage->NotifyEnemyKilled();
                }
                active = false;
            }
            return;
        }

        bool isPhase2 = (health <= maxHealth / 2);
        double gVal = 2400.0;
        if (auto g = Engine::GetGameStateManager().GetGSComponent<Gravity>())
        {
            gVal = g->GetValue();
        }

        if (currentState == FB2State::Idle)
        {
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = FB2State::Charge;
                stateTimer = 3.0;
                if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(1);
            }
        }
        else if (currentState == FB2State::Charge)
        {
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = FB2State::FireOrbs;

                if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(2);

                int orbCount = 3 + (rand() % 3);
                double orbRadius = Player::DefaultHeight * 0.5;
                double orbDiam = orbRadius * 2.0;

                for (int i = 0; i < orbCount; ++i)
                {
                    orbs[i].active = true;
                    orbs[i].grounded = false;
                    orbs[i].exploding = false;
                    orbs[i].timer = 5.0;
                    orbs[i].rect = { { Center().x - orbRadius, rect.Top() }, { Center().x + orbRadius, rect.Top() + orbDiam } };

                    orbs[i].velocity.x = (rand() % 1600) - 800.0;
                    orbs[i].velocity.y = 800.0 + (rand() % 600);
                }

                if (isPhase2)
                {
                    currentState = FB2State::Laser;
                    stateTimer = 3.0;
                    laserMovesRight = (rand() % 2 == 0);
                    laserDamageApplied = false;
                }
                else
                {
                    currentState = FB2State::Idle;
                    stateTimer = 5.0;
                }
            }
        }
        else if (currentState == FB2State::Laser)
        {
            stateTimer -= dt;

            double mapWidth = 1920.0;
            if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
            {
                mapWidth = stage->worldWidthCur;
            }

            double progress = 1.0 - (stateTimer / 3.0);
            if (progress < 0.0) progress = 0.0;
            if (progress > 1.0) progress = 1.0;

            double currentX = laserMovesRight ? (mapWidth * progress) : (mapWidth * (1.0 - progress));

            laserActive = true;

            laserP1 = { Center().x, Center().y + 80.0 };
            laserP2 = { currentX, 80.0 };

            Math::vec2 pCenter = player.Center();
            Math::vec2 a = laserP1;
            Math::vec2 b = laserP2;

            Math::vec2 ab = { b.x - a.x, b.y - a.y };
            Math::vec2 ap = { pCenter.x - a.x, pCenter.y - a.y };

            double proj = ap.x * ab.x + ap.y * ab.y;
            double abLenSq = ab.x * ab.x + ab.y * ab.y;
            double t = 0.0;
            if (abLenSq > 0.0) t = proj / abLenSq;

            if (t < 0.0) t = 0.0;
            else if (t > 1.0) t = 1.0;

            Math::vec2 closest = { a.x + t * ab.x, a.y + t * ab.y };
            Math::vec2 distVec = { pCenter.x - closest.x, pCenter.y - closest.y };
            double distSq = distVec.x * distVec.x + distVec.y * distVec.y;

            double hitRadius = std::max(player.GetRect().Size().x, player.GetRect().Size().y) * 0.5;
            double laserThickness = 20.0;

            if (!laserDamageApplied && distSq <= (hitRadius + laserThickness) * (hitRadius + laserThickness))
            {
                auto* pNonConst = const_cast<Player*>(&player);
                if (pNonConst->HandleParry())
                {
                    laserDamageApplied = true;
                }
                else if (pNonConst->IsRolling()) {
                    if (!pNonConst->move.dodgeRewardClaimed) {
                        pNonConst->AddTimeGauge(pNonConst->gaugeGainRoll);
                        pNonConst->move.dodgeRewardClaimed = true;
                    }
                }
                else if (pNonConst->CanTakeHit() && !pNonConst->IsInvincible())
                {
                    pNonConst->TakeHit();
                    laserDamageApplied = true;
                }
            }

            if (stateTimer <= 0.0)
            {
                laserActive = false;
                currentState = FB2State::Idle;
                stateTimer = 5.0;

                if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);
            }
        }

        for (int i = 0; i < 5; ++i)
        {
            if (!orbs[i].active)
            {
                continue;
            }

            if (orbs[i].exploding)
            {
                orbs[i].explodeTimer -= dt;
                if (orbs[i].explodeTimer <= 0.0)
                {
                    orbs[i].active = false;
                }
                else if (!orbs[i].damageApplied)
                {
                    double expRadius = Player::DefaultWidth * 1.5;
                    Math::vec2 oC = { orbs[i].rect.Left() + orbs[i].rect.Size().x * 0.5, orbs[i].rect.Bottom() + orbs[i].rect.Size().y * 0.5 };
                    Math::rect expRect = { { oC.x - expRadius, oC.y - expRadius }, { oC.x + expRadius, oC.y + expRadius } };

                    if (Math::IntersectsRect(expRect, player.GetRect()))
                    {
                        auto* pNonConst = const_cast<Player*>(&player);
                        if (pNonConst->HandleParry())
                        {
                            orbs[i].damageApplied = true;
                        }
                        else if (pNonConst->IsRolling()) {
                            if (!pNonConst->move.dodgeRewardClaimed) {
                                pNonConst->AddTimeGauge(pNonConst->gaugeGainRoll);
                                pNonConst->move.dodgeRewardClaimed = true;
                            }
                        }
                        else if (pNonConst->CanTakeHit()) {
                            pNonConst->TakeHit();
                            orbs[i].damageApplied = true;
                        }
                    }
                    if (Math::IntersectsRect(expRect, GetRect()))
                    {
                        health -= 1;
                    }
                    orbs[i].damageApplied = true;
                }
                continue;
            }

            if (!orbs[i].grounded)
            {
                orbs[i].velocity.y -= gVal * dt;
                orbs[i].rect.point_1.x += orbs[i].velocity.x * dt;
                orbs[i].rect.point_2.x += orbs[i].velocity.x * dt;
                orbs[i].rect.point_1.y += orbs[i].velocity.y * dt;
                orbs[i].rect.point_2.y += orbs[i].velocity.y * dt;

                double mapWidth = 1920.0;
                if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState())) {
                    mapWidth = stage->worldWidthCur;
                }

                if (orbs[i].rect.Left() < 0.0) {
                    orbs[i].velocity.x = std::abs(orbs[i].velocity.x);
                    double w = orbs[i].rect.Size().x;
                    orbs[i].rect.point_1.x = 0.0;
                    orbs[i].rect.point_2.x = w;
                }
                else if (orbs[i].rect.Right() > mapWidth) {
                    orbs[i].velocity.x = -std::abs(orbs[i].velocity.x);
                    double w = orbs[i].rect.Size().x;
                    orbs[i].rect.point_2.x = mapWidth;
                    orbs[i].rect.point_1.x = mapWidth - w;
                }

                if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
                {
                    for (CS230::GameObject* obj : stage->staticObjects)
                    {
                        if (obj->Type() == GameObjectTypes::Platform)
                        {
                            if (auto* rc = obj->GetGOComponent<CS230::RectCollision>())
                            {
                                Math::rect pr = rc->WorldBoundary();
                                if (Math::IntersectsRect(orbs[i].rect, pr))
                                {
                                    if (orbs[i].velocity.y <= 0.0 && (orbs[i].rect.Bottom() - orbs[i].velocity.y * dt) >= pr.Top() - 15.0)
                                    {
                                        orbs[i].grounded = true;
                                        orbs[i].velocity = { 0.0, 0.0 };
                                        double h = orbs[i].rect.Size().y;
                                        orbs[i].rect.point_1.y = pr.Top();
                                        orbs[i].rect.point_2.y = pr.Top() + h;
                                        break;
                                    }
                                    else
                                    {
                                        orbs[i].velocity.x *= -1.0;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                orbs[i].timer -= dt;
                if (orbs[i].timer <= 0.0)
                {
                    orbs[i].exploding = true;
                    orbs[i].explodeTimer = 0.2;
                    orbs[i].damageApplied = false;
                }
            }

            if (player.IsAttackActive() && Math::IntersectsRect(orbs[i].rect, player.AttackBox()))
            {
                orbs[i].grounded = false;
                orbs[i].velocity.x = player.IsFacingRight() ? 1200.0 : -1200.0;
                orbs[i].velocity.y = 400.0;
            }
        }

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            Math::ivec2 fs = spr->GetFrameSize();
            if (fs.x > 0 && fs.y > 0)
            {
                Math::vec2 curSize = rect.Size();
                Math::vec2 newScale{ curSize.x / static_cast<double>(fs.x), curSize.y / static_cast<double>(fs.y) };

                if (currentState == FB2State::Charge)
                {
                    newScale.x *= 0.5;
                    newScale.y *= 0.5;
                }
                SetScale(newScale);
            }
        }

        SetPosition({ rect.Left(), rect.Bottom() });
        UpdateGOComponents(dt);
    }

    void ForestBoss::Draw(Math::TransformationMatrix camM)
    {
        if (!active || health <= 0)
        {
            return;
        }

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            if (hitCooldownSec > 0.0) {
                spr->Draw(camM * GetMatrix(), GAME200::RED);
            }
            else {
                CS230::GameObject::Draw(camM);
            }
        }
        else
        {
            Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(Center()) * Math::ScaleMatrix(Math::vec2{ Width(), Height() }), GAME200::DKGREEN, GAME200::BLACK, 1.0);
        }

        for (int i = 0; i < 5; ++i)
        {
            if (orbs[i].active)
            {
                if (orbs[i].exploding)
                {
                    double expRadius = Player::DefaultWidth * 1.5;
                    Math::vec2 expDiam = { expRadius * 2.0, expRadius * 2.0 };
                    Math::vec2 oC = { orbs[i].rect.Left() + orbs[i].rect.Size().x * 0.5, orbs[i].rect.Bottom() + orbs[i].rect.Size().y * 0.5 };
                    Engine::GetRenderer2D().DrawCircle(camM * Math::TranslationMatrix(oC) * Math::ScaleMatrix(expDiam), GAME200::RED, GAME200::RED, 0.0);
                }
                else
                {
                    const Math::vec2 os = orbs[i].rect.Size();
                    const Math::vec2 oc = { orbs[i].rect.Left() + os.x * 0.5, orbs[i].rect.Bottom() + os.y * 0.5 };
                    Engine::GetRenderer2D().DrawCircle(camM * Math::TranslationMatrix(oc) * Math::ScaleMatrix(os), GAME200::BLACK, GAME200::GRAY, 2.0);

                    static std::shared_ptr<CS230::Texture> numTex[5];
                    if (!numTex[0]) {
                        numTex[0] = Engine::GetFont(0).PrintToTextureScaled("1", GAME200::RED, 1.0);
                        numTex[1] = Engine::GetFont(0).PrintToTextureScaled("2", GAME200::RED, 1.0);
                        numTex[2] = Engine::GetFont(0).PrintToTextureScaled("3", GAME200::RED, 1.0);
                        numTex[3] = Engine::GetFont(0).PrintToTextureScaled("4", GAME200::RED, 1.0);
                        numTex[4] = Engine::GetFont(0).PrintToTextureScaled("5", GAME200::RED, 1.0);
                    }

                    int sec = static_cast<int>(std::ceil(orbs[i].timer));
                    if (sec >= 1 && sec <= 5 && numTex[sec - 1]) {
                        Math::ivec2 ts = numTex[sec - 1]->GetSize();
                        double tx = oc.x - ts.x * 0.5;
                        double ty = oc.y - ts.y * 0.5;
                        numTex[sec - 1]->Draw(camM * Math::TranslationMatrix(Math::vec2{ tx, ty }));
                    }
                }
            }
        }

        if (laserActive)
        {
            Math::vec2 dir = { laserP2.x - laserP1.x, laserP2.y - laserP1.y };
            double length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            double angle = std::atan2(dir.y, dir.x);

            Math::vec2 center = { (laserP1.x + laserP2.x) * 0.5, (laserP1.y + laserP2.y) * 0.5 };
            Math::vec2 size = { length, 20.0 };

            Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(center) * Math::RotationMatrix(angle) * Math::ScaleMatrix(size), 0xFF000080, 0xFF000080, 0.0);
        }
    }
}
