#include "Enemy.h"
#include "StoryItem.h"
#include "Player.h"
#include "Scrap.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../OpenGL/RGBA.h"
#include "../Game/Gravity.h"
#include <algorithm>
#include <cmath>
#include "../Stage/StageManager.h"

namespace CS230
{
    Enemy::Enemy()
    {
        RegisterEnemy();
    }

    Enemy::~Enemy()
    {
        UnregisterEnemy();
    }

    const Math::rect& Enemy::GetRect() const
    {
        if (auto go = dynamic_cast<const CS230::GameObject*>(this))
        {
            if (auto rc = const_cast<CS230::GameObject*>(go)->GetGOComponent<CS230::RectCollision>())
            {
                collisionRectCache = rc->WorldBoundary();
                return collisionRectCache;
            }
        }
        collisionRectCache = rect;
        return collisionRectCache;
    }

    void Enemy::DrawStatus(Math::TransformationMatrix camM)
    {
        DrawHP(camM);
        if (groggyTimer > 0.0)
        {
            if (!groggyTexture) groggyTexture = Engine::GetFont(0).PrintToTextureScaled("Groggy", GAME200::RED, 0.5);
            if (groggyTexture)
            {
                double x = Center().x - groggyTexture->GetSize().x * 0.5;
                double y = rect.Top() + 20.0;
                groggyTexture->Draw(camM * Math::TranslationMatrix(Math::vec2{ x, y }));
            }
        }
    }

    std::vector<Enemy*>& Enemy::Registry()
    {
        return AllEnemies();
    }

    void Enemy::DrawTelegraph(const Math::TransformationMatrix& camM) const
    {
        static std::shared_ptr<CS230::Texture> tex = nullptr;
        if (!tex) tex = Engine::GetFont(0).PrintToTextureScaled("!", 0xFF0000FF, 1.0);
        if (tex)
        {
            double x = Center().x;
            double y = rect.Top() - 30;
            tex->Draw(camM * Math::TranslationMatrix(Math::vec2{ x, y }));
        }
    }

    void Enemy::DrawHP(const Math::TransformationMatrix& camM) const
    {
#ifdef _DEBUG
        bool showDebug = false;
        if (auto sc = Engine::GetGameStateManager().GetGSComponent<CS230::ShowCollision>()) showDebug = sc->Enabled();
        if (!showDebug) return;
#else
        return;
#endif
        if (health <= 0) return;
        if (lastHP != health || !hpTexture)
        {
            hpTexture = Engine::GetFont(0).PrintToTextureScaled("HP: " + std::to_string(health), GAME200::BLACK, 0.5);
            lastHP = health;
        }
        if (hpTexture) hpTexture->Draw(camM * Math::TranslationMatrix(Math::vec2{ rect.Left(), rect.Top() + 10 }));
    }

    void Enemy::ApplyGravity(double dt)
    {
        lastDt = dt;

        if (isGrounded) isDraggedByHammer = false;

        constexpr double MAX_VELOCITY_X = 1400.0;
        constexpr double MAX_VELOCITY_Y = 1400.0;
        constexpr double MAX_FALL_VELOCITY = -1400.0;

        if (velocity.x > MAX_VELOCITY_X) velocity.x = MAX_VELOCITY_X;
        if (velocity.x < -MAX_VELOCITY_X) velocity.x = -MAX_VELOCITY_X;
        if (velocity.y > MAX_VELOCITY_Y) velocity.y = MAX_VELOCITY_Y;
        if (velocity.y < MAX_FALL_VELOCITY) velocity.y = MAX_FALL_VELOCITY;

        if (std::abs(velocity.x) > 1e-6)
        {
            double dx = velocity.x * dt;
            double clampedDx = ApplyPlatformEdge(dx);

            auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
            if (sm)
            {
                Math::rect cr = GetRect();
                for (auto* obj : sm->staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform)
                    {
                        auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                        if (rc)
                        {
                            Math::rect pRect = rc->WorldBoundary();
                            if (cr.Bottom() < pRect.Top() - 2.0 && cr.Top() > pRect.Bottom() + 2.0)
                            {
                                if (clampedDx > 0.0 && cr.Right() <= pRect.Left() && cr.Right() + clampedDx > pRect.Left())
                                {
                                    clampedDx = pRect.Left() - cr.Right(); 
                                }
                                else if (clampedDx < 0.0 && cr.Left() >= pRect.Right() && cr.Left() + clampedDx < pRect.Right())
                                {
                                    clampedDx = pRect.Right() - cr.Left(); 
                                }
                            }
                        }
                    }
                }
            }
            if (std::abs(clampedDx) < std::abs(dx)) velocity.x = 0.0;

            rect.point_1.x += clampedDx;
            rect.point_2.x += clampedDx;

            const double damping = isGrounded ? 0.90 : 0.99;
            velocity.x *= std::pow(damping, dt * 60.0);
            if (std::abs(velocity.x) < 0.5) velocity.x = 0.0;
        }

