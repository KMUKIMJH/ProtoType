#pragma once
#include <algorithm>
#include <memory>
#include <deque>
#include <list>
#include "../Engine/GameObject.h"
#include "../Engine/Rect.h"
#include "../Engine/Vec2.h"
#include "../Game/GameObjectTypes.h"
#include "Weapon.h"
#include "PlayerStates.h"

namespace CS230
{
    class Enemy;

    struct PlayerInput
    {
        int moveDir{ 0 };
        bool jumpJustPressed{ false };
        bool jumpJustReleased{ false };
        bool rollJustPressed{ false };
        bool parryJustPressed{ false };
        bool dropThrough{ false };
        bool slowMoJustPressed{ false };
        bool debugInvinciblePressed{ false };

        void Update();
    };

    struct PlayerMovement
    {
        Math::vec2 velocity{ 0.0, 0.0 };
        bool isGrounded{ false };
        int jumpCount{ 0 };
        bool facingRight{ true };

        bool isRolling{ false };
        double rollTimer{ 0.0 };
        double rollCooldown{ 0.0 };
        double rollSpeed{ 0.0 };

        double preRollVelocityX{ 0.0 };

        bool dodgeRewardClaimed{ false };

        bool isDashing{ false };
        double dashTimer{ 0.0 };
        double dashCooldown{ 0.0 };
        bool airDashUsed{ false };

        bool isZiplining{ false };
        double ziplineTargetX{ 0.0 };
        Math::rect currentZiplineRect{ { 0, 0 }, { 0, 0 } };

        double speedMultiplier{ 1.0 };
        double idleTimer{ 0.0 };
        double coyoteTimer{ 0.0 };
        double jumpBufferTimer{ 0.0 };

        CS230::GameObject* standingOn{ nullptr };
        Math::rect standingRect{ { 0, 0 }, { 0, 0 } };
        bool hasStandingRect{ false };
        bool requestDropThrough{ false };

        int airMoveSignLock{ 0 };
        bool airFlipUsed{ false };
        int airInitialMoveSign{ 0 };
        double airJumpStartX{ 0.0 };

        double lastDt{ 0.016 };
        double speedBonus{ 0.0 };
    };

    struct PlayerCombat
    {
        int health{ 5 };
        int maxHealth{ 5 };
        int scrap{ 0 };
        int savedAmmo{ 5 };
        int comboCount{ 0 };
        double comboTimer{ 0.0 };

        double hitCooldown{ 0.0 };
        double invincibilityTimer{ 0.0 };
        double invincibilityCooldown{ 0.0 };
        double groggyTimer{ 0.0 };
        bool invincibleHitTriggered{ false };
        int enemyHitEvents{ 0 };

        bool isParrying{ false };
        double parryTimer{ 0.0 };
        double parryCooldown{ 0.0 };
        double parrySpeedBuffTimer{ 0.0 };

        double timeGauge{ 0.0 };
        double slowMoTimer{ 0.0 };
        bool externalSlowMo{ false };

        bool attackActive{ false };
        Math::rect attackBox{ { 0, 0 }, { 0, 0 } };
        int attackDirection{ 0 };

        bool weaponLocked{ false };
        bool debugInvincible{ false };

        double jumpHeightBonus{ 0.0 };
        bool tripleJump{ false };

        int hammerDamageBonus{ 0 };
        bool hammerPullOnCharge{ false };
        double hammerRangeBonus{ 0.0 };
        double hammerChargeReduction{ 0.0 };

        int arrowMaxAmmoBonus{ 0 };
        int arrowMeleeDamageBonus{ 0 };
        double arrowMeleeRangeBonus{ 0.0 };

        int swordComboBonus{ 0 };
        int swordDamageBonus{ 0 };
        double swordRangeBonus{ 0.0 };

        double damageBonus{ 0.0 };

        double slowMoDurationBonus{ 0.0 };
        double parryWindowBonus{ 0.0 };
        int slowMoGaugeReduction{ 0 };

        double runTimer{ 0.0 };
        int noHitKillCount{ 0 };
        bool timeRewardClaimed{ false };
        bool noHitRewardClaimed{ false };
        bool timerPaused{ false };

        std::vector<int> purchasedUpgrades;

        int storyFragments{ 0 };
        bool showStoryUI{ false };
        bool showMissingFragmentsUI{ false };
        double storyUITimer{ 0.0 };
        int currentStoryId{ 0 };
        int viewedBossStories{ 0 };
    };

    class Player : public CS230::GameObject
    {
    public:
        Player() : CS230::GameObject({ 0, 0 })
        {}

