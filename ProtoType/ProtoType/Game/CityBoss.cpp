#include "CityBoss.h"
#include "Player.h"
#include "Scrap.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../Game/Gravity.h"
#include "../OpenGL/RGBA.h"
#include "../Stage/StageManager.h"
#include <cmath>
#include <cstdlib> 
#include <GL/glew.h>
#include <algorithm>

namespace CS230
{
    void CityBoss::Load(const Math::vec2& position_, int initialHealth)
    {
        health = initialHealth;
        maxHealth = initialHealth;
        active = true;

        currentState = CBState::Idle;
        stateTimer = 5.0;
        attackApplied = false;

        AddGOComponent(new CS230::Sprite("Assets/image/Bosses/CityBoss/CityBoss_Idle.spt", this));
        if (auto spr = GetGOComponent<CS230::Sprite>()) {
            spr->PlayAnimation(0);
        }

        double finalW = 150.0;
        double finalH = 240.0;

        rect.point_1 = position_;
        rect.point_2.x = position_.x + finalW;
        rect.point_2.y = position_.y + finalH;

        Math::vec2 startPos;
        startPos.x = rect.Left();
        startPos.y = rect.Bottom();
        SetPosition(startPos);

        for (int i = 0; i < 100; ++i)
        {
            shurikens[i].active = false;
            shurikens[i].reflected = false;
        }
        swordAttackEffect.AddGOComponent(new CS230::Sprite("Assets/image/Bosses/CityBoss/BossATK.spt", &swordAttackEffect));
    }

    void CityBoss::Unload()
    {
        ClearGOComponents();
        active = false;
        health = 0;
        rect.point_1.x = -100000.0;
        rect.point_1.y = -100000.0;
        rect.point_2.x = -99900.0;
        rect.point_2.y = -99900.0;

        for (int i = 0; i < 100; ++i)
        {
            shurikens[i].active = false;
        }
        delete swordAttackEffect.GetGOComponent<CS230::Sprite>();
        swordAttackEffect.ClearGOComponents();
    }

    bool CityBoss::IsMeleeAttackActive() const
    {
        if (attackApplied) return false;
        return (currentState == CBState::Sword_Approach || currentState == CBState::Sword_Attack || currentState == CBState::Dash_Sweep);
    }

    Math::rect CityBoss::GetAttackBox() const
    {
        if (attackApplied) return Math::rect{ {0,0}, {0,0} };

        if (currentState == CBState::Sword_Attack) return swordAttackRect;
        if (currentState == CBState::Sword_Approach || currentState == CBState::Dash_Sweep) return GetRect();

        Math::rect emptyRect;
        emptyRect.point_1.x = 0.0; emptyRect.point_1.y = 0.0;
        emptyRect.point_2.x = 0.0; emptyRect.point_2.y = 0.0;
        return emptyRect;
    }

    Math::vec2 CityBoss::FindCenterAndFloorY(double& outMapWidth, double& outFloorY)
    {
        outMapWidth = 1920.0;
        outFloorY = 80.0;
        Math::vec2 center;
        center.x = 960.0;
        center.y = 300.0;

        if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
        {
            outMapWidth = static_cast<double>(stage->worldWidthCur);

            double offX = (outMapWidth < 1920.0) ? (1920.0 - outMapWidth) / 2.0 : 0.0;
            center.x = offX + (outMapWidth * 0.5);

            for (auto* obj : stage->staticObjects)
            {
                if (obj->Type() == GameObjectTypes::Platform)
                {
                    Math::rect pr = obj->GetGOComponent<CS230::RectCollision>()->WorldBoundary();
                    outFloorY = pr.Top();
                    break;
                }
            }

            center.y = outFloorY + 300.0;
        }
        return center;
    }

