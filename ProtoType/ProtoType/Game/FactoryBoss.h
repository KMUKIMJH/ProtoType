#pragma once
#include "Enemy.h"
#include "../Engine/GameObject.h"
#include "Player.h"

namespace CS230
{
    class BossEffectGO : public CS230::GameObject
    {
    public:
        BossEffectGO() : CS230::GameObject({ 0, 0 }) {}

        void Update(double dt) override { CS230::GameObject::Update(dt); }
        GameObjectTypes Type() override { return static_cast<GameObjectTypes>(0); }
        std::string TypeName() override { return "BossEffect"; }

        using CS230::GameObject::AddGOComponent;
        using CS230::GameObject::ClearGOComponents;
        using CS230::GameObject::SetScale;
    };

    struct BossWave
    {
        Math::rect rect;
        double velocityX{ 0.0 };
        bool active{ false };

        BossEffectGO effect;
    };

    class FactoryBoss : public Enemy, public CS230::GameObject
    {
    public:
        FactoryBoss() : CS230::GameObject({ 0, 0 })
        {}

        void Load(const Math::vec2& position_, const Math::vec2& size, int initialHealth = 20);
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
            return "Boss";
        }

        bool IsMeleeOnly() const override
        {
            return true;
        }

        bool IsMeleeAttackActive() const override
        {
            if (currentState == FB1State::Jump && jumpHitApplied) return false;
            if (strikeActive && strikeApplied) return false;

            return currentState == FB1State::Wave || strikeActive || currentState == FB1State::Jump;
        }

        Math::rect GetAttackBox() const override
        {
            if (currentState == FB1State::Jump)
            {
                if (jumpHitApplied) return Math::rect{ { 0, 0 }, { 0, 0 } };
                return GetRect();
            }
            if (strikeActive)
            {
                if (strikeApplied) return Math::rect{ { 0, 0 }, { 0, 0 } };
                return strikeRect;
            }
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
            jumpHitApplied = true;
            strikeApplied = true;
        }

    private:
        enum class FB1State
        {
            Idle,
            Crouch,
            Jump,
            Wave,
            Strike
        };

        FB1State currentState{ FB1State::Idle };
        double stateTimer{ 0.0 };

        bool jumpHitApplied{ false };

        BossWave waves[2];

        int strikeCount{ 0 };
        bool strikeActive{ false };
        bool strikeApplied{ false };
        double strikeTimer{ 0.0 };
        Math::rect strikeRect{ { 0, 0 }, { 0, 0 } };

        BossEffectGO strikeEffect;
    };
}