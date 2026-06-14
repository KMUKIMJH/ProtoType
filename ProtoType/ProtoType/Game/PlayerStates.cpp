#include "PlayerStates.h"
#include "Player.h"
#include "Weapon.h"
#include "../Engine/Engine.h"
#include "../Game/Gravity.h"
#include "../Stage/StageManager.h"
#include <cmath>

namespace
{
    static bool can_control(const CS230::Player* p)
    {
        return p->GetGroggyTimer() <= 0.0;
    }
}

namespace CS230
{
    // IDLE STATE
    void PlayerState_Idle::Enter(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        p->move.isRolling = false;
        p->move.isDashing = false;
        p->move.airMoveSignLock = 0;
        p->move.airFlipUsed = false;

        if (auto spr = p->GetGOComponent<CS230::Sprite>())
        {
            if (!(p->combat.attackActive && p->currentSpritePath == "Assets/image/Player/PlayerSword.spt"))
            {
                spr->PlayAnimation(0);
            }
        }
    }

    void PlayerState_Idle::Update(CS230::GameObject* o, double dt)
    {
        auto* p = static_cast<Player*>(o);
        bool control = can_control(p);

        if (control && p->combat.health > 0 && p->GetWeapon())
        {
            p->GetWeapon()->TryStartAttack(*p);
        }

        p->move.requestDropThrough = false;
        if (control && p->input.dropThrough)
        {
            p->move.requestDropThrough = true;
        }

        p->update_horizontal(dt, true);
    }

    void PlayerState_Idle::CheckExit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        if (!can_control(p))
        {
            return;
        }

        if (p->input.dropThrough)
        {
            p->move.isGrounded = false;
            p->move.standingOn = nullptr;
            p->move.hasStandingRect = false;

            const double fallMin = -2.0 * 60.0;
            if (p->move.velocity.y > fallMin)
            {
                p->move.velocity.y = fallMin;
            }

            p->change_state(&p->state_falling);
            return;
        }

        if (p->input.rollJustPressed && p->move.isGrounded && p->move.rollCooldown <= 0.0 && p->combat.groggyTimer <= 0.0)
        {
            p->change_state(&p->state_rolling);
            return;
        }

        bool onPlatform = p->IsSupportedOnGround();

        if (onPlatform && p->move.jumpBufferTimer > 0.0)
        {
            p->move.jumpBufferTimer = 0.0;
            p->change_state(&p->state_jumping);
            return;
        }

        if (!onPlatform)
        {
            p->move.standingOn = nullptr;
            p->move.hasStandingRect = false;
            p->move.isGrounded = false;
            p->change_state(&p->state_falling);
            return;
        }

