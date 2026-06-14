#pragma once
#include "Player.h"
#include "../Engine/Rect.h"
#include "../Engine/Vec2.h"
#include "../Engine/Component.h"
#include "../Engine/Matrix.h"
#include "../Game/GameObjectTypes.h"
#include <memory>
#include <vector>
#include <algorithm>

namespace CS230
{
    class Texture;
}

namespace CS230
{
    class Enemy : public CS230::Component
    {
    public:
        Enemy();
        ~Enemy() override;

        static constexpr double SlowTimeScale = 0.33333333;
        static constexpr double PatrolHalfDefault = 300.0;
        static constexpr double PortalWidth = 100.0;
        static constexpr double PortalHeight = 150.0;
        static constexpr double SlowMotionBaseDuration = 3.0;
        static constexpr double DefaultWidth = 100.0;
        static constexpr double DefaultHeight = 165.0;

        class State
        {
        public:
            virtual ~State() = default;
            virtual void Enter(Enemy* e) = 0;
            virtual void Update(Enemy* e, const Player& player, double dt, bool isSlow) = 0;
            virtual void CheckExit(Enemy* e, const Player& player) = 0;
            virtual const char* GetName() const = 0;
        };

        void ChangeState(State* next)
        {
            current_state = next;
            if (current_state) current_state->Enter(this);
        }

        void UpdateState(const Player& player, double dt, bool isSlow)
        {
            if (current_state)
            {
                current_state->Update(this, player, dt, isSlow);
                current_state->CheckExit(this, player);
            }
        }

        void Update(double) override {}
        virtual bool CanIgnorePlayerAttack(const Player& player) const
        {
            (void)player;
            return false;
        }
        virtual void ApplyKnockback(double amount, int dir);
        void ResolveCollision(GameObjectTypes type, const void* object, bool aux = false, double dt = 0.0);

        void SnapToGround(const Math::rect& platform)
        {
            double h = rect.Top() - rect.Bottom();
            rect.point_1.y = platform.Top();
            rect.point_2.y = platform.Top() + h;
            velocity.y = 0.0;
            isGrounded = true;
            hasStandingRect = true;
            standingRect = platform;
            standingOn = nullptr;
        }

        void ApplyGravity(double dt);
        void NotifyHit(const Player* p = nullptr);

        virtual const Math::rect& GetRect() const;
        virtual bool IsBoss() const { return false; }
        virtual int GetHealth() const
        {
            return health;
        }
        virtual void SetHealth(int h)
        {
            health = h;
            if (health < 0) health = 0;
        }
        virtual bool IsActive() const
        {
            return active;
        }

        virtual void SetBottomTo(double y)
        {
            double h = rect.Top() - rect.Bottom();
            rect.point_1.y = y;
            rect.point_2.y = y + h;
        }

        virtual bool IsMeleeOnly() const
        {
            return false;
        }
        virtual bool IsMeleeAttackActive() const
        {
            return false;
        }
        virtual Math::rect GetAttackBox() const
        {
            return Math::rect{ { 0, 0 }, { 0, 0 } };
        }
        virtual int& ShootTimer()
        {
            static int dummy = 0;
            return dummy;
        }
        virtual int ShootTimer() const
        {
            return 0;
        }

        virtual void ApplyParryDebuff(double secs)
        {
            (void)secs;
            isParryGroggy = true;
        }
        virtual bool IsDebuffed() const
        {
            return false;
        }
        void UpdateDebuffTimer(double dt)
        {
            (void)dt;
        }

        virtual void OnSuccessfulHitPlayer() {}

        double ClampToPatrol(double dx)
        {
            if (!patrolSet) return dx;

            if (rect.Right() > patrolRight)
            {
                if (moveDir > 0)
                {
                    moveDir = -1;
                    return 0.0;
                }
                return dx;
            }

            if (rect.Left() < patrolLeft)
            {
                if (moveDir < 0)
                {
                    moveDir = 1;
                    return 0.0;
                }
                return dx;
            }

            double nextL = rect.Left() + dx;
            double nextR = rect.Right() + dx;

            if (moveDir > 0 && nextR > patrolRight)
            {
                dx = patrolRight - rect.Right();
                moveDir = -1;
            }
            else if (moveDir < 0 && nextL < patrolLeft)
            {
                dx = patrolLeft - rect.Left();
                moveDir = 1;
            }
            return dx;
        }

