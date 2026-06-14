#pragma once
#include "Enemy.h"
#include "../Engine/GameObject.h"
#include "Player.h"

namespace CS230
{
    struct BossOrb
    {
        Math::rect rect;
        Math::vec2 velocity{ 0.0, 0.0 };
        double timer{ 0.0 };
        double explodeTimer{ 0.0 };
        bool active{ false };
        bool grounded{ false };
        bool exploding{ false };
        bool damageApplied{ false };
    };

    class ForestBoss : public Enemy, public CS230::GameObject
    {
    public:
        ForestBoss() : CS230::GameObject({ 0, 0 })
        {
        }

        void Load(const Math::vec2& position_, int initialHealth = 20);
        void Update(const CS230::Player& player, double dt, bool isSlow);
        void Draw(Math::TransformationMatrix camM) override;
        void Unload();
        bool IsBoss() const override { return true; }

        GameObjectTypes Type() override
        {
            return GameObjectTypes::MeleeEnemy;
        }

        std::string TypeName() override
        {
            return "ForestBoss";
        }

        bool IsMeleeOnly() const override
        {
            return true;
        }

        bool IsMeleeAttackActive() const override
        {
            return laserActive;
        }

        Math::rect GetAttackBox() const override
        {
            return Math::rect{ { 0, 0 }, { 0, 0 } };
        }

        int& ShootTimer() override
        {
            static int dummy = 0;
            return dummy;
        }

        int ShootTimer() const override
        {
            return 0;
        }

        void ApplyParryDebuff(double) override
        {
        }
        BossOrb orbs[5];
    private:
        enum class FB2State
        {
            Idle,
            Charge,
            FireOrbs,
            Laser
        };

        FB2State currentState{ FB2State::Idle };
        double stateTimer{ 0.0 };

        bool laserActive{ false };
        bool laserMovesRight{ true };
        bool laserDamageApplied{ false }; 

        Math::vec2 laserP1{ 0.0, 0.0 };
        Math::vec2 laserP2{ 0.0, 0.0 };
    };
}