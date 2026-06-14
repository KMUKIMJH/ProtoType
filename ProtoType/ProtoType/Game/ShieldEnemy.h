#pragma once
#include "Enemy.h"
#include "../Game/GameObjectTypes.h"
#include "../Engine/GameObject.h"

namespace CS230
{
    class ShieldEnemy : public Enemy, public CS230::GameObject
    {
    public:
        ShieldEnemy() : CS230::GameObject({ 0,0 }) {}
        void Load(const Math::vec2& position_, int initialHealth = 0);
        void Update(const CS230::Player& player, double dt, bool isSlow);
        void Draw(Math::TransformationMatrix camM) override;
        void Unload();

        GameObjectTypes Type() override { return GameObjectTypes::ShieldEnemy; }
        std::string TypeName() override { return "ShieldEnemy"; }
        bool IsMeleeOnly() const override { return true; }

        bool IsMeleeAttackActive() const override
        {
            return (dashing && dashActive && !dashHasHit);
        }

        Math::rect GetAttackBox() const override
        {
            if (!(dashing && dashActive)) return Math::rect{ {0,0},{0,0} };

            Math::rect body = GetRect();
            double width = body.Size().x; 

            if (moveDir >= 0)
            {
                return { { body.Left(), body.Bottom() }, { body.Right() + width, body.Top() } };
            }
            else
            {
                return { { body.Left() - width, body.Bottom() }, { body.Right(), body.Top() } };
            }
        }

        void HandleDashCollision(Player& player);
        void ApplyParryDebuff(double sec) override;
        bool IsDebuffed() const override { return Enemy::IsDebuffed(); }
        void SetPatrolRange(double left, double right) { Enemy::SetPatrolRange(left, right); }
        void CancelDash();

        double retreatDistance{ 25.0 };
        double reengageDelaySec{ 0.5 };
        bool waitingForReengage{ false };
        double reengageTimer{ 0.0 };
        double attackPauseSec{ 0.5 };

        static constexpr int DefaultHealth = 4;
        static constexpr double BaseSpeed = 180.0;
        static constexpr double ChaseSpeed = 300.0;
        static constexpr double RecognizeRangeX = 300.0;
        static constexpr double DashDistanceMul = 5.0;
        static constexpr double DashSpeedMul = 3.6;
        static constexpr double DashCooldownSec = 0.5;
        static constexpr double PatrolExtentX = 200.0;
        static constexpr double TelegraphSec = 2.0;
        static constexpr double GroggyDuration = 2.0;

        bool CanIgnorePlayerAttack(const Player& player) const override;
        bool PlayerInRecognition(const Player& player) const;
        void ForceAggro(const Player& p) override;
        void OnSuccessfulHitPlayer() override;
        void EnterGroggyState(double sec = GroggyDuration);
        void OnStun() override
        {
            CancelDash();
            telegraphActive = false;
            telegraphTimer = 0.0;
            attackPauseTimer = attackPauseSec;
        }

    private:
        enum class Animations { Move = 0, Attack = 1, Hit = 2, Dead = 3, Dash = 4 };

        class State_Patrol : public Enemy::State {
        public:
            void Enter(Enemy*) override;
            void Update(Enemy*, const CS230::Player&, double, bool) override;
            void CheckExit(Enemy*, const CS230::Player&) override;
            const char* GetName() const override { return "Patrol"; }
        };

        class State_EngageDash : public Enemy::State {
        public:
            void Enter(Enemy*) override;
            void Update(Enemy*, const CS230::Player&, double, bool) override;
            void CheckExit(Enemy*, const CS230::Player&) override;
            const char* GetName() const override { return "EngageDash"; }
        };

        bool dashing{ false };
        bool dashActive{ false };
        bool dashHasHit{ false };
        double dashTimer{ 0.0 };
        double dashCooldown{ 0.0 };
        Math::rect dashHitBox{ {0,0},{0,0} };
        bool telegraphActive{ false };
        double telegraphTimer{ 0.0 };
        int dashStartDir{ 0 };
        double recognizeRangeX{ RecognizeRangeX };
        double dashDistanceMul{ DashDistanceMul };
        double dashSpeedMul{ DashSpeedMul };
        double attackPauseTimer{ 0.0 };

        State_Patrol state_patrol;
        State_EngageDash state_dash;
    };
}