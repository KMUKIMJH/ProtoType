#include "Player.h"
#include "PlayerStates.h"
#include "Enemy.h"
#include "MeleeEnemy.h"
#include "../Game/Gravity.h"
#include "Weapon.h"
#include "../Engine/Engine.h"
#include "../Engine/Input.h"
#include "../Engine/Matrix.h"
#include "../Engine/Collision.h"
#include "../Engine/ShowCollision.h"
#include "../OpenGL/RGBA.h"
#include "../Engine/Camera.h"
#include "Particles.h"
#include "KeyBinding.h"
#include "../Stage/StageManager.h"
#include <string>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

namespace
{
    struct TempRectGO : CS230::GameObject
    {
        TempRectGO(const Math::rect& r) : CS230::GameObject({ r.Left(), r.Bottom() })
        {
            AddGOComponent(new CS230::RectCollision({ { 0, 0 }, { static_cast<int>(r.Right() - r.Left()), static_cast<int>(r.Top() - r.Bottom()) } }, this));
        }

        GameObjectTypes Type() override
        {
            return GameObjectTypes::Particle;
        }

        std::string TypeName() override
        {
            return "TempRectGO";
        }
    };

    static bool can_control(const CS230::Player* p)
    {
        return p->GetGroggyTimer() <= 0.0;
    }
}

namespace CS230
{
    void Player::StartZipline(double targetX, const Math::rect& zRect)
    {
        if (current_state != &state_ziplining)
        {
            move.ziplineTargetX = targetX;
            move.currentZiplineRect = zRect;
            change_state(&state_ziplining);
        }
    }

    void PlayerInput::Update()
    {
        moveDir = 0;
        if (KeyBinding::GetInstance().IsActionKeyDown(Action::MoveLeft))
        {
            moveDir -= 1;
        }
        if (KeyBinding::GetInstance().IsActionKeyDown(Action::MoveRight))
        {
            moveDir += 1;
        }

        bool sDown = KeyBinding::GetInstance().IsActionKeyDown(Action::DropThrough);
        bool spacePressed = KeyBinding::GetInstance().IsActionKeyJustPressed(Action::Jump);

        dropThrough = (sDown && spacePressed);
        jumpJustPressed = (!sDown && spacePressed);
        jumpJustReleased = KeyBinding::GetInstance().IsActionKeyJustReleased(Action::Jump);

        rollJustPressed = KeyBinding::GetInstance().IsActionKeyJustPressed(Action::Roll);
        parryJustPressed = KeyBinding::GetInstance().IsActionKeyJustPressed(Action::Parry);
        slowMoJustPressed = KeyBinding::GetInstance().IsActionKeyJustPressed(Action::SlowMo);
        debugInvinciblePressed = Engine::GetInput().KeyJustPressed(CS230::Input::Keys::I);
    }

    void Player::TakeHit()
    {
        if (combat.debugInvincible)
        {
            return;
        }

        if (combat.health > 0)
        {
            combat.health -= 1;
            combat.hitCooldown = groggyDurationSec;
            combat.groggyTimer = combat.hitCooldown;
            if (combat.invincibilityTimer < groggyInvincibleDurationSec)
            {
                combat.invincibilityTimer = groggyInvincibleDurationSec;
            }

            combat.isParrying = false;
            combat.parryTimer = 0.0;
            move.speedMultiplier = 1.0;
            combat.timeGauge = 0.0;

            combat.noHitKillCount = 0;
        }
    }

    void Player::Heal(int amount)
    {
        combat.health += amount;
        if (combat.health > combat.maxHealth)
        {
            combat.health = combat.maxHealth;
        }
    }

    void Player::AddTimeGauge(double amt)
    {
        combat.timeGauge += amt;
        if (combat.timeGauge > GetMaxGauge())
        {
            combat.timeGauge = GetMaxGauge();
        }
    }