        static constexpr double DefaultWidth = 90.0 * 0.8;
        static constexpr double DefaultHeight = 150.0 * 0.8;
        static constexpr double PLAYER_SCALE = 1.25;

        PlayerInput input;
        PlayerMovement move;
        PlayerCombat combat;

        Math::rect rect{ { 0, 0 }, { 0, 0 } };
        mutable Math::rect collisionRectCache{ { 0, 0 }, { 0, 0 } };

        std::string currentSpritePath{ "" };
        void ChangeSprite(const std::string& path);

        void Load(Player& player, Math::vec2 pos);

        void ResetStageProgress()
        {
            combat.runTimer = 0.0;
            combat.noHitKillCount = 0;
            combat.timeRewardClaimed = false;
            combat.noHitRewardClaimed = false;
            combat.timerPaused = false; 
        }
        void Update(Math::rect ground, int worldWidth, int worldHeight, double dt);
        void Draw(Math::TransformationMatrix camM) override;

        GameObjectTypes Type() override
        {
            return GameObjectTypes::Player;
        }

        std::string TypeName() override
        {
            return "Player";
        }

        void ResolveCollision(CS230::GameObject* other_object) override;
        void ResolveCollision(GameObjectTypes type, const void* object, bool aux = false, double dt = 0.0);

        const Math::rect& GetRect() const;

        const Math::vec2 Center() const
        {
            const Math::rect& r = GetRect();
            return { r.Left() + (r.Right() - r.Left()) * 0.5, r.Bottom() + (r.Top() - r.Bottom()) * 0.5 };
        }

        bool IsRolling() const
        {
            return move.isRolling;
        }

        bool IsDashing() const
        {
            return move.isDashing;
        }

        bool IsInvincible() const
        {
            return combat.debugInvincible || combat.invincibilityTimer > 0.0 || combat.groggyTimer > 0.0 || (move.isRolling && move.rollTimer > 0.0);
        }

        bool IsAttackActive() const
        {
            return combat.attackActive;
        }

        Math::rect AttackBox() const
        {
            return combat.attackBox;
        }

        bool CanTakeHit() const
        {
            return !IsInvincible() && combat.hitCooldown <= 0.0 && combat.health > 0;
        }

        void TakeHit();
        void ApplyKnockback(double velocityX, int dir, double velocityY = 150.0);

        bool IsAlive() const
        {
            return combat.health > 0;
        }

        int GetHealth() const
        {
            return combat.health;
        }

        void SetHealth(int hp)
        {
            combat.health = hp;
            if (combat.health < 0)
            {
                combat.health = 0;
            }
        }

        void Heal(int amount);

        int GetScrap() const
        {
            return combat.scrap;
        }

        void AddScrap(int s)
        {
            combat.scrap += s;
        }

        bool UseScrap(int amount)
        {
            if (combat.scrap >= amount)
            {
                combat.scrap -= amount;
                return true;
            }
            return false;
        }

        void AddTimeGauge(double amt);

        double GetMaxGauge() const
        {
            double rightVal = timeGaugeMax - (timeGaugeMax / 10.0) * static_cast<double>(combat.slowMoGaugeReduction);
            return std::max(10.0, rightVal);
        }

        bool IsTimeSlowReady() const
        {
            return combat.timeGauge >= GetMaxGauge();
        }

        bool IsSlowMoActive() const
        {
            return combat.slowMoTimer > 0.0 || combat.externalSlowMo;
        }

        void SetSlowMotionActive(bool b)
        {
            combat.externalSlowMo = b;
        }

        void ActivateSlowMo()
        {
            combat.slowMoTimer = slowMoDurationSec + combat.slowMoDurationBonus;
            combat.timeGauge = 0.0;
        }

        void DeactivateSlowMo()
        {
            combat.slowMoTimer = 0.0;
        }

        Weapon* GetWeapon() const
        {
            return weapon.get();
        }

        void SetWeapon(std::unique_ptr<Weapon> w)
        {
            weapon = std::move(w);
            combat.weaponLocked = true;
        }

        void UnlockWeapon()
        {
            combat.weaponLocked = false;
        }

        bool IsWeaponLocked() const
        {
            return combat.weaponLocked;
        }

        void NotifyEnemyHit()
        {
            combat.enemyHitEvents++;
        }

        bool IsFacingRight() const
        {
            return move.facingRight;
        }

        double GetGroggyTimer() const
        {
            return combat.groggyTimer;
        }

        void SetGroggyTimer(double t)
        {
            combat.groggyTimer = t;
        }

        bool IsParrying() const
        {
            return combat.isParrying;
        }

        void EndParry()
        {
            combat.isParrying = false;
            combat.parryTimer = 0.0;
        }

        double GaugeSlotSize() const
        {
            return timeGaugeMax / 10.0;
        }