    void CityBoss::Update(const CS230::Player& player, double dt, bool isSlow)
    {
        if (player.IsSlowMoActive()) isSlow = true;
        if (isSlow) dt *= Enemy::SlowTimeScale;
        if (!active) return;

        if (playerHitCooldownSec > 0.0) {
            playerHitCooldownSec -= dt;
            if (playerHitCooldownSec < 0.0) playerHitCooldownSec = 0.0;
        }

        if (hitCooldownSec > 0.0) {
            hitCooldownSec -= dt;
            if (hitCooldownSec < 0.0) hitCooldownSec = 0.0;
        }

        if (health <= 0) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/enumhit.wav");
            if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState())) {
                stage->NotifyEnemyKilled();
            }
            active = false;
            return;
        }

        isPhase2 = (health <= maxHealth / 2);

        auto playAnim = [&](const std::string& path) {

            while (GetGOComponent<CS230::RectCollision>() != nullptr) {
                RemoveGOComponent<CS230::RectCollision>();
            }

            if (auto spr = GetGOComponent<CS230::Sprite>()) {
                spr->Load(path);
                spr->PlayAnimation(0);
                Math::ivec2 fs = spr->GetFrameSize();
                if (fs.x > 0 && fs.y > 0) {
                    double newW = static_cast<double>(fs.x);
                    double newH = static_cast<double>(fs.y);

                    double cx = rect.Left() + rect.Size().x * 0.5;
                    double by = rect.Bottom();

                    rect.point_1.x = cx - (newW * 0.5);
                    rect.point_1.y = by;
                    rect.point_2.x = rect.point_1.x + newW;
                    rect.point_2.y = rect.point_1.y + newH;
                }
            }
        };

        auto GetCurrentRect = [&]() -> Math::rect {
            if (auto* col = GetGOComponent<CS230::RectCollision>()) {
                return col->WorldBoundary();
            }
            return rect;
        };

        double mapW = 0.0, floorY = 0.0;
        Math::vec2 mapCenter = FindCenterAndFloorY(mapW, floorY);

        double offX = mapCenter.x - (mapW * 0.5);
        double limitRight = offX + mapW;

        double trueLeft = offX;
        double trueRight = limitRight;

        if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
        {
            for (auto* obj : stage->staticObjects)
            {
                if (obj->Type() == GameObjectTypes::Platform)
                {
                    Math::rect pr = obj->GetGOComponent<CS230::RectCollision>()->WorldBoundary();

                    if (pr.Left() <= offX + 100.0 && pr.Right() < mapCenter.x) {
                        if (pr.Right() > trueLeft) trueLeft = pr.Right();
                    }
                    if (pr.Right() >= limitRight - 100.0 && pr.Left() > mapCenter.x) {
                        if (pr.Left() < trueRight) trueRight = pr.Left();
                    }
                }
            }
        }

        switch (currentState)
        {
        case CBState::Idle:
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = CBState::Sword_Approach;
                moveDir = (player.Center().x > Center().x) ? 1 : -1;
                this->Enemy::velocity.x = static_cast<double>(moveDir) * 1200.0;
                this->Enemy::velocity.y = 0.0;
                attackApplied = false;
            }
            break;

        case CBState::Sword_Approach:
            rect.point_1.x += this->Enemy::velocity.x * dt;
            rect.point_2.x += this->Enemy::velocity.x * dt;

            if (!attackApplied && Math::IntersectsRect(GetCurrentRect(), player.GetRect()))
            {
                auto* pNC = const_cast<Player*>(&player);
                if (pNC->HandleParry()) { attackApplied = true; }
                else if (pNC->IsRolling()) {
                    if (!pNC->move.dodgeRewardClaimed) {
                        pNC->AddTimeGauge(pNC->gaugeGainRoll);
                        pNC->move.dodgeRewardClaimed = true;
                    }
                }
                else if (pNC->CanTakeHit() && !pNC->IsInvincible()) {
                    pNC->TakeHit();
                    pNC->ApplyKnockback(800.0, moveDir, 200.0);
                    attackApplied = true;
                }
            }

            if (std::abs(Center().x - player.Center().x) < (Width() * 0.5 + Player::DefaultWidth * 2.0) || rect.Left() <= trueLeft || rect.Right() >= trueRight)
            {
                bool reachedPlayer = std::abs(Center().x - player.Center().x) < (Width() * 0.5 + Player::DefaultWidth * 2.0);
                bool hitLeftWall = (moveDir < 0 && rect.Left() <= trueLeft);
                bool hitRightWall = (moveDir > 0 && rect.Right() >= trueRight);

                if (reachedPlayer || hitLeftWall || hitRightWall)
                {
                    this->Enemy::velocity.x = 0.0;
                    
                    if (hitLeftWall) {
                        rect.point_1.x = trueLeft;
                        rect.point_2.x = trueLeft + Width();
                    } else if (hitRightWall) {
                        rect.point_2.x = trueRight;
                        rect.point_1.x = trueRight - Width();
                    }

                    currentState = CBState::Sword_AttackWait;
                    subCount = 0;
                    maxSubCount = isPhase2 ? 4 : 3;
                    stateTimer = 0.5;
                    swordTelegraphTimer = 0.5;

                    playAnim("Assets/image/Bosses/CityBoss/CityBoss_Idle.spt");
                }
            }
            break;

        case CBState::Sword_AttackWait:
            stateTimer -= dt;
            swordTelegraphTimer -= dt;
            moveDir = (player.Center().x > Center().x) ? 1 : -1;

            {
                double reach = Player::DefaultWidth * 6.0;
                double h = Player::DefaultHeight;
                Math::rect bodyRect = GetCurrentRect();

                if (moveDir > 0)
                {
                    swordAttackRect.point_1.x = bodyRect.Right();
                    swordAttackRect.point_1.y = bodyRect.Bottom();
                    swordAttackRect.point_2.x = bodyRect.Right() + reach;
                    swordAttackRect.point_2.y = bodyRect.Bottom() + h;
                }
                else
                {
                    swordAttackRect.point_1.x = bodyRect.Left() - reach;
                    swordAttackRect.point_1.y = bodyRect.Bottom();
                    swordAttackRect.point_2.x = bodyRect.Left();
                    swordAttackRect.point_2.y = bodyRect.Bottom() + h;
                }
            }

            if (stateTimer <= 0.0)
            {
                currentState = CBState::Sword_Attack;
                stateTimer = 0.5;
                attackApplied = false;
                if (swordAttackEffect.GetGOComponent<CS230::Sprite>()) 
                {
                    swordAttackEffect.GetGOComponent<CS230::Sprite>()->PlayAnimation(0);
                }
                if (subCount % 2 == 0) {
                    playAnim("Assets/image/Bosses/CityBoss/CityBoss_First_Slash.spt");
                }
                else {
                    playAnim("Assets/image/Bosses/CityBoss/CityBoss_Second_Slash.spt");
                }
            }
            break;

        case CBState::Sword_Attack:
            stateTimer -= dt;
            if (!attackApplied && Math::IntersectsRect(swordAttackRect, player.GetRect()))
            {
                auto* pNC = const_cast<Player*>(&player);
                if (pNC->HandleParry()) { attackApplied = true; }
                else if (pNC->IsRolling()) {
                    if (!pNC->move.dodgeRewardClaimed) {
                        pNC->AddTimeGauge(pNC->gaugeGainRoll);
                        pNC->move.dodgeRewardClaimed = true;
                    }
                }
                else if (pNC->CanTakeHit() && !pNC->IsInvincible()) {
                    pNC->TakeHit();
                    pNC->ApplyKnockback(600.0, moveDir, 300.0);
                    attackApplied = true;
                }
            }

            if (stateTimer <= 0.0)
            {
                subCount++;
                swordAttackRect.point_1.x = 0.0; swordAttackRect.point_1.y = 0.0;
                swordAttackRect.point_2.x = 0.0; swordAttackRect.point_2.y = 0.0;

                if (subCount >= maxSubCount)
                {
                    currentState = CBState::Wait_3s;
                    stateTimer = 3.0;
                    playAnim("Assets/image/Bosses/CityBoss/CityBoss_Idle.spt");
                }
                else
                {
                    currentState = CBState::Sword_AttackWait;
                    stateTimer = 0.8;
                    swordTelegraphTimer = 0.5;
                    playAnim("Assets/image/Bosses/CityBoss/CityBoss_Idle.spt");
                }
            }
            break;

        case CBState::Wait_3s:
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = CBState::Shuriken_Teleport;
            }
            break;

        case CBState::Shuriken_Teleport:
        {
            playAnim("Assets/image/Bosses/CityBoss/CityBoss_Shuriken.spt");

            Math::vec2 curSize = rect.Size();

            rect.point_1.x = mapCenter.x - (curSize.x * 0.5);
            rect.point_1.y = mapCenter.y;
            rect.point_2.x = rect.point_1.x + curSize.x;
            rect.point_2.y = rect.point_1.y + curSize.y;

            currentState = CBState::Shuriken_Wait;
            stateTimer = 2.0;
            subCount = 0;
            maxSubCount = isPhase2 ? 3 : 2;
        }
        break;

        case CBState::Shuriken_Wait:
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/portal.wav");

                for (int d = 0; d < 12; ++d)
                {
                    double angle = static_cast<double>(d) * (3.1415926535 / 6.0);
                    for (int i = 0; i < 100; ++i)
                    {
                        if (!shurikens[i].active)
                        {
                            shurikens[i].active = true;
                            shurikens[i].reflected = false;
                            shurikens[i].rotation = angle;
                            shurikens[i].velocity.x = std::cos(angle) * 800.0;
                            shurikens[i].velocity.y = std::sin(angle) * 800.0;

                            shurikens[i].rect.point_1.x = Center().x - 30.0;
                            shurikens[i].rect.point_1.y = Center().y - 7.5;
                            shurikens[i].rect.point_2.x = Center().x + 30.0;
                            shurikens[i].rect.point_2.y = Center().y + 7.5;
                            break;
                        }
                    }
                }

                subCount++;
                if (subCount >= maxSubCount)
                {
                    currentState = CBState::Float_3s;
                    stateTimer = 3.0;
                }
                else
                {
                    stateTimer = 2.0;
                    playAnim("Assets/image/Bosses/CityBoss/CityBoss_Shuriken.spt");
                }
            }
            break;

        case CBState::Float_3s:
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = CBState::Dash_Teleport;
                subCount = 0;
                maxSubCount = isPhase2 ? 2 : 1;
            }
            break;

        case CBState::Dash_Teleport:
        {
            playAnim("Assets/image/Bosses/CityBoss/CityBoss_Dash.spt");

            static int lastSide = -1;
            int side = std::rand() % 2;

            if (subCount > 0) {
                side = (lastSide == 0) ? 1 : 0;
            }
            lastSide = side;

            Math::vec2 curSize = rect.Size();
            double currentW = curSize.x;
            double currentH = curSize.y;

            double xPos = (side == 0) ? trueLeft + 10.0 : trueRight - currentW - 10.0;
            moveDir = (side == 0) ? 1 : -1;

            rect.point_1.x = xPos;
            rect.point_1.y = floorY;
            rect.point_2.x = rect.point_1.x + currentW;
            rect.point_2.y = rect.point_1.y + currentH;

            currentState = CBState::Dash_Charge;
            stateTimer = 2.0;
        }
        break;

        case CBState::Dash_Charge:
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = CBState::Dash_Sweep;
                this->Enemy::velocity.x = static_cast<double>(moveDir) * 2500.0;
                attackApplied = false;
            }
            break;

        case CBState::Dash_Sweep:
            if (this->Enemy::velocity.x < 0) moveDir = -1;
            else if (this->Enemy::velocity.x > 0) moveDir = 1;

            rect.point_1.x += this->Enemy::velocity.x * dt;
            rect.point_2.x += this->Enemy::velocity.x * dt;

            if (!attackApplied && Math::IntersectsRect(GetCurrentRect(), player.GetRect()))
            {
                auto* pNC = const_cast<Player*>(&player);
                if (pNC->HandleParry()) { attackApplied = true; }
                else if (pNC->IsRolling()) {
                    if (!pNC->move.dodgeRewardClaimed) {
                        pNC->AddTimeGauge(pNC->gaugeGainRoll);
                        pNC->move.dodgeRewardClaimed = true;
                    }
                }
                else if (pNC->CanTakeHit() && !pNC->IsInvincible()) {
                    pNC->TakeHit();
                    pNC->ApplyKnockback(1200.0, moveDir, 400.0);
                    attackApplied = true;
                }
            }

            if (std::abs(this->Enemy::velocity.x) < 0.1 || rect.Right() >= trueRight - 20.0 || rect.Left() <= trueLeft + 20.0)
            {
                this->Enemy::velocity.x = 0.0;
                subCount++;
                if (subCount >= maxSubCount)
                {
                    currentState = CBState::Idle;
                    stateTimer = 5.0;
                    playAnim("Assets/image/Bosses/CityBoss/CityBoss_Idle.spt");
                }
                else
                {
                    currentState = CBState::Dash_Teleport;
                }
            }
            break;

        default:
            break;
        }

        if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
        {
            for (int i = 0; i < 100; ++i)
            {
                if (shurikens[i].active)
                {
                    shurikens[i].rect.point_1.x += shurikens[i].velocity.x * dt;
                    shurikens[i].rect.point_2.x += shurikens[i].velocity.x * dt;
                    shurikens[i].rect.point_1.y += shurikens[i].velocity.y * dt;
                    shurikens[i].rect.point_2.y += shurikens[i].velocity.y * dt;

                    if (shurikens[i].reflected) {
                        if (Math::IntersectsRect(shurikens[i].rect, GetCurrentRect())) {
                            health -= 2;
                            hitCooldownSec = 0.25;
                            shurikens[i].active = false;
                            continue;
                        }
                    }
                    else {
                        if (Math::IntersectsRect(shurikens[i].rect, player.GetRect()))
                        {
                            auto* pNC = const_cast<Player*>(&player);
                            if (pNC->HandleParry())
                            {
                                if (pNC->IsSlowMoActive()) {
                                    shurikens[i].reflected = true;
                                    shurikens[i].velocity.x *= -1.5;
                                    shurikens[i].velocity.y *= -1.5;
                                    shurikens[i].rotation += 3.1415926535;
                                }
                                else {
                                    shurikens[i].active = false;
                                }
                            }
                            else if (pNC->IsRolling()) {
                                if (!pNC->move.dodgeRewardClaimed) {
                                    pNC->AddTimeGauge(pNC->gaugeGainRoll);
                                    pNC->move.dodgeRewardClaimed = true;
                                }
                            }
                            else if (pNC->CanTakeHit() && !pNC->IsInvincible())
                            {
                                pNC->TakeHit();
                                shurikens[i].active = false;
                            }
                            continue;
                        }
                    }

                    bool hitWall = false;
                    for (auto* obj : stage->staticObjects)
                    {
                        if (obj->Type() == GameObjectTypes::Platform)
                        {
                            if (auto* rc = obj->GetGOComponent<CS230::RectCollision>())
                            {
                                if (Math::IntersectsRect(shurikens[i].rect, rc->WorldBoundary()))
                                {
                                    hitWall = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (hitWall)
                    {
                        shurikens[i].active = false;
                        continue;
                    }

                    if (shurikens[i].rect.Right() < -1000.0 || shurikens[i].rect.Left() > mapW + 1000.0 || shurikens[i].rect.Top() < -1000.0 || shurikens[i].rect.Bottom() > 3000.0)
                    {
                        shurikens[i].active = false;
                    }
                }
            }
        }

        Math::vec2 syncPos;
        syncPos.y = rect.Bottom();

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            Math::vec2 curSize = rect.Size();
            Math::ivec2 fs = spr->GetFrameSize();
            if (fs.x > 0 && fs.y > 0)
            {
                Math::vec2 sprScale;
                sprScale.x = curSize.x / static_cast<double>(fs.x);
                sprScale.y = curSize.y / static_cast<double>(fs.y);

                if (moveDir < 0)
                {
                    sprScale.x = -sprScale.x;
                    syncPos.x = (rect.Right() + rect.Left()) * 0.5;
                }
                else 
                {
                    syncPos.x = (rect.Left() + rect.Right()) * 0.5;
                }
                SetScale(sprScale);
            }
        }
        else 
        {
            syncPos.x = (rect.Left() + rect.Right()) * 0.5;
        }

        SetPosition(syncPos);
        UpdateGOComponents(dt);

        if (swordAttackEffect.GetGOComponent<CS230::Sprite>()) 
        {
            swordAttackEffect.GetGOComponent<CS230::Sprite>()->Update(dt);
        }
    }

    void CityBoss::Draw(Math::TransformationMatrix camM)
    {
        if (!active || health <= 0) return;

        if (auto spr = GetGOComponent<CS230::Sprite>()) {
            if (hitCooldownSec > 0.0) {
                spr->Draw(camM * GetMatrix(), GAME200::WHITE);
            }
            else {
                CS230::GameObject::Draw(camM);
            }
        }
        else {
            Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(Center()) * Math::ScaleMatrix(rect.Size()), GAME200::PURPLE, GAME200::BLACK, 1.0);
        }

        bool showDebugBoxes = false;
        if (auto* sc = Engine::GetGameStateManager().GetGSComponent<CS230::ShowCollision>()) {
            showDebugBoxes = sc->Enabled();
        }

        if (currentState == CBState::Sword_Attack) {
            if (swordAttackEffect.GetGOComponent<CS230::Sprite>()) {
                Math::ivec2 fs = swordAttackEffect.GetGOComponent<CS230::Sprite>()->GetFrameSize();
                Math::vec2 texSize;
                texSize.x = static_cast<double>(fs.x);
                texSize.y = static_cast<double>(fs.y);

                Math::vec2 slashScale;
                slashScale.x = swordAttackRect.Size().x / texSize.x;
                slashScale.y = swordAttackRect.Size().y / texSize.y;

                Math::vec2 c2;
                c2.y = swordAttackRect.Bottom() + 50.0;

                if (moveDir < 0) {
                    slashScale.x = -slashScale.x;
                    c2.x = swordAttackRect.Right();
                }
                else {
                    c2.x = swordAttackRect.Left();
                }

                Math::TransformationMatrix m = camM * Math::TranslationMatrix(c2) * Math::ScaleMatrix(slashScale);
                swordAttackEffect.GetGOComponent<CS230::Sprite>()->Draw(m);

                swordAttackEffect.SetPosition(c2);
                swordAttackEffect.SetScale(slashScale);
            }
        }

        if (showDebugBoxes) {
            if (auto col = GetGOComponent<CS230::RectCollision>()) {
                col->Draw(camM);
            }
            if (currentState == CBState::Sword_Attack) {
                if (auto col = swordAttackEffect.GetGOComponent<CS230::RectCollision>()) {
                    col->Draw(camM);
                }
            }

            if (currentState == CBState::Sword_AttackWait && swordTelegraphTimer > 0.0) {
                Math::vec2 c1Center;
                c1Center.x = swordAttackRect.Left() + swordAttackRect.Size().x * 0.5;
                c1Center.y = swordAttackRect.Bottom() + swordAttackRect.Size().y * 0.5;
                Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(c1Center) * Math::ScaleMatrix(swordAttackRect.Size()), 0x80FF0000u, 0x80FF0000u, 0.0);
            }

            if (currentState == CBState::Dash_Charge) {
                double mapWidth = 1920.0;
                if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState())) {
                    mapWidth = static_cast<double>(stage->worldWidthCur);
                }

                Math::vec2 sweepSize;
                sweepSize.x = mapWidth;
                sweepSize.y = Height();

                Math::vec2 c3;
                c3.x = mapWidth * 0.5;
                c3.y = rect.Bottom() + Height() * 0.5;

                Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(c3) * Math::ScaleMatrix(sweepSize), 0x5000FF00u, 0x5000FF00u, 0.0);
            }
        }

        static std::shared_ptr<CS230::Texture> arrowTex;
        if (!arrowTex) arrowTex = Engine::GetTextureManager().Load("Assets/image/Objects/Arrow.PNG");

        for (int i = 0; i < 100; ++i) {
            if (shurikens[i].active) {
                Math::vec2 os = shurikens[i].rect.Size();
                Math::vec2 oc;
                oc.x = shurikens[i].rect.Left() + os.x * 0.5;
                oc.y = shurikens[i].rect.Bottom() + os.y * 0.5;

                if (arrowTex) {
                    Math::vec2 texSize;
                    texSize.x = static_cast<double>(arrowTex->GetSize().x);
                    texSize.y = static_cast<double>(arrowTex->GetSize().y);

                    Math::vec2 arrowScale;
                    arrowScale.x = os.x / texSize.x;
                    arrowScale.y = os.y / texSize.y;

                    Math::TransformationMatrix m = camM * Math::TranslationMatrix(oc) * Math::RotationMatrix(shurikens[i].rotation) * Math::ScaleMatrix(arrowScale);
                    arrowTex->Draw(m);
                }
                else {
                    Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(oc) * Math::RotationMatrix(shurikens[i].rotation) * Math::ScaleMatrix(os), GAME200::YELLOW, GAME200::YELLOW, 1.0);
                }
            }
        }

        DrawStatus(camM);
    }
}