    void Player::ChangeSprite(const std::string& path)
    {
        if (currentSpritePath == path)
        {
            return;
        }

        ClearGOComponents();
        AddGOComponent(new CS230::Sprite(path, this));
        currentSpritePath = path;

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            if (combat.attackActive && path == "Assets/image/Player/PlayerSword.spt")
            {
                int comboIndex = 0;
                if (combat.comboCount > 0)
                {
                    comboIndex = (combat.comboCount - 1) % 2;
                }
                spr->PlayAnimation(comboIndex);
            }
            else if (path == "Assets/image/Player/PlayerStay.spt")
            {
                spr->PlayAnimation(0);
            }
            else if (path == "Assets/image/Player/PlayerRun.spt")
            {
                spr->PlayAnimation(1);
            }
            else if (path == "Assets/image/Player/PlayerJump.spt")
            {
                spr->PlayAnimation(0);
            }
            else
            {
                spr->PlayAnimation(0);
            }
        }
    }

    void Player::Load(Player& player, Math::vec2 pos)
    {
        player.ClearGOComponents();
        Math::vec2 fixedSize{ Player::DefaultWidth, Player::DefaultHeight };

        player.rect = { pos, pos + fixedSize };

        PlayerCombat savedCombat = player.combat;
        PlayerMovement savedMove = player.move;

        player.move = PlayerMovement();
        player.combat = PlayerCombat();

        player.combat.health = savedCombat.health;
        player.combat.maxHealth = savedCombat.maxHealth;

        if (player.combat.health <= 0)
        {
            player.combat.health = player.combat.maxHealth;
        }

        player.combat.scrap = savedCombat.scrap;
        player.combat.savedAmmo = savedCombat.savedAmmo;
        player.combat.storyFragments = savedCombat.storyFragments;
        player.combat.showStoryUI = savedCombat.showStoryUI;
        player.combat.currentStoryId = savedCombat.currentStoryId;
        player.combat.viewedBossStories = savedCombat.viewedBossStories;
        player.combat.showMissingFragmentsUI = savedCombat.showMissingFragmentsUI;
        player.combat.storyUITimer = savedCombat.storyUITimer;

        player.combat.jumpHeightBonus = savedCombat.jumpHeightBonus;
        player.combat.tripleJump = savedCombat.tripleJump;
        player.combat.hammerDamageBonus = savedCombat.hammerDamageBonus;
        player.combat.hammerPullOnCharge = savedCombat.hammerPullOnCharge;
        player.combat.hammerRangeBonus = savedCombat.hammerRangeBonus;
        player.combat.arrowMaxAmmoBonus = savedCombat.arrowMaxAmmoBonus;
        player.combat.arrowMeleeDamageBonus = savedCombat.arrowMeleeDamageBonus;
        player.combat.arrowMeleeRangeBonus = savedCombat.arrowMeleeRangeBonus;
        player.combat.swordComboBonus = savedCombat.swordComboBonus;
        player.combat.swordDamageBonus = savedCombat.swordDamageBonus;
        player.combat.swordRangeBonus = savedCombat.swordRangeBonus;
        player.combat.slowMoDurationBonus = savedCombat.slowMoDurationBonus;
        player.combat.parryWindowBonus = savedCombat.parryWindowBonus;
        player.combat.slowMoGaugeReduction = savedCombat.slowMoGaugeReduction;

        player.combat.runTimer = savedCombat.runTimer;
        player.combat.noHitKillCount = savedCombat.noHitKillCount;
        player.combat.timeRewardClaimed = savedCombat.timeRewardClaimed;
        player.combat.noHitRewardClaimed = savedCombat.noHitRewardClaimed;

        player.combat.purchasedUpgrades = savedCombat.purchasedUpgrades;
        player.combat.timerPaused = savedCombat.timerPaused;

        player.move.speedBonus = savedMove.speedBonus;
        player.move.speedMultiplier = 1.0;

        player.DeactivateSlowMo();
        player.SetSlowMotionActive(false);

        const double gaugeSlot = player.GaugeSlotSize();
        player.gaugeGainRoll = gaugeSlot * 3.0;
        player.gaugeGainParry = gaugeSlot * 4.0;

        SetPosition({ player.rect.Left(), player.rect.Bottom() });

        player.currentSpritePath = "Assets/image/Player/PlayerStay.spt";
        AddGOComponent(new CS230::Sprite(player.currentSpritePath, this));

        SetScale({ 1.0, 1.0 });

        if (!player.GetWeapon() && !player.IsWeaponLocked())
        {
            static std::mt19937 gen(std::random_device{}());
            static std::vector<int> weaponBag;

            if (weaponBag.empty())
            {
                weaponBag = { 0, 1, 2 };
                std::shuffle(weaponBag.begin(), weaponBag.end(), gen);
            }

            int randWep = weaponBag.back();
            weaponBag.pop_back();

            CS230::WeaponType wType = CS230::WeaponType::Sword;
            if (randWep == 1)
            {
                wType = CS230::WeaponType::Hammer;
            }
            else if (randWep == 2)
            {
                wType = CS230::WeaponType::Arrow;
            }

            player.SetWeapon(CS230::MakeWeapon(wType));
            if (wType == CS230::WeaponType::Arrow)
            {
                static_cast<CS230::ArrowWeapon*>(player.GetWeapon())->SetAmmo(player.combat.savedAmmo);
            }
        }
        else if (player.GetWeapon())
        {
            if (player.GetWeapon()->GetType() == CS230::WeaponType::Arrow)
            {
                static_cast<CS230::ArrowWeapon*>(player.GetWeapon())->SetAmmo(player.combat.savedAmmo);
            }
            player.GetWeapon()->Reset();
        }

        current_state = &state_idle;
        current_state->Enter(this);
    }

    const Math::rect& Player::GetRect() const
    {
        if (auto rc = const_cast<Player*>(this)->GetGOComponent<CS230::RectCollision>())
        {
            collisionRectCache = rc->WorldBoundary();
            return collisionRectCache;
        }
        collisionRectCache = rect;
        if (!move.facingRight)
        {
            double w = collisionRectCache.Size().x;
            collisionRectCache.point_1.x -= w;
            collisionRectCache.point_2.x -= w;
        }
        return collisionRectCache;
    }

    void Player::ApplyKnockback(double velocityX, int dir, double velocityY)
    {
        move.velocity.x = velocityX * static_cast<double>(dir);
        if (move.velocity.y < 0.0)
        {
            move.velocity.y = 0.0;
        }
        move.velocity.y += velocityY;
        move.isGrounded = false;
    }

    void Player::ResolveCollision(CS230::GameObject* other_object)
    {
        if (!IsAlive() || other_object == nullptr)
        {
            return;
        }

        auto* otCol = other_object->GetGOComponent<CS230::RectCollision>();
        if (otCol == nullptr)
        {
            return;
        }

        const Math::rect myR = GetRect();
        const Math::rect otR = otCol->WorldBoundary();

        if (!(myR.Left() < otR.Right() && myR.Right() > otR.Left() && myR.Bottom() < otR.Top() && myR.Top() > otR.Bottom()))
        {
            return;
        }

        switch (other_object->Type())
        {
        case GameObjectTypes::Platform:
        {
            if (move.requestDropThrough && move.standingOn == other_object)
            {
                move.requestDropThrough = false;
                break;
            }

            ResolveCollision(GameObjectTypes::Platform, &otR, false, 0.0);

            if (move.isGrounded && move.velocity.y == 0.0 && move.hasStandingRect)
            {
                if (move.standingRect.Left() == otR.Left() && move.standingRect.Top() == otR.Top())
                {
                    move.standingOn = other_object;
                }
            }
            break;
        }
        case GameObjectTypes::TwoWayPlatform:
        {
            if (KeyBinding::GetInstance().IsActionKeyDown(Action::DropThrough))
            {
                break;
            }

            if (move.requestDropThrough && move.standingOn == other_object)
            {
                move.requestDropThrough = false;
                break;
            }

            ResolveCollision(GameObjectTypes::Platform, &otR, false, 0.0);

            if (move.isGrounded && move.velocity.y == 0.0 && move.hasStandingRect)
            {
                if (move.standingRect.Left() == otR.Left() && move.standingRect.Top() == otR.Top())
                {
                    move.standingOn = other_object;
                }
            }
            break;
        }
        case GameObjectTypes::MeleeEnemy:
        case GameObjectTypes::RangedEnemy:
        {
            auto* enemyGO = dynamic_cast<CS230::GameObject*>(other_object);
            auto* enemy = dynamic_cast<CS230::Enemy*>(other_object);
            if (enemy == nullptr)
            {
                break;
            }

            if (combat.isParrying && enemy->IsMeleeAttackActive() && combat.invincibilityTimer <= 0.0)
            {
                if (Math::IntersectsRect(myR, enemy->GetAttackBox()))
                {
                    enemy->ApplyParryDebuff(2.0);
                    HandleParry();
                    break;
                }
            }

            if (move.isRolling)
            {
                if (enemy->IsMeleeAttackActive() && Math::IntersectsRect(myR, enemy->GetAttackBox()))
                {
                    if (!move.dodgeRewardClaimed)
                    {
                        if (auto* me = dynamic_cast<CS230::MeleeEnemy*>(enemy))
                        {
                            me->NotifyAttackDodged();
                        }
                        AddTimeGauge(gaugeGainRoll);
                        move.dodgeRewardClaimed = true;
                    }
                }
                break;
            }

            ProcessEnemyCollision(enemy, enemyGO);
            break;
        }
        default:
        {
            break;
        }
        }

        SetPosition({ rect.Left(), rect.Bottom() });
    }

    void Player::ResolveCollision(GameObjectTypes type, const void* object, bool aux, double dt)
    {
        if (!IsAlive())
        {
            return;
        }
        (void)dt;

        switch (type)
        {
        case GameObjectTypes::Platform:
        {
            const Math::rect* platform = static_cast<const Math::rect*>(object);
            if (platform == nullptr || aux)
            {
                break;
            }

            const Math::rect playerRect = GetRect();

            if (Math::IntersectsRect(playerRect, *platform))
            {
                double realDt = move.lastDt;
                if (realDt <= 0.0)
                {
                    realDt = 0.016;
                }

                double prevBottom = playerRect.Bottom() - move.velocity.y * realDt;
                double prevTop = playerRect.Top() - move.velocity.y * realDt;
                double prevRight = playerRect.Right() - move.velocity.x * realDt;
                double prevLeft = playerRect.Left() - move.velocity.x * realDt;

                bool wasAbove = prevBottom >= platform->Top() - 0.5;
                bool wasBelow = prevTop <= platform->Bottom() + 0.5;
                bool wasLeft = prevRight <= platform->Left() + 0.5;
                bool wasRight = prevLeft >= platform->Right() - 0.5;

                double overlapXLeft = playerRect.Right() - platform->Left();
                double overlapXRight = platform->Right() - playerRect.Left();
                double overlapYBottom = playerRect.Top() - platform->Bottom();
                double overlapYTop = platform->Top() - playerRect.Bottom();

                double dx = (overlapXLeft < overlapXRight) ? -overlapXLeft : overlapXRight;
                double dy = (overlapYBottom < overlapYTop) ? -overlapYBottom : overlapYTop;

                bool resolveX = false;
                bool resolveY = false;

                if (wasAbove && move.velocity.y <= 0.0)
                {
                    dy = overlapYTop;
                    resolveY = true;
                }
                else if (wasBelow && move.velocity.y >= 0.0)
                {
                    dy = -overlapYBottom;
                    resolveY = true;
                }
                else if (wasLeft && move.velocity.x >= 0.0)
                {
                    dx = -overlapXLeft;
                    resolveX = true;
                }
                else if (wasRight && move.velocity.x <= 0.0)
                {
                    dx = overlapXRight;
                    resolveX = true;
                }

                if (!resolveX && !resolveY)
                {
                    if (std::abs(dx) < std::abs(dy))
                    {
                        resolveX = true;
                    }
                    else
                    {
                        resolveY = true;
                    }
                }

                if (resolveY && !resolveX)
                {
                    rect.point_1.y += dy;
                    rect.point_2.y += dy;
                    move.velocity.y = 0.0;

                    if (dy >= 0.0)
                    {
                        move.isGrounded = true;
                        move.jumpCount = 0;
                        move.standingRect = *platform;
                        move.hasStandingRect = true;
                        move.standingOn = nullptr;
                        move.airDashUsed = false;
                    }
                }
                else if (resolveX && !resolveY)
                {
                    rect.point_1.x += dx;
                    rect.point_2.x += dx;
                    move.velocity.x = 0.0;
                }
            }
            break;
        }
        case GameObjectTypes::MeleeEnemy:
        case GameObjectTypes::RangedEnemy:
        {
            const auto* enemy = static_cast<const CS230::Enemy*>(object);
            if (enemy == nullptr || !enemy->IsActive() || enemy->GetHealth() <= 0)
            {
                break;
            }

            const Math::rect playerRect = GetRect();

            bool meleeHit = false;
            if (enemy->IsMeleeAttackActive())
            {
                meleeHit = Math::IntersectsRect(playerRect, enemy->GetAttackBox());
                if (combat.isParrying && meleeHit && combat.invincibilityTimer <= 0.0)
                {
                    CS230::Enemy* em = const_cast<CS230::Enemy*>(enemy);
                    em->ApplyParryDebuff(2.0);
                    HandleParry();
                    break;
                }
            }

            if (!meleeHit)
            {
                break;
            }

            if (move.isRolling)
            {
                if (meleeHit && !move.dodgeRewardClaimed)
                {
                    AddTimeGauge(gaugeGainRoll);
                    move.dodgeRewardClaimed = true;
                }
                break;
            }

            if (meleeHit && combat.hitCooldown <= 0.0 && combat.health > 0)
            {
                TakeHit();
                CS230::Enemy* em = const_cast<CS230::Enemy*>(enemy);
                em->OnSuccessfulHitPlayer();

                const Math::vec2 eC = enemy->Center();
                const Math::vec2 pC = Center();
                ApplyKnockback(30.0, (pC.x >= eC.x) ? 1 : -1);
            }
            break;
        }
        default:
        {
            break;
        }
        }
    }

    bool Player::IsSupportedOnGround() const
    {
        const Math::rect myRect = GetRect();
        auto checkOnTop = [&](const Math::rect& r)
            {
                if (myRect.Right() <= r.Left() + 1.0 || myRect.Left() >= r.Right() - 1.0)
                {
                    return false;
                }
                const double topBand = 5.0;
                if (myRect.Bottom() <= r.Top() + topBand && myRect.Bottom() >= r.Top() - topBand)
                {
                    return true;
                }
                return false;
            };

        if (move.standingOn != nullptr)
        {
            auto* platformCol = move.standingOn->GetGOComponent<CS230::RectCollision>();
            if (platformCol != nullptr)
            {
                if (move.standingOn->Type() == GameObjectTypes::TwoWayPlatform && KeyBinding::GetInstance().IsActionKeyDown(Action::DropThrough))
                {
                    return false;
                }
                if (checkOnTop(platformCol->WorldBoundary())) return true;
            }
        }
        else if (move.hasStandingRect)
        {
            if (checkOnTop(move.standingRect)) return true;
        }

        if (checkOnTop(groundRectCache)) return true;

        if (auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
        {
            for (auto* obj : sm->staticObjects)
            {
                if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
                {
                    if (obj->Type() == GameObjectTypes::TwoWayPlatform && KeyBinding::GetInstance().IsActionKeyDown(Action::DropThrough))
                    {
                        continue;
                    }

                    auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                    if (rc && checkOnTop(rc->WorldBoundary()))
                    {
                        return true; 
                    }
                }
            }
        }

        return false;
    }

    void Player::UpdateTimers(double dt)
    {
        if (combat.isParrying)
        {
            input.moveDir = 0;
        }

        if (IsSlowMoActive())
        {
            move.speedMultiplier = 1.0;
        }

        if (move.isGrounded)
        {
            move.coyoteTimer = CoyoteTimeSec;
        }
        else if (move.coyoteTimer > 0.0)
        {
            move.coyoteTimer -= dt;
        }

        if (move.jumpBufferTimer > 0.0)
        {
            move.jumpBufferTimer -= dt;
        }
        if (input.jumpJustPressed)
        {
            move.jumpBufferTimer = JumpBufferSec;
        }

        if (input.moveDir == 0)
        {
            move.idleTimer += dt;
            if (move.idleTimer >= 2.0)
            {
                move.speedMultiplier = 1.0;
            }
        }
        else
        {
            move.idleTimer = 0.0;
            move.speedMultiplier += 0.03 * dt;
            if (move.speedMultiplier > 2.0)
            {
                move.speedMultiplier = 2.0;
            }
        }

#ifdef _DEBUG
        if (input.debugInvinciblePressed)
        {
            combat.debugInvincible = !combat.debugInvincible;
        }
#endif

        if (combat.hitCooldown > 0.0)
        {
            combat.hitCooldown -= dt;
            if (combat.hitCooldown < 0.0) combat.hitCooldown = 0.0;
        }

        if (!combat.attackActive && combat.comboTimer > 0.0)
        {
            combat.comboTimer -= dt;
            if (combat.comboTimer <= 0.0) combat.comboCount = 0;
        }

        if (combat.invincibilityTimer > 0.0)
        {
            combat.invincibilityTimer -= dt;
            if (combat.invincibilityTimer < 0.0) combat.invincibilityTimer = 0.0;
        }

        if (move.rollCooldown > 0.0)
        {
            move.rollCooldown -= dt;
            if (move.rollCooldown < 0.0) move.rollCooldown = 0.0;
        }

        move.dashCooldown = 0.0;
        move.dashTimer = 0.0;

        if (combat.parryCooldown > 0.0)
        {
            combat.parryCooldown -= dt;
            if (combat.parryCooldown < 0.0) combat.parryCooldown = 0.0;
        }

        if (combat.isParrying)
        {
            combat.parryTimer -= dt;
            if (combat.parryTimer <= 0.0)
            {
                combat.parryTimer = 0.0;
                combat.isParrying = false;
            }
        }

        if (combat.parrySpeedBuffTimer > 0.0)
        {
            combat.parrySpeedBuffTimer -= dt;
            if (combat.parrySpeedBuffTimer < 0.0) combat.parrySpeedBuffTimer = 0.0;
        }

        if (combat.groggyTimer > 0.0)
        {
            combat.groggyTimer -= dt;
            if (combat.groggyTimer < 0.0) combat.groggyTimer = 0.0;
        }

        if (!combat.isParrying && combat.parryCooldown <= 0.0 && combat.groggyTimer <= 0.0 && combat.health > 0)
        {
            if (input.parryJustPressed)
            {
                combat.isParrying = true;
                double window = parryWindowSec + combat.parryWindowBonus;
                if (IsSlowMoActive()) window *= 1.5;
                combat.parryTimer = window;
                combat.parryCooldown = parryCooldownSec;
            }
        }

        if (combat.slowMoTimer <= 0.0 && IsTimeSlowReady() && input.slowMoJustPressed)
        {
            ActivateSlowMo();
            move.airDashUsed = false;
        }

        if (combat.slowMoTimer > 0.0)
        {
            combat.slowMoTimer -= dt;
            if (slowMoDurationSec > 0.0)
            {
                combat.timeGauge = (combat.slowMoTimer / (slowMoDurationSec + combat.slowMoDurationBonus)) * GetMaxGauge();
            }
            if (combat.slowMoTimer <= 0.0)
            {
                DeactivateSlowMo();
                combat.timeGauge = 0.0;
                move.airDashUsed = false;
            }
        }
        if (!combat.timerPaused)
        {
            combat.runTimer += dt;
        }
    }

    void Player::HandleDeathState(int worldWidth)
    {
        combat.attackActive = false;
        move.isRolling = false;
        move.isDashing = false;
        move.isZiplining = false;
        combat.isParrying = false;

        if (!collisionRemoved)
        {
            ClearGOComponents();
            collisionRemoved = true;
        }

        combat.timeGauge = 0.0;
        DeactivateSlowMo();

        if (rect.Left() < 0)
        {
            const double w = rect.Right() - rect.Left();
            rect.point_1.x = 0;
            rect.point_2.x = w;
        }
        if (rect.Right() > worldWidth)
        {
            const double w = rect.Right() - rect.Left();
            rect.point_2.x = static_cast<double>(worldWidth);
            rect.point_1.x = rect.point_2.x - w;
        }
        SetPosition({ rect.Left(), rect.Bottom() });

        const Math::vec2 curSize = rect.Size();
        Math::vec2 newScale{ 1.0, 1.0 };
        newScale.x = curSize.x / Player::DefaultWidth;
        newScale.y = curSize.y / Player::DefaultHeight;
        SetScale(newScale);
    }

    void Player::CheckGroundSupport()
    {
        bool supported = IsSupportedOnGround();

        if (!supported && move.isGrounded)
        {
            move.isGrounded = false;
            move.standingOn = nullptr;
            move.hasStandingRect = false;

            if (!move.isRolling && !move.isZiplining)
            {
                change_state(&state_falling);
            }
        }
    }

    void Player::UpdatePhysics(int worldWidth, int worldHeight, double dt)
    {
        if (rect.Left() < 0)
        {
            const double w = rect.Right() - rect.Left();
            rect.point_1.x = 0;
            rect.point_2.x = w;
        }
        if (rect.Right() > worldWidth)
        {
            const double w = rect.Right() - rect.Left();
            rect.point_2.x = static_cast<double>(worldWidth);
            rect.point_1.x = rect.point_2.x - w;
        }
        if (rect.Bottom() < 0.0)
        {
            const double h = rect.Top() - rect.Bottom();
            rect.point_1.y = 0.0;
            rect.point_2.y = h;
            move.velocity.y = 0.0;
        }
        if (rect.Top() > static_cast<double>(worldHeight))
        {
            const double h = rect.Top() - rect.Bottom();
            rect.point_2.y = static_cast<double>(worldHeight);
            rect.point_1.y = rect.point_2.y - h;
            move.velocity.y = 0.0;
        }

        constexpr double MAX_VELOCITY_X = 3000.0;
        constexpr double MAX_VELOCITY_Y = 1400.0;

        if (move.velocity.x > MAX_VELOCITY_X) move.velocity.x = MAX_VELOCITY_X;
        if (move.velocity.x < -MAX_VELOCITY_X) move.velocity.x = -MAX_VELOCITY_X;
        if (move.velocity.y > MAX_VELOCITY_Y) move.velocity.y = MAX_VELOCITY_Y;

        SetPosition({ rect.Left(), rect.Bottom() });

        if (std::abs(move.velocity.x) > 0.1)
        {
            rect.point_1.x += move.velocity.x * dt;
            rect.point_2.x += move.velocity.x * dt;

            if (!move.isRolling && !move.isZiplining)
            {
                const double damping = 0.95;
                const double frameFactor = std::pow(damping, dt * 60.0);
                move.velocity.x *= frameFactor;
                if (std::abs(move.velocity.x) < 1.0)
                {
                    move.velocity.x = 0.0;
                }
            }
        }

        if (!move.isGrounded)
        {
            if (!move.isZiplining)
            {
                double gval = 0.0;
                if (auto grav = Engine::GetGameStateManager().GetGSComponent<Gravity>())
                {
                    gval = grav->GetValue();
                }

                if (std::abs(move.velocity.y) < 150.0) gval *= 0.5;
                else if (move.velocity.y > 0.0) gval *= 0.8;
                else gval *= 1.5;

                move.velocity.y -= gval * dt;
                if (move.velocity.y < MaxFallVelocity) move.velocity.y = MaxFallVelocity;
            }

            rect.point_1.y += move.velocity.y * dt;
            rect.point_2.y += move.velocity.y * dt;
        }

        SetPosition({ rect.Left(), rect.Bottom() });
    }

    void Player::UpdateCombat(double dt)
    {
        bool wasAttacking = combat.attackActive;

        if (weapon)
        {
            weapon->Update(*this, dt);

            if (weapon->IsAttackActive() || weapon->GetType() == WeaponType::Arrow)
            {
                if (weapon->IsAttackActive())
                {
                    combat.attackActive = true;
                    combat.attackBox = weapon->GetAttackBox(*this);
                }
                else
                {
                    combat.attackActive = false;
                    combat.attackBox = { { 0, 0 }, { 0, 0 } };
                }

                auto hits = weapon->ResolveHits(*this);
                for (auto& h : hits)
                {
                    if (!h.enemy) continue;

                    bool isBlocked = false;
                    if (h.isProjectile) isBlocked = (h.damageMult <= 0.0);
                    else isBlocked = h.enemy->CanIgnorePlayerAttack(*this);

                    if (isBlocked)
                    {
                        if (h.enemy->playerHitCooldownSec <= 0.0 || h.isProjectile)
                        {
                            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/block.wav");
                            if (!h.isProjectile) h.enemy->playerHitCooldownSec = 0.5;
                        }
                        continue;
                    }

                    if (h.enemy->playerHitCooldownSec <= 0.0 || h.isProjectile)
                    {
                        if (!h.isProjectile) h.enemy->playerHitCooldownSec = 0.5;
                        h.enemy->hitCooldownSec = 15.0 / 60.0;
                        h.enemy->NotifyHit(this);

                        int dmg = h.strong ? 2 : 1;
                        if (weapon->GetType() == WeaponType::Sword) dmg += combat.swordDamageBonus;
                        else if (weapon->GetType() == WeaponType::Hammer) dmg += combat.hammerDamageBonus;
                        else if (weapon->GetType() == WeaponType::Arrow && !h.isProjectile) dmg += combat.arrowMeleeDamageBonus;

                        if (IsSlowMoActive())
                        {
                            if (h.enemy->IsBoss())
                            {
                                dmg += 2;
                            }
                            else
                            {
                                dmg = 9999;
                            }
                        }

                        int hp = h.enemy->GetHealth();
                        hp -= dmg;
                        h.enemy->SetHealth(hp);

                        if (hp <= 0)
                        {
                            combat.noHitKillCount++;

                            if (auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState()))
                            {
                                sm->NotifyEnemyKilled();
                            }
                        }

                        bool isHammerPull = (weapon->GetType() == WeaponType::Hammer && h.strong);
                        double knockbackNudge = isHammerPull ? 0.0 : (h.strong ? 45.0 : 30.0);

                        h.enemy->ApplyStunAndKnockbackFromPlayer(*this, h.strong ? 0.35 : 0.2, knockbackNudge, combat.hammerPullOnCharge);
                        NotifyEnemyHit();
                    }
                }
            }
            else
            {
                combat.attackActive = false;
                combat.attackBox = { { 0, 0 }, { 0, 0 } };
            }
        }

        bool attackJustStarted = (!wasAttacking && combat.attackActive);
        bool attackJustEnded = (wasAttacking && !combat.attackActive);

        if (attackJustStarted)
        {
            if (weapon && weapon->GetType() == CS230::WeaponType::Sword)
            {

                combat.comboCount++;
                combat.comboTimer = 0.5;
            }
        }
        else if (attackJustEnded)
        {
            if (auto spr = GetGOComponent<CS230::Sprite>())
            {
                if (current_state == &state_running) spr->PlayAnimation(1);
                else spr->PlayAnimation(0);
            }
        }
    }

    void Player::UpdateAnimations()
    {
        std::string targetSprite = "Assets/image/Player/PlayerStay.spt";

        if (combat.isParrying)
        {
            if (weapon)
            {
                switch (weapon->GetType())
                {
                case CS230::WeaponType::Sword: targetSprite = "Assets/image/Player/PlayerParryS.spt"; break;
                case CS230::WeaponType::Hammer: targetSprite = "Assets/image/Player/PlayerParryH.spt"; break;
                case CS230::WeaponType::Arrow: targetSprite = "Assets/image/Player/PlayerParryA.spt"; break;
                }
            }
            else
            {
                targetSprite = "Assets/image/Player/PlayerParryS.spt";
            }
        }
        else if (current_state == &state_rolling)
        {
            targetSprite = "Assets/image/Player/PlayerRoll.spt";
        }
        else if (current_state == &state_running)
        {
            targetSprite = "Assets/image/Player/PlayerRun.spt";
        }
        else if (current_state == &state_jumping || current_state == &state_falling)
        {
            targetSprite = "Assets/image/Player/PlayerJump.spt";
        }
        else if (current_state == &state_ziplining)
        {
            targetSprite = "Assets/image/Player/PlayerZipLine.spt";
        }

        if (!combat.isParrying && current_state != &state_rolling)
        {
            if (weapon && weapon->GetType() == CS230::WeaponType::Sword && combat.attackActive)
            {
                targetSprite = "Assets/image/Player/PlayerSword.spt";
            }
            else if (weapon && weapon->GetType() == CS230::WeaponType::Arrow)
            {
                auto* arrowWeapon = static_cast<CS230::ArrowWeapon*>(weapon.get());
                if (arrowWeapon->IsShooting() || arrowWeapon->IsAttackActive())
                {
                    targetSprite = "Assets/image/Player/PlayerArrow.spt";
                }
            }
            else if (weapon && weapon->GetType() == CS230::WeaponType::Hammer)
            {
                auto* hammerWeapon = static_cast<CS230::HammerWeapon*>(weapon.get());
                if (hammerWeapon->IsAttackActive() || hammerWeapon->IsCharging())
                {
                    targetSprite = "Assets/image/Player/PlayerHammer.spt";
                }
            }
        }

        ChangeSprite(targetSprite);

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            if (targetSprite == "Assets/image/Player/PlayerHammer.spt")
            {
                auto* hammerWeapon = static_cast<CS230::HammerWeapon*>(weapon.get());
                if (hammerWeapon->IsCharging())
                {
                    if (spr->CurrentAnimation() != 1) spr->PlayAnimation(1);
                }
                else if (hammerWeapon->IsAttackActive())
                {
                    if (spr->CurrentAnimation() != 0) spr->PlayAnimation(0);
                }
            }
            else if (targetSprite == "Assets/image/Player/PlayerArrow.spt")
            {
                auto* arrowWeapon = static_cast<CS230::ArrowWeapon*>(weapon.get());
                if (arrowWeapon->IsShooting())
                {
                    if (spr->CurrentAnimation() != 0) spr->PlayAnimation(0);
                }
                else if (arrowWeapon->IsAttackActive())
                {
                    if (spr->CurrentAnimation() != 1) spr->PlayAnimation(1);
                }
            }

            const Math::vec2 curSize = rect.Size();
            Math::vec2 newScale{ 1.0, 1.0 };
            Math::ivec2 fs = spr->GetFrameSize();
            if (fs.x > 0) newScale.x = curSize.x / fs.x;
            if (fs.y > 0) newScale.y = curSize.y / fs.y;

            if (!move.facingRight) newScale.x = -std::abs(newScale.x);
            else newScale.x = std::abs(newScale.x);

            SetScale(newScale);
        }
    }

    void Player::UpdateAfterimages(double dt)
    {
        if (IsSlowMoActive())
        {
            afterimageSpawnTimer -= dt;
            if (afterimageSpawnTimer <= 0.0)
            {
                if (auto spr = GetGOComponent<CS230::Sprite>())
                {
                    Afterimage img;
                    img.matrix = GetMatrix();
                    img.maxTime = 0.5;
                    img.timer = img.maxTime;
                    img.framePos = spr->GetFrameTexel(spr->GetCurrentFrame());
                    img.frameSize = spr->GetFrameSize();
                    img.frameTex = spr->GetTexture();
                    img.hotSpot = { static_cast<double>(spr->GetHotSpot(0).x), static_cast<double>(spr->GetHotSpot(0).y) };
                    img.flip = !move.facingRight;
                    unsigned int tints[] = { GAME200::RED, GAME200::GREEN, GAME200::SKYBLUE };
                    img.tint = tints[rand() % 3];
                    afterimages.push_back(img);
                }
                afterimageSpawnTimer = 0.015;
            }
        }

        for (auto it = afterimages.begin(); it != afterimages.end();)
        {
            it->timer -= dt;
            if (it->timer <= 0.0) it = afterimages.erase(it);
            else ++it;
        }
    }

    void Player::Update(Math::rect ground, int worldWidth, int worldHeight, double dt)
    {
        move.lastDt = dt;
        groundRectCache = ground;

        input.Update();
        if (combat.showStoryUI)
        {
            return;
        }

        UpdateTimers(dt);

        if (combat.health <= 0)
        {
            HandleDeathState(worldWidth);
            return;
        }

        if (input.rollJustPressed && move.isGrounded && move.rollCooldown <= 0.0 && combat.groggyTimer <= 0.0 && !move.isRolling)
        {
            change_state(&state_rolling);
        }

        if (current_state == nullptr)
        {
            current_state = &state_idle;
            current_state->Enter(this);
        }
        current_state->Update(this, dt);

        CheckGroundSupport();

        current_state->CheckExit(this);

        UpdatePhysics(worldWidth, worldHeight, dt);
        UpdateCombat(dt);
        UpdateAnimations();
        UpdateGOComponents(dt);
        UpdateAfterimages(dt);
    }

    void Player::Draw(Math::TransformationMatrix camM)
    {
        for (const auto& img : afterimages)
        {
            if (!img.frameTex) continue;

            double aRatio = img.timer / img.maxTime;
            unsigned int alpha = static_cast<unsigned int>(aRatio * 180.0);
            unsigned int finalTint = (img.tint & 0xFFFFFF00) | (alpha & 0xFF);

            Math::TransformationMatrix m = camM * img.matrix;
            m *= Math::TranslationMatrix(-img.hotSpot);
            img.frameTex->Draw(m, img.framePos, img.frameSize, finalTint);
        }

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            Math::TransformationMatrix objM = GetMatrix();
            spr->Draw(camM * objM);
        }

        if (GetWeapon())
        {
            GetWeapon()->Draw(camM);
        }

        GAME200::RGBA color = GAME200::GREEN;
        const double blinkFast = 0.0833;
        const double blinkMed = 0.2;

        auto blink = [&](double t, double period)
            {
                double fm = std::fmod(t, period);
                return (fm < period * 0.5);
            };

        if (combat.health <= 0) color = GAME200::GRAY;
        else if (move.isRolling)
        {
            if (blink(move.rollTimer, blinkFast)) color = GAME200::SKYBLUE;
            else color = GAME200::DKBLUE;
        }
        else if (combat.isParrying) color = GAME200::MAROON;
        else if (combat.groggyTimer > 0.0)
        {
            if (blink(combat.groggyTimer, blinkMed)) color = GAME200::RED;
            else color = GAME200::CLEAR;
        }
        else if (combat.invincibilityTimer > 0.0 || combat.debugInvincible)
        {
            double add = combat.debugInvincible ? 0.1 : 0.0;
            if (blink(combat.invincibilityTimer + add, blinkMed)) color = GAME200::GOLD;
            else color = GAME200::YELLOW;
        }
        else if (combat.hitCooldown > 0.0)
        {
            if (blink(combat.hitCooldown, blinkMed)) color = GAME200::RED;
            else color = GAME200::CLEAR;
        }