        if (p->input.moveDir != 0)
        {
            p->change_state(&p->state_running);
            return;
        }
    }

    // RUNNING STATE
    void PlayerState_Running::Enter(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        p->move.airMoveSignLock = 0;
        p->move.airFlipUsed = false;

        if (auto spr = p->GetGOComponent<CS230::Sprite>())
        {
            if (!(p->combat.attackActive && p->currentSpritePath == "Assets/image/Player/PlayerSword.spt"))
            {
                spr->PlayAnimation(1);
            }
        }
    }

    void PlayerState_Running::Update(CS230::GameObject* o, double dt)
    {
        auto* p = static_cast<Player*>(o);
        bool control = can_control(p);

        if (control && p->combat.health > 0 && p->GetWeapon())
        {
            p->GetWeapon()->TryStartAttack(*p);
        }
        p->update_horizontal(dt, true);
    }

    void PlayerState_Running::CheckExit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        if (!can_control(p))
        {
            return;
        }

        if (p->input.dropThrough)
        {
            p->move.isGrounded = false;
            p->move.standingOn = nullptr;
            p->move.hasStandingRect = false;
            const double fallMin = -2.0 * 60.0;
            if (p->move.velocity.y > fallMin)
            {
                p->move.velocity.y = fallMin;
            }
            p->change_state(&p->state_falling);
            return;
        }

        if (p->input.rollJustPressed && p->move.isGrounded && p->move.rollCooldown <= 0.0 && p->combat.groggyTimer <= 0.0)
        {
            p->change_state(&p->state_rolling);
            return;
        }

        bool onPlatform = p->IsSupportedOnGround();

        if (onPlatform && p->move.jumpBufferTimer > 0.0)
        {
            p->move.jumpBufferTimer = 0.0;
            p->change_state(&p->state_jumping);
            return;
        }

        if (!onPlatform)
        {
            p->move.standingOn = nullptr;
            p->move.hasStandingRect = false;
            p->move.isGrounded = false;
            p->change_state(&p->state_falling);
            return;
        }

        if (p->input.moveDir == 0)
        {
            p->change_state(&p->state_idle);
            return;
        }
    }

    // JUMPING STATE
    void PlayerState_Jumping::Enter(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);

        double gval = 0.0;
        if (auto grav = Engine::GetGameStateManager().GetGSComponent<Gravity>())
        {
            gval = grav->GetValue();
        }

        const double desiredHeight = (Player::DefaultHeight * 2.0) * p->move.speedMultiplier * (1.0 + p->combat.jumpHeightBonus);

        double v0 = p->jumpVelocity;
        if (gval > 0.0)
        {
            v0 = std::sqrt(2.0 * (gval * 0.8) * desiredHeight);
        }

        p->move.velocity.y = v0;

        if (p->move.isGrounded || p->move.coyoteTimer > 0.0)
        {
            p->move.jumpCount = 1;
            p->move.airMoveSignLock = p->input.moveDir;
            if (p->move.airMoveSignLock == 0)
            {
                if (p->move.velocity.x < 0.0)  p->move.airMoveSignLock = -1;
                else if (p->move.velocity.x > 0.0) p->move.airMoveSignLock = +1;
            }
            p->move.airInitialMoveSign = p->move.airMoveSignLock;
            p->move.airFlipUsed = false;
        }
        else
        {
            p->move.jumpCount++;
            p->move.airFlipUsed = true;
        }

        p->move.coyoteTimer = 0.0;
        p->move.isGrounded = false;
        p->move.airJumpStartX = (p->GetRect().Left() + p->GetRect().Right()) * 0.5;

        p->ChangeSprite("Assets/image/Player/PlayerJump.spt");
        if (auto spr = p->GetGOComponent<CS230::Sprite>())
        {
            spr->PlayAnimation(0);
        }
    }

    void PlayerState_Jumping::Update(CS230::GameObject* o, double dt)
    {
        auto* p = static_cast<Player*>(o);
        bool control = can_control(p);

        if (control && p->combat.health > 0 && p->GetWeapon())
        {
            p->GetWeapon()->TryStartAttack(*p);
        }

        if (p->input.jumpJustReleased && p->move.velocity.y > 0.0)
        {
            p->move.velocity.y *= 0.5;
        }

        int maxJumps = p->combat.tripleJump ? 3 : 2;

        if (control && p->move.jumpBufferTimer > 0.0 && p->move.jumpCount < maxJumps)
        {
            p->move.jumpBufferTimer = 0.0;
            double gval = 0.0;
            if (auto grav = Engine::GetGameStateManager().GetGSComponent<Gravity>())
            {
                gval = grav->GetValue();
            }

            const double desiredHeight = (Player::DefaultHeight * 2.0) * p->move.speedMultiplier * (1.0 + p->combat.jumpHeightBonus);
            double v0 = p->jumpVelocity;
            if (gval > 0.0)
            {
                v0 = std::sqrt(2.0 * (gval * 0.8) * desiredHeight);
            }

            p->move.velocity.y = v0;
            p->move.jumpCount++;
            p->move.airFlipUsed = true;

            if (auto spr = p->GetGOComponent<CS230::Sprite>())
            {
                spr->PlayAnimation(0);
            }
        }

        if (p->combat.slowMoTimer > 0.0 && !p->move.airDashUsed && p->input.rollJustPressed)
        {
            p->move.airDashUsed = true;
            p->change_state(&p->state_rolling);
            return;
        }

        p->update_horizontal(dt, true);
    }

    void PlayerState_Jumping::CheckExit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        if (p->move.isGrounded)
        {
            if (p->input.moveDir != 0)
            {
                p->change_state(&p->state_running);
            }
            else
            {
                p->change_state(&p->state_idle);
            }
            return;
        }

        if (p->move.velocity.y <= 0.0)
        {
            p->change_state(&p->state_falling);
            return;
        }
    }

    // FALLING STATE
    void PlayerState_Falling::Enter(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        p->move.isGrounded = false;
    }

    void PlayerState_Falling::Update(CS230::GameObject* o, double dt)
    {
        auto* p = static_cast<Player*>(o);
        bool control = can_control(p);

        if (control && p->combat.health > 0 && p->GetWeapon())
        {
            p->GetWeapon()->TryStartAttack(*p);
        }

        int maxJumps = p->combat.tripleJump ? 3 : 2;

        if (control && p->move.jumpBufferTimer > 0.0 && p->move.jumpCount < maxJumps)
        {
            p->move.jumpBufferTimer = 0.0;
            p->change_state(&p->state_jumping);
            return;
        }

        if (p->combat.slowMoTimer > 0.0 && !p->move.airDashUsed && p->input.rollJustPressed)
        {
            p->move.airDashUsed = true;
            p->change_state(&p->state_rolling);
            return;
        }
        p->update_horizontal(dt, true);
    }

    void PlayerState_Falling::CheckExit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);

        if (p->move.coyoteTimer > 0.0 && p->move.jumpBufferTimer > 0.0)
        {
            p->move.coyoteTimer = 0.0;
            p->move.jumpBufferTimer = 0.0;
            p->change_state(&p->state_jumping);
            return;
        }

        if (p->move.isGrounded)
        {
            if (p->input.moveDir != 0)
            {
                p->change_state(&p->state_running);
            }
            else
            {
                p->change_state(&p->state_idle);
            }
            return;
        }
    }

    // ROLLING STATE
    void PlayerState_Rolling::Enter(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        p->move.isRolling = true;
        p->move.dodgeRewardClaimed = false;
        p->combat.isParrying = false;
        p->combat.parryTimer = 0.0;

        p->rollDurationSec = 0.4;
        p->move.rollTimer = p->rollDurationSec;

        double baseRunSpeed = p->runSpeed * (1.0 + p->move.speedBonus);
        double bonusDistance = Player::DefaultWidth * 3.0;
        double bonusVelocity = bonusDistance / p->rollDurationSec;

        p->move.rollSpeed = (baseRunSpeed + bonusVelocity) * p->move.speedMultiplier;

        const double bottom = p->rect.Bottom();
        const double newH = Player::DefaultHeight * 0.5;
        p->rect.point_2.y = bottom + newH;

        p->move.preRollVelocityX = p->move.velocity.x;

        double dir = p->move.facingRight ? 1.0 : -1.0;
        p->move.velocity.x = p->move.rollSpeed * dir;

        p->ChangeSprite("Assets/image/Player/PlayerRoll.spt");
        if (auto spr = p->GetGOComponent<CS230::Sprite>())
        {
            spr->PlayAnimation(0);
        }
    }

    void PlayerState_Rolling::Update(CS230::GameObject* o, double dt)
    {
        auto* p = static_cast<Player*>(o);
        p->move.rollTimer -= dt;

        double dir = p->move.facingRight ? 1.0 : -1.0;
        p->move.velocity.x = p->move.rollSpeed * dir;
    }

    void PlayerState_Rolling::CheckExit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);

        bool hasCeiling = false;
        Math::rect fullRect = p->GetRect();
        fullRect.point_2.y = fullRect.point_1.y + Player::DefaultHeight;

        auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
        if (sm)
        {
            for (auto* obj : sm->staticObjects)
            {
                if (obj->Type() == GameObjectTypes::Platform)
                {
                    auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                    if (rc && Math::IntersectsRect(fullRect, rc->WorldBoundary()))
                    {
                        hasCeiling = true;
                        break;
                    }
                }
            }
        }

        if (p->move.jumpBufferTimer > 0.0)
        {
            if (!hasCeiling)
            {
                p->move.jumpBufferTimer = 0.0;
                p->change_state(&p->state_jumping);
                return;
            }
        }

        if (p->move.rollTimer <= 0.0)
        {
            if (hasCeiling)
            {
                p->move.rollTimer = 0.1;
            }
            else
            {
                if (p->input.moveDir != 0)
                {
                    p->change_state(&p->state_running);
                }
                else
                {
                    p->change_state(&p->state_idle);
                }
            }
        }
    }

    void PlayerState_Rolling::Exit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        p->move.isRolling = false;

        p->move.rollCooldown = 1.0;

        const double bottom = p->rect.Bottom();
        p->rect.point_2.y = bottom + Player::DefaultHeight;

        p->move.velocity.x = p->move.preRollVelocityX;
    }

    void PlayerState_Ziplining::Enter(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        p->move.isZiplining = true;
        p->move.isGrounded = false;

        p->move.velocity.x = 0.0;
        p->move.velocity.y = 900.0;

    }

    void PlayerState_Ziplining::Update(CS230::GameObject* o, double dt)
    {
        auto* p = static_cast<Player*>(o);
        (void)dt;

        p->move.velocity.x = 0.0;
        p->move.velocity.y = 900.0;

        if (p->input.jumpJustPressed)
        {
            p->move.jumpBufferTimer = 0.0;
            p->change_state(&p->state_jumping);
        }
    }

    void PlayerState_Ziplining::CheckExit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);

        Math::rect pRect = p->GetRect();
        Math::rect zRect = p->move.currentZiplineRect;

        if (pRect.Bottom() >= zRect.Top())
        {
            p->change_state(&p->state_falling);
        }
    }

    void PlayerState_Ziplining::Exit(CS230::GameObject* o)
    {
        auto* p = static_cast<Player*>(o);
        p->move.isZiplining = false;
    }
}