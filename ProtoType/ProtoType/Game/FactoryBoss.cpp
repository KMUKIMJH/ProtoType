#include "FactoryBoss.h"
#include "Player.h"
#include "Scrap.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../Game/Gravity.h"
#include "../OpenGL/RGBA.h"
#include "../Stage/StageManager.h"
#include <cmath>
#include <GL/glew.h>

namespace CS230
{
    void FactoryBoss::Load(const Math::vec2& position_, const Math::vec2& size, int initialHealth)
    {
        (void)size;
        health = initialHealth;
        maxHealth = initialHealth;

        currentState = FB1State::Idle;
        stateTimer = 4.0;
        jumpHitApplied = false;

        waves[0].active = false;
        waves[1].active = false;

        strikeCount = 0;
        strikeActive = false;
        strikeApplied = false;
        strikeTimer = 0.0;
        strikeRect = Math::rect{ { 0, 0 }, { 0, 0 } };

        active = true;

        AddGOComponent(new CS230::Sprite("Assets/image/Bosses/FactoryBoss/FactoryBoss.spt", this));
        strikeEffect.AddGOComponent(new CS230::Sprite("Assets/image/Bosses/FactoryBoss/BossATK.spt", &strikeEffect));

        for (int i = 0; i < 2; ++i)
        {
            waves[i].effect.AddGOComponent(new CS230::Sprite("Assets/image/Bosses/FactoryBoss/BossWaveATK.spt", &waves[i].effect));
        }

        Math::vec2 finalSize{ 150, 240 };
        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            spr->PlayAnimation(0);
            Math::ivec2 fs = spr->GetFrameSize();
            if (fs.x > 0 && fs.y > 0)
            {
                finalSize = { static_cast<double>(fs.x * 1.5), static_cast<double>(fs.y * 2.4) };
            }
        }