#ifdef _DEBUG

        if (auto showCollision = Engine::GetGameStateManager().GetGSComponent<CS230::ShowCollision>())
        {
            if (showCollision->Enabled() && combat.attackActive)
            {
                const Math::vec2 atkSize{ combat.attackBox.Size() };
                const Math::vec2 atkCenter{ combat.attackBox.Left() + atkSize.x * 0.5, combat.attackBox.Bottom() + atkSize.y * 0.5 };
                Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(atkCenter) * Math::ScaleMatrix(atkSize), GAME200::RED, GAME200::RED, 0.0);
            }
        }
#endif

        if (combat.health > 0 && !collisionRemoved)
        {
            if (combat.debugInvincible)
            {
                static std::shared_ptr<CS230::Texture> invinTex = nullptr;
                if (!invinTex)
                {
                    invinTex = Engine::GetFont(0).PrintToTextureScaled("Invincible", GAME200::GOLD, 0.5);
                }

                if (invinTex)
                {
                    Math::ivec2 texSize = invinTex->GetSize();
                    const Math::rect r = GetRect();
                    const double centerX = (r.Left() + r.Right()) * 0.5;
                    double textX = centerX - texSize.x * 0.5;
                    double textY = r.Top() + 30.0;
                    invinTex->Draw(camM * Math::TranslationMatrix(Math::vec2{ textX, textY }));
                }
            }
        }