        void TeleportBottomLeft(const Math::vec2& pos)
        {
            double w = rect.Right() - rect.Left();
            double h = rect.Top() - rect.Bottom();
            rect.point_1 = pos;
            rect.point_2 = { pos.x + w, pos.y + h };
            SetPosition({ rect.Left(), rect.Bottom() });
        }

        void SetBottomTo(double y)
        {
            double h = rect.Top() - rect.Bottom();
            rect.point_1.y = y;
            rect.point_2.y = y + h;
            SetPosition({ rect.Left(), rect.Bottom() });
        }

        double GetRollCooldown() const
        {
            return move.rollCooldown;
        }

        double GetRollCooldownMax() const
        {
            return 1.0;
        }

        double GetTimeGaugeRatio() const
        {
            return combat.timeGauge / GetMaxGauge();
        }

        void SetWeaponLocked(bool b)
        {
            combat.weaponLocked = b;
        }

        void SetScrap(int s)
        {
            combat.scrap = s;
        }

        bool ConsumeInvincibleHitTrigger()
        {
            if (combat.invincibleHitTriggered)
            {
                combat.invincibleHitTriggered = false;
                return true;
            }
            return false;
        }

        int ConsumeEnemyHitEvents()
        {
            int e = combat.enemyHitEvents;
            combat.enemyHitEvents = 0;
            return e;
        }

        void SetSpeedMultiplier(double m)
        {
            move.speedMultiplier = m;
        }

        void DecGroggyTimer(double dt)
        {
            combat.groggyTimer -= dt;
            if (combat.groggyTimer < 0.0)
            {
                combat.groggyTimer = 0.0;
            }
        }

        bool TryConsumeDropThroughRequest()
        {
            if (move.requestDropThrough)
            {
                move.requestDropThrough = false;
                return true;
            }
            return false;
        }

        void AddStoryItem(bool isBossDrop);
        int GetStoryFragments() const
        {
            return combat.storyFragments;
        }
        void ResetStoryFragments()
        {
            combat.storyFragments = 0;
        }
        bool ShouldShowStoryUI() const
        {
            return combat.showStoryUI;
        }
        void ClearStoryUI()
        {
            combat.showStoryUI = false;
        }

        bool IsSupportedOnGround() const;

        double gaugeGainRoll{ 0.0 };
        double gaugeGainParry{ 0.0 };

        void ProcessEnemyCollision(CS230::Enemy* enemy, CS230::GameObject* enemyGO);
        void NotifyInvincibleHit();

        double runSpeed{ 480.0 };
        double jumpVelocity{ 700.0 };
        double rollDurationSec{ 0.5 };
        double parryCooldownSec{ 0.5 };
        double parryWindowSec{ 0.5 };
        double slowMoDurationSec{ 3.0 };
        double groggyDurationSec{ 0.5 };
        double groggyInvincibleDurationSec{ 2.0 };
        double timeGaugeMax{ 100.0 };
        double parrySpeedMultiplier{ 1.5 };
        double parrySpeedBuffDuration{ 3.0 };

        bool HandleParry();

        struct Afterimage
        {
            Math::TransformationMatrix matrix;
            double timer;
            double maxTime;
            Math::ivec2 framePos;
            Math::ivec2 frameSize;
            std::shared_ptr<CS230::Texture> frameTex;
            Math::vec2 hotSpot;
            bool flip;
            unsigned int tint;
        };

        std::list<Afterimage> afterimages;
        double afterimageSpawnTimer{ 0.0 };

    public:
        PlayerState_Idle state_idle;
        PlayerState_Running state_running;
        PlayerState_Jumping state_jumping;
        PlayerState_Falling state_falling;
        PlayerState_Rolling state_rolling;

        PlayerState_Ziplining state_ziplining;
        void StartZipline(double targetX, const Math::rect& zRect);

        Math::rect groundRectCache{ { 0, 0 }, { 0, 0 } };

        void update_horizontal(double dt, bool allowInput);

        void change_state(State* new_state)
        {
            GameObject::change_state(new_state);
        }

    private:
        std::unique_ptr<Weapon> weapon;
        bool collisionRemoved{ false };

        void UpdateTimers(double dt);
        void HandleDeathState(int worldWidth);
        void CheckGroundSupport();
        void UpdatePhysics(int worldWidth, int worldHeight, double dt);
        void UpdateCombat(double dt);
        void UpdateAnimations();
        void UpdateAfterimages(double dt);

        static constexpr double CoyoteTimeSec = 0.1;
        static constexpr double JumpBufferSec = 0.1;
        static constexpr double MaxFallVelocity = -1200.0;
    };
}