        if (!isGrounded)
        {
            double grav = 2400.0;
            if (auto g = Engine::GetGameStateManager().GetGSComponent<Gravity>()) grav = g->GetValue();
            velocity.y -= grav * dt;

            if (velocity.y < MAX_FALL_VELOCITY) velocity.y = MAX_FALL_VELOCITY;

            rect.point_1.y += velocity.y * dt;
            rect.point_2.y += velocity.y * dt;
        }
    }

    double Enemy::ApplyPlatformEdge(double dx)
    {
        if (!hasStandingRect) return dx;

        Math::rect cr = GetRect();
        double nextLeft = cr.Left() + dx;
        double nextRight = cr.Right() + dx;

        bool outOfCurrent = false;
        if (dx < 0 && nextLeft < standingRect.Left()) outOfCurrent = true;
        else if (dx > 0 && nextRight > standingRect.Right()) outOfCurrent = true;

        if (outOfCurrent)
        {
            bool foundNextPlatform = false;
            auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
            if (sm)
            {
                double checkX = (dx < 0) ? nextLeft : nextRight;

                for (auto* obj : sm->staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
                    {
                        auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                        if (rc)
                        {
                            Math::rect pRect = rc->WorldBoundary();

                            if (checkX >= pRect.Left() && checkX <= pRect.Right() &&
                                std::abs(pRect.Top() - standingRect.Top()) < 10.0)
                            {
                                foundNextPlatform = true;
                                standingRect = pRect; 
                                break;
                            }
                        }
                    }
                }
            }

            if (!foundNextPlatform)
            {
                if (dx < 0)
                {
                    if (cr.Left() <= standingRect.Left()) dx = 0.0;
                    else dx = standingRect.Left() - cr.Left();
                    moveDir = 1;
                }
                else if (dx > 0)
                {
                    if (cr.Right() >= standingRect.Right()) dx = 0.0;
                    else dx = standingRect.Right() - cr.Right(); 
                    moveDir = -1;
                }
            }
        }

        return dx;
    }

    void Enemy::ApplyStunAndKnockbackFromPlayer(const Player& player, double stunSec, double knockback, bool pull)
    {
        if (isParryGroggy)
        {
            groggyTimer = std::max(groggyTimer, stunSec);
            isParryGroggy = false;
            OnStun();
        }
        int dir = (player.Center().x >= Center().x) ? -1 : 1;

        if (pull)
        {
            dir = -dir;
        }

        velocity.x = (knockback / 0.25) * dir;
    }

    void Enemy::ResolveCollision(GameObjectTypes type, const void* object, bool aux, double dt)
    {
        (void)aux;
        (void)dt;

        if (type != GameObjectTypes::Platform || !active || !object) return;

        const Math::rect* platform = static_cast<const Math::rect*>(object);
        const Math::rect cr = GetRect();

        if (Math::IntersectsRect(cr, *platform))
        {
            double realDt = lastDt;
            if (realDt <= 0.0) realDt = 0.016;

            double prevBottom = cr.Bottom() - velocity.y * realDt;
            double prevTop = cr.Top() - velocity.y * realDt;
            double prevRight = cr.Right() - velocity.x * realDt;
            double prevLeft = cr.Left() - velocity.x * realDt;

            bool wasAbove = prevBottom >= platform->Top() - 0.5;
            bool wasBelow = prevTop <= platform->Bottom() + 0.5;
            bool wasLeft = prevRight <= platform->Left() + 0.5;
            bool wasRight = prevLeft >= platform->Right() - 0.5;

            double oL = cr.Right() - platform->Left();
            double oR = platform->Right() - cr.Left();
            double oB = cr.Top() - platform->Bottom();
            double oT = platform->Top() - cr.Bottom();

            double dx = (oL < oR) ? -oL : oR;
            double dy = (oB < oT) ? -oB : oT;

            bool resolveX = false;
            bool resolveY = false;

            if (wasAbove && velocity.y <= 0.0)
            {
                dy = oT;
                resolveY = true;
            }
            else if (wasBelow && velocity.y >= 0.0)
            {
                dy = -oB;
                resolveY = true;
            }
            else if (wasLeft && velocity.x >= 0.0)
            {
                dx = -oL;
                resolveX = true;
            }
            else if (wasRight && velocity.x <= 0.0)
            {
                dx = oR;
                resolveX = true;
            }

            if (!resolveX && !resolveY)
            {
                if (std::abs(dx) < std::abs(dy)) resolveX = true;
                else resolveY = true;
            }

            if (resolveY && !resolveX)
            {
                rect.point_1.y += dy;
                rect.point_2.y += dy;
                velocity.y = 0.0;
                if (dy > 0.0)
                {
                    isGrounded = true;
                    isDraggedByHammer = false;
                    hasStandingRect = true;
                    standingRect = *platform;
                    standingOn = nullptr;
                }
                else if (dy < 0.0 && velocity.y > 0.0)
                {
                    velocity.y = 0.0;
                }
            }
            else if (resolveX && !resolveY)
            {
                rect.point_1.x += dx;
                rect.point_2.x += dx;
                velocity.x = 0.0;
                if (hitCooldownSec <= 0.0 && playerHitCooldownSec <= 0.0 && !IsGroggy())
                {
                    if (dx > 0 && moveDir < 0) moveDir = 1;
                    else if (dx < 0 && moveDir > 0) moveDir = -1;
                }
            }
        }
        else if (hasStandingRect &&
            standingRect.Left() == platform->Left() &&
            standingRect.Right() == platform->Right() &&
            standingRect.Top() == platform->Top() &&
            standingRect.Bottom() == platform->Bottom())
        {
            if (cr.Right() <= platform->Left() || cr.Left() >= platform->Right())
            {
                hasStandingRect = false;
                isGrounded = false;
            }
        }
        allowLeavePlatform = false;
    }

    void Enemy::NotifyHit(const Player* p)
    {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/enumhit.wav");
        if (p)
        {
            aggroLockTimer = 2.0;
            ForceAggro(*p);
        }
    }

    void Enemy::ApplyKnockback(double amount, int dir)
    {
        velocity.x = (amount / 0.25) * dir;
        if (velocity.y < 0.0) velocity.y = 0.0;
        velocity.y += 120.0;
        isGrounded = false;
    }

    void Enemy::SpawnDeathReward()
    {
        auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
        if (sm)
        {
            if (IsBoss())
            {
                auto* player = sm->GetPlayerPtr();
                if (player->GetStoryFragments() >= 3)
                {
                    auto* item = new CS230::StoryItem(rect.point_1 + Math::vec2{ 0, 20 }, true);
                    sm->items.push_back(item);
                }
                else
                {
                    player->combat.showMissingFragmentsUI = true;
                    player->combat.storyUITimer = 5.0; 
                }
            }
            else if (dropsStoryItem)
            {
                auto* item = new CS230::StoryItem(rect.point_1 + Math::vec2{ 0, 20 });
                sm->items.push_back(item);
            }
            else
            {
                auto* item = new CS230::Scrap(rect.point_1 + Math::vec2{ 0, 20 });
                sm->items.push_back(item);
            }
        }
    }

    Math::vec2 Delta(const Enemy& e, const Player& p)
    {
        return { p.Center().x - e.Center().x, p.Center().y - e.Center().y };
    }

    double DistanceSq(const Enemy& e, const Player& p)
    {
        Math::vec2 d = Delta(e, p);
        return d.x * d.x + d.y * d.y;
    }

    bool IsBlockedByPlatform(const Math::vec2& p1, const Math::vec2& p2)
    {
        auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
        if (!sm) return false;

        Math::rect lineRect{ { std::min(p1.x, p2.x), std::min(p1.y, p2.y) }, { std::max(p1.x, p2.x), std::max(p1.y, p2.y) } };
        if (lineRect.Size().y < 2.0)
        {
            lineRect.point_1.y -= 1.0;
            lineRect.point_2.y += 1.0;
        }
        if (lineRect.Size().x < 2.0)
        {
            lineRect.point_1.x -= 1.0;
            lineRect.point_2.x += 1.0;
        }

        for (auto* obj : sm->staticObjects)
        {
            if (obj->Type() == GameObjectTypes::Platform)
            {
                if (auto* rc = obj->GetGOComponent<CS230::RectCollision>())
                {
                    if (Math::IntersectsRect(lineRect, rc->WorldBoundary())) return true;
                }
            }
        }
        return false;
    }
}