#ifdef _DEBUG
        if (auto showCollision = Engine::GetGameStateManager().GetGSComponent<CS230::ShowCollision>())
        {
            if (showCollision->Enabled())
            {
                if (auto col = GetGOComponent<CS230::RectCollision>())
                {
                    col->Draw(camM);
                }
                else
                {
                    const Math::rect r = GetRect();
                    Math::vec2 size = { r.Right() - r.Left(), r.Top() - r.Bottom() };
                    Math::vec2 center = { r.Left() + size.x * 0.5, r.Bottom() + size.y * 0.5 };
                    Engine::GetRenderer2D().DrawRectangle(
                        camM * Math::TranslationMatrix(center) * Math::ScaleMatrix(size),
                        GAME200::CLEAR, GAME200::WHITE, 1.0
                    );
                }
            }
        }
#endif
    }

    void Player::update_horizontal(double dt, bool allowInput)
    {
        if (!allowInput || !can_control(this))
        {
            return;
        }

        double localMultiplier = move.speedMultiplier;
        if (combat.parrySpeedBuffTimer > 0.0)
        {
            localMultiplier *= parrySpeedMultiplier;
        }

        double finalRunSpeed = runSpeed * (1.0 + move.speedBonus);

        if (input.moveDir < 0)
        {
            const double dx = finalRunSpeed * localMultiplier * dt;
            rect.point_1.x -= dx;
            rect.point_2.x -= dx;
            move.facingRight = false;
        }
        else if (input.moveDir > 0)
        {
            const double dx = finalRunSpeed * localMultiplier * dt;
            rect.point_1.x += dx;
            rect.point_2.x += dx;
            move.facingRight = true;
        }
    }


    bool Player::HandleParry()
    {
        if (combat.isParrying && combat.health > 0)
        {
            NotifyInvincibleHit();
            AddTimeGauge(gaugeGainParry);

            combat.invincibilityTimer = 0.3;
            EndParry();
            return true;
        }
        return false;
    }
    void Player::ProcessEnemyCollision(CS230::Enemy* enemy, CS230::GameObject* enemyGO)
    {
        if (enemy == nullptr)
        {
            return;
        }

        (void)enemyGO;
        const Math::rect playerRect = GetRect();
        bool meleeHit = false;

        if (enemy->IsMeleeAttackActive() && Math::IntersectsRect(playerRect, enemy->GetAttackBox()))
        {
            meleeHit = true;
        }

        if (!meleeHit)
        {
            return;
        }

        if (meleeHit && combat.hitCooldown <= 0.0 && combat.health > 0)
        {
            TakeHit();
            enemy->OnSuccessfulHitPlayer();

            const Math::vec2 eC = enemy->Center();
            const Math::vec2 pC = Center();
            ApplyKnockback(30.0, (pC.x >= eC.x) ? 1 : -1);
        }
    }

    void Player::NotifyInvincibleHit()
    {
        combat.invincibleHitTriggered = true;
        if (auto pm = Engine::GetGameStateManager().GetGSComponent<CS230::ParticleManager<Particles::Effect1>>())
        {
            Math::vec2 effPos = GetPosition();
            auto rectComp = GetGOComponent<CS230::RectCollision>();
            if (rectComp)
            {
                effPos.y += rectComp->WorldBoundary().Size().y / 2.0;
                effPos.x += rectComp->WorldBoundary().Size().x / 2.0;
            }
            pm->Emit(15, effPos, { 0.0, 0.0 }, { 0.0, 400.0 }, 3.14159265358979323846 * 2.0);
        }
    }

    void Player::AddStoryItem(bool isBossDrop)
    {
        if (isBossDrop)
        {
            combat.currentStoryId = combat.viewedBossStories % 3;
            combat.viewedBossStories++;
            combat.showStoryUI = true;
        }
        else
        {
            combat.storyFragments++;
        }
    }
}