#pragma once
#include "Enemy.h"
#include "../Game/GameObjectTypes.h"
#include "../Engine/GameObject.h"

namespace CS230
{
    class Player;
    class MeleeEnemy : public Enemy, public CS230::GameObject
    {
    public:
        MeleeEnemy() : CS230::GameObject({ 0,0 }) {}
        void Load(const Math::vec2& position_, int initialHealth = 0);
        void Update(const CS230::Player& player, double dt, bool isSlow);
        void Draw(Math::TransformationMatrix camM) override;
        void Unload();

        GameObjectTypes Type() override { return GameObjectTypes::MeleeEnemy; }
        std::string TypeName() override { return "MeleeEnemy"; }

        Math::rect GetAttackBox() const override
        {
            if (!meleeAttackActive) return { {0,0}, {0,0} };

            Math::rect actualBody = GetRect();

            const double w = actualBody.Size().x * 0.85;

            if (moveDir >= 0)
            {
                return { { actualBody.Right(), actualBody.Bottom() }, { actualBody.Right() + w, actualBody.Top() } };
            }
            else
            {
                return { { actualBody.Left() - w, actualBody.Bottom() }, { actualBody.Left(), actualBody.Top() } };
            }
        }

        void ApplyParryDebuff(double sec) override { Enemy::ApplyParryDebuff(sec); CancelAttack(); }
        bool IsDebuffed() const override { return Enemy::IsDebuffed(); }

        static constexpr double AttackDurationSec = 0.5;
        static constexpr double TelegraphSec = 0.5;
        static constexpr double ChaseRangeX = 72.0 * 12.0;
        static constexpr double AttackPauseSec = 2.0;

        void CancelAttack()
        {
            meleeAttackActive = false;
            meleeAttackTimer = 0.0;
            jumpActive = false;
        }
        void StartAttack();
        bool IsTelegraphActive() const { return telegraphActive; }
        void CancelTelegraphAndDebuff();
        void OnSuccessfulHitPlayer() override;
        void ResolveCollision(CS230::GameObject* other_object) override;

        bool IsMeleeAttackActive() const override { return meleeAttackActive && !attackHitOccurred; }
        bool IsMeleeOnly() const override { return true; }

        void OnParriedByPlayer(Player& player);
        void OnStun() override
        {
            CancelAttack();
            CancelTelegraphAndDebuff();
            meleeAttackPauseTimer = AttackPauseSec;
        }
        void NotifyAttackDodged() { attackHitOccurred = true; }
        bool PlayerInRecognition(const Player& player) const;
        void ForceAggro(const Player& p) override;

    private:
        enum class Animations { Move = 0, Attack = 1, Hit = 2, Dead = 3, Patrol = 4, Idle = 5 };

        class State_Patrol : public Enemy::State {
        public:
            void Enter(Enemy*) override;
            void Update(Enemy*, const CS230::Player&, double, bool) override;
            void CheckExit(Enemy*, const CS230::Player&) override;
            const char* GetName() const override { return "Patrol"; }
        };
        class State_EngageMelee : public Enemy::State {
        public:
            void Enter(Enemy*) override;
            void Update(Enemy*, const CS230::Player&, double, bool) override;
            void CheckExit(Enemy*, const CS230::Player&) override;
            const char* GetName() const override { return "EngageMelee"; }
        };

        double meleeAttackTimer{ 0.0 };
        bool meleeAttackActive{ false };
        double meleeAttackPauseTimer{ 0.0 };
        bool jumpActive{ false };
        double leapTimer{ 0.0 };

        double leapVerticalSpeed{ 0.0 };
        double leapHorizontalSpeed{ 1440.0 };

        bool attackHitOccurred{ false };
        bool telegraphActive{ false };
        double telegraphTimer{ 0.0 };
        bool waitingAfterHit{ false };

        State_Patrol state_patrol;
        State_EngageMelee state_melee;
    };
}
