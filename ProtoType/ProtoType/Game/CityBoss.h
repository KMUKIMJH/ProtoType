#pragma once
#include "Enemy.h"
#include "../Engine/GameObject.h"
#include "Player.h"
#include "../Engine/Sprite.h"

namespace CS230
{
    class CityBossEffectGO : public CS230::GameObject
    {
    public:
        CityBossEffectGO() : CS230::GameObject({ 0, 0 }) {}

        void Update(double dt) override { CS230::GameObject::Update(dt); }

        GameObjectTypes Type() override { return static_cast<GameObjectTypes>(0); }
        std::string TypeName() override { return "CityBossEffect"; }

        using CS230::GameObject::AddGOComponent;
        using CS230::GameObject::ClearGOComponents;
        using CS230::GameObject::SetScale;
    };

    struct BossShuriken
    {
        Math::rect rect;
        Math::vec2 velocity{ 0.0, 0.0 };
        double rotation{ 0.0 };
        bool active{ false };
        bool reflected{ false };
    };

    class CityBoss : public Enemy, public CS230::GameObject
    {
    public:
        CityBoss() : CS230::GameObject({ 0, 0 })
        {}

        void Load(const Math::vec2& position_, int initialHealth = 40);
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
            return "CityBoss";
        }

        bool IsMeleeOnly() const override
        {
            return true;
        }

        bool IsMeleeAttackActive() const override;
        Math::rect GetAttackBox() const override;

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
            attackApplied = true;
        }

    private:
        enum class CBState
        {
            Idle,
            Sword_Approach,
            Sword_AttackWait,
            Sword_Attack,
            Wait_3s,
            Shuriken_Teleport,
            Shuriken_Wait,
            Float_3s,
            Dash_Teleport,
            Dash_Charge,
            Dash_Sweep
        };

        CBState currentState{ CBState::Idle };
        double stateTimer{ 0.0 };
        int subCount{ 0 };
        int maxSubCount{ 0 };

        bool isPhase2{ false };
        bool attackApplied{ false };

        Math::rect swordAttackRect{ {0,0},{0,0} };
        double swordTelegraphTimer{ 0.0 };

        BossShuriken shurikens[100];

        Math::vec2 FindCenterAndFloorY(double& outMapWidth, double& outFloorY);

        CityBossEffectGO swordAttackEffect;
    };
}