        double ApplyPlatformEdge(double dx);

        void DrawHP(const Math::TransformationMatrix& camM) const;
        void DrawTelegraph(const Math::TransformationMatrix& camM) const;

        double Width() const
        {
            return GetRect().Right() - GetRect().Left();
        }
        double Height() const
        {
            return GetRect().Top() - GetRect().Bottom();
        }
        Math::vec2 Center() const
        {
            return { GetRect().Left() + Width() * 0.5, GetRect().Bottom() + Height() * 0.5 };
        }

        static std::vector<Enemy*>& Registry();

    protected:
        State* current_state{ nullptr };
        static std::vector<Enemy*>& AllEnemies()
        {
            static std::vector<Enemy*> reg;
            return reg;
        }
        void RegisterEnemy()
        {
            auto& v = AllEnemies();
            if (std::find(v.begin(), v.end(), this) == v.end()) v.push_back(this);
        }
        void UnregisterEnemy()
        {
            auto& v = AllEnemies();
            v.erase(std::remove(v.begin(), v.end(), this), v.end());
        }

    public:
        Math::rect rect{ { 0, 0 }, { 0, 0 } };
        mutable Math::rect collisionRectCache{ { 0, 0 }, { 0, 0 } };
        int health{ 0 };
        bool active{ true };
        mutable std::shared_ptr<CS230::Texture> hpTexture;
        mutable std::shared_ptr<CS230::Texture> groggyTexture;
        mutable std::shared_ptr<CS230::Texture> debuffTexture;
        mutable int lastHP{ -1 };
        double hitCooldownSec{ 0.0 };
        double groggyTimer{ 0.0 };
        bool isParryGroggy{ false };
        double playerHitCooldownSec{ 0.0 };
        int moveDir{ 1 };
        double patrolLeft{ 0.0 };
        double patrolRight{ 0.0 };
        bool patrolSet{ false };
        double baseSpeed{ 180.0 };
        double chaseSpeed{ 300.0 };
        Math::vec2 velocity{ 0.0, 0.0 };
        bool isGrounded{ false };
        Math::rect standingRect{ { 0, 0 }, { 0, 0 } };
        bool hasStandingRect{ false };
        CS230::GameObject* standingOn{ nullptr };
        bool allowLeavePlatform{ false };
        int maxHealth{ 0 };
        double aggroLockTimer{ 0.0 };
        bool isDraggedByHammer{ false };
        double lastDt{ 0.016 };

        bool dropsStoryItem{ false };
        void SetDropsStoryItem(bool b) { dropsStoryItem = b; }

        virtual void ForceAggro(const Player& p)
        {
            (void)p;
        }

        void SetPatrolRange(double left, double right)
        {
            patrolLeft = left;
            patrolRight = right;
            patrolSet = true;
        }

        virtual void OnStun() {}

        void ApplyStunAndKnockbackFromPlayer(const Player& player, double stunSec = 0.2, double knockback = 10.0, bool pull = false);

        void UpdateGroggyTimer(double dt)
        {
            if (groggyTimer > 0.0)
            {
                groggyTimer -= dt;
                if (groggyTimer < 0.0) groggyTimer = 0.0;
            }
        }
        bool IsGroggy() const
        {
            return groggyTimer > 0.0;
        }

        void SpawnDeathReward();
        void DrawStatus(Math::TransformationMatrix camM);
    };

    Math::vec2 Delta(const Enemy& e, const Player& p);
    double DistanceSq(const Enemy& e, const Player& p);
    bool IsBlockedByPlatform(const Math::vec2& p1, const Math::vec2& p2);
}