        rect.point_1 = position_;
        rect.point_2 = { position_.x + finalSize.x, position_.y + finalSize.y };
        SetPosition({ rect.Left(), rect.Bottom() });
    }

    void FactoryBoss::Unload()
    {
        ClearGOComponents();
        strikeEffect.ClearGOComponents();
        waves[0].effect.ClearGOComponents();
        waves[1].effect.ClearGOComponents();
        active = false;
        waves[0].active = false;
        waves[1].active = false;
        strikeActive = false;
        health = 0;
        rect = Math::rect{ { -100000.0, -100000.0 }, { -99900.0, -99900.0 } };
    }

    void FactoryBoss::Update(const CS230::Player& player, double dt, bool isSlow)
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
                if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
                {
                    auto* item = new CS230::Scrap(Center());
                    stage->items.push_back(item);
                }
                active = false;
            }
            return;
        }

        for (int i = 0; i < 2; ++i)
        {
            if (!waves[i].active)
            {
                continue;
            }

            waves[i].rect.point_1.x += waves[i].velocityX * dt;
            waves[i].rect.point_2.x += waves[i].velocityX * dt;

            if (Math::IntersectsRect(waves[i].rect, player.GetRect()))
            {
                auto* pNC = const_cast<Player*>(&player);
                if (pNC->HandleParry())
                {
                    waves[i].active = false;
                }
                else if (pNC->IsRolling()) {
                    if (!pNC->move.dodgeRewardClaimed) {
                        pNC->AddTimeGauge(pNC->gaugeGainRoll);
                        pNC->move.dodgeRewardClaimed = true;
                    }
                }
                else if (pNC->CanTakeHit())
                {
                    pNC->TakeHit();
                    pNC->ApplyKnockback(600.0, (waves[i].velocityX > 0) ? 1 : -1);
                    waves[i].active = false;
                }
            }

            if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
            {
                for (CS230::GameObject* obj : stage->staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform)
                    {
                        if (auto* rc = obj->GetGOComponent<CS230::RectCollision>())
                        {
                            if (Math::IntersectsRect(waves[i].rect, rc->WorldBoundary()))
                            {
                                waves[i].active = false;
                                break;
                            }
                        }
                    }
                }
            }

            if (waves[i].active)
            {
                if (auto spr = waves[i].effect.GetGOComponent<CS230::Sprite>())
                {
                    Math::ivec2 fs = spr->GetFrameSize();
                    if (fs.x > 0 && fs.y > 0)
                    {
                        Math::vec2 curSize = waves[i].rect.Size();
                        Math::vec2 newScale{ curSize.x / static_cast<double>(fs.x), curSize.y / static_cast<double>(fs.y) };

                        if (waves[i].velocityX < 0.0)
                        {
                            newScale.x = -newScale.x;
                            waves[i].effect.SetPosition({ waves[i].rect.Right(), waves[i].rect.Bottom() });
                        }
                        else
                        {
                            waves[i].effect.SetPosition({ waves[i].rect.Left(), waves[i].rect.Bottom() });
                        }
                        waves[i].effect.SetScale(newScale);
                    }
                }
                waves[i].effect.Update(dt);
            }
        }

        bool isPhase2 = (health <= maxHealth / 2);
        double gVal = 2400.0;
        if (auto g = Engine::GetGameStateManager().GetGSComponent<Gravity>())
        {
            gVal = g->GetValue();
        }

        if (currentState == FB1State::Idle)
        {
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = FB1State::Crouch;
                stateTimer = 2.0;
                this->Enemy::velocity = { 0.0, 0.0 };

                if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);
            }
        }
        else if (currentState == FB1State::Crouch)
        {
            stateTimer -= dt;
            if (stateTimer <= 0.0)
            {
                currentState = FB1State::Jump;
                jumpHitApplied = false;

                double jumpH = Player::DefaultHeight * 4.0;
                this->Enemy::velocity.y = std::sqrt(2.0 * gVal * jumpH);

                double timeToPeak = this->Enemy::velocity.y / gVal;
                double timeToFall = timeToPeak;
                double totalTime = timeToPeak + timeToFall;
                double dist = player.Center().x - Center().x;

                this->Enemy::velocity.x = dist / totalTime;
                isGrounded = false;
                hasStandingRect = false;
            }
        }
        else if (currentState == FB1State::Jump)
        {
            this->Enemy::velocity.y -= gVal * dt;

            rect.point_1.x += this->Enemy::velocity.x * dt;
            rect.point_2.x += this->Enemy::velocity.x * dt;
            rect.point_1.y += this->Enemy::velocity.y * dt;
            rect.point_2.y += this->Enemy::velocity.y * dt;

            if (!jumpHitApplied && Math::IntersectsRect(rect, player.GetRect()))
            {
                auto* pNC = const_cast<Player*>(&player);
                if (pNC->HandleParry())
                {
                    jumpHitApplied = true;
                }
                else if (pNC->IsRolling()) {
                    if (!pNC->move.dodgeRewardClaimed) {
                        pNC->AddTimeGauge(pNC->gaugeGainRoll);
                        pNC->move.dodgeRewardClaimed = true;
                    }
                }
                else if (pNC->CanTakeHit())
                {
                    pNC->TakeHit();
                    pNC->ApplyKnockback(800.0, (player.Center().x >= Center().x) ? 1 : -1, 600.0);
                    jumpHitApplied = true;
                }
            }

            bool landed = false;
            double landY = 0.0;
            if (auto* stage = dynamic_cast<::StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
            {
                for (CS230::GameObject* obj : stage->staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform)
                    {
                        Math::rect r = obj->GetGOComponent<CS230::RectCollision>()->WorldBoundary();
                        if (this->Enemy::velocity.y < 0 && rect.Right() > r.Left() && rect.Left() < r.Right() &&
                            rect.Bottom() <= r.Top() && (rect.Bottom() - this->Enemy::velocity.y * dt) >= r.Top())
                        {
                            landed = true;
                            landY = r.Top();
                            break;
                        }
                    }
                }
            }

            if (landed)
            {
                double h = rect.Top() - rect.Bottom();
                rect.point_1.y = landY;
                rect.point_2.y = landY + h;
                this->Enemy::velocity = { 0.0, 0.0 };
                isGrounded = true;

                double waveHeight = Player::DefaultHeight * 1.3;

                waves[0].active = true;
                waves[0].velocityX = -800.0;
                waves[0].rect = { { Center().x - 60.0, rect.Bottom() }, { Center().x, rect.Bottom() + waveHeight } };
                if (auto spr = waves[0].effect.GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);

                waves[1].active = true;
                waves[1].velocityX = 800.0;
                waves[1].rect = { { Center().x, rect.Bottom() }, { Center().x + 60.0, rect.Bottom() + waveHeight } };
                if (auto spr = waves[1].effect.GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);

                if (isPhase2)
                {
                    currentState = FB1State::Strike;
                    strikeCount = 0;
                    stateTimer = 0.5;

                    if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(2);
                }
                else
                {
                    currentState = FB1State::Idle;
                    stateTimer = 4.0;

                    if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);
                }
            }
        }
        else if (currentState == FB1State::Wave)
        {
            currentState = FB1State::Idle;
            stateTimer = 4.0;
        }
        else if (currentState == FB1State::Strike)
        {
            if (stateTimer > 0.0)
            {
                stateTimer -= dt;
            }
            else
            {
                if (!strikeActive)
                {
                    strikeActive = true;
                    strikeTimer = 0.4;
                    strikeApplied = false;

                    if (auto spr = strikeEffect.GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);

                    double w = Player::DefaultWidth * 5.0;
                    double h = Player::DefaultHeight * 0.5;
                    double bottom = rect.Bottom() + 50.0;

                    double bossVisualCenter = (rect.Left() + rect.Right()) * 0.5;
                    double bossHalfWidth = rect.Size().x * 0.5;

                    double bossLeftEdge = bossVisualCenter - bossHalfWidth;
                    double bossRightEdge = bossVisualCenter + bossHalfWidth;

                    if (player.Center().x >= bossVisualCenter)
                    {
                        strikeRect = { { bossRightEdge, bottom }, { bossRightEdge + w, bottom + h } };
                    }
                    else
                    {
                        strikeRect = { { bossLeftEdge - w, bottom }, { bossLeftEdge, bottom + h } };
                    }
                }
                else
                {
                    strikeTimer -= dt;
                    if (!strikeApplied && Math::IntersectsRect(strikeRect, player.GetRect()))
                    {
                        auto* pNonConst = const_cast<Player*>(&player);
                        if (pNonConst->HandleParry())
                        {
                            strikeApplied = true;
                        }
                        else if (pNonConst->IsRolling()) {
                            if (!pNonConst->move.dodgeRewardClaimed) {
                                pNonConst->AddTimeGauge(pNonConst->gaugeGainRoll);
                                pNonConst->move.dodgeRewardClaimed = true;
                            }
                        }
                        else if (pNonConst->CanTakeHit())
                        {
                            pNonConst->TakeHit();
                            pNonConst->ApplyKnockback(500.0, pNonConst->Center().x >= strikeRect.Left() + (strikeRect.Size().x * 0.5) ? 1 : -1, 300.0);
                            strikeApplied = true;
                        }
                    }

                    if (strikeTimer <= 0.0)
                    {
                        strikeActive = false;
                        strikeCount++;
                        if (strikeCount >= 3)
                        {
                            currentState = FB1State::Idle;
                            stateTimer = 4.0;

                            if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);
                        }
                        else
                        {
                            stateTimer = 0.5;
                        }
                    }
                }
            }
        }

        if (strikeActive)
        {
            if (auto spr = strikeEffect.GetGOComponent<CS230::Sprite>())
            {
                Math::ivec2 fs = spr->GetFrameSize();
                if (fs.x > 0 && fs.y > 0)
                {
                    Math::vec2 curSize = strikeRect.Size();
                    Math::vec2 newScale{ curSize.x / static_cast<double>(fs.x), curSize.y / static_cast<double>(fs.y) };

                    if (strikeRect.Left() < rect.Left())
                    {
                        newScale.x = -newScale.x;
                        strikeEffect.SetPosition({ strikeRect.Right(), strikeRect.Bottom() });
                    }
                    else
                    {
                        strikeEffect.SetPosition({ strikeRect.Left(), strikeRect.Bottom() });
                    }
                    strikeEffect.SetScale(newScale);
                }
            }
            strikeEffect.Update(dt);
        }

        bool isFacingRight = true;
        if (strikeActive) {
            isFacingRight = (strikeRect.Left() >= rect.Left());
        }
        else if (currentState == FB1State::Jump) {
            isFacingRight = (this->Enemy::velocity.x >= 0.0);
        }
        else {
            isFacingRight = (player.Center().x >= Center().x);
        }

        Math::vec2 drawPos = { rect.Left(), rect.Bottom() };

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            Math::ivec2 fs = spr->GetFrameSize();
            if (fs.x > 0 && fs.y > 0)
            {
                Math::vec2 curSize = rect.Size();
                Math::vec2 newScale{ curSize.x / static_cast<double>(fs.x), curSize.y / static_cast<double>(fs.y) };

                if (currentState == FB1State::Crouch)
                {
                    newScale.y *= 0.6;
                }

                if (!isFacingRight)
                {
                    newScale.x = -newScale.x;
                    drawPos.x = rect.Right();
                }

                SetScale(newScale);
            }
        }

        SetPosition(drawPos);
        UpdateGOComponents(dt);
    }

    void FactoryBoss::Draw(Math::TransformationMatrix camM)
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
            Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(Center()) * Math::ScaleMatrix(Math::vec2{ Width(), Height() }), GAME200::DKBLUE, GAME200::BLACK, 1.0);
        }

        for (int i = 0; i < 2; ++i)
        {
            if (waves[i].active)
            {
                waves[i].effect.Draw(camM);
            }
        }

        if (strikeActive)
        {
            strikeEffect.Draw(camM);
        }
    }
}