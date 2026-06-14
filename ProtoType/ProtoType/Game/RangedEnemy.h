#pragma once
#include "Enemy.h"
#include "../Game/GameObjectTypes.h"
#include "../Engine/Rect.h"
#include "../Engine/Vec2.h"
#include "../Engine/GameObject.h"
#include "../Engine/Texture.h"
#include "Bullet.h"
#include <vector>
#include <algorithm>
#include <string>

namespace CS230 { class Player; }

namespace CS230
{
    class RangedEnemy : public Enemy, public CS230::GameObject
    {
    public:
        static constexpr int DefaultHealth = 2;
        static constexpr int MAX_BULLETS = 3;

        RangedEnemy() : CS230::GameObject({ 0,0 }) {}

        ~RangedEnemy() override
        {
            auto& reg = CS230::RangedEnemy::Registry();
            reg.erase(std::remove(reg.begin(), reg.end(), this), reg.end());
        }

        void Load(const Math::vec2& position_, int initialHealth = DefaultHealth);
        void Update(const CS230::Player& player, double dt, bool isSlow);
        void Draw(Math::TransformationMatrix camM) override;

        inline void Unload()
        {
            auto& reg = CS230::RangedEnemy::Registry();
            reg.erase(std::remove(reg.begin(), reg.end(), this), reg.end());
            for (int i = 0; i < MAX_BULLETS; ++i) bullets[i].Reset();
            ClearGOComponents(); active = false;
        }

        GameObjectTypes Type() override { return GameObjectTypes::RangedEnemy; }
        std::string TypeName() override { return "RangedEnemy"; }
        void SetBottomTo(double y) override { Enemy::SetBottomTo(y); }
        bool IsMeleeOnly() const override { return false; }
        bool IsMeleeAttackActive() const override { return false; }
        Math::rect GetAttackBox() const override { return { {0,0},{0,0} }; }
        int& ShootTimer() override { return Enemy::ShootTimer(); }
        int ShootTimer() const override { return static_cast<int>(cooldownTimer * 60.0); }
        Bullet* Bullets() { return bullets; }
        void ApplyParryDebuff(double seconds) override { Enemy::ApplyParryDebuff(seconds); }
        bool IsDebuffed() const override { return Enemy::IsDebuffed(); }
        void OnStun() override { telegraphActive = false; telegraphTimer = 0.0; }

        static constexpr double BulletRange = 72.0 * 30.0;
        static constexpr double RetreatDistance = 72.0 * 4.0;
        static constexpr double TelegraphSec = 0.5;
        static constexpr double RecognizeRangeX = 72.0 * 15.0;

        static std::vector<RangedEnemy*>& Registry();
        static void ClearAllBullets();
        void ForceAggro(const Player& p) override;

    private:
        class State_Patrol : public Enemy::State {
        public:
            void Enter(Enemy* e) override;
            void Update(Enemy* e, const CS230::Player& player, double dt, bool isSlow) override;
            void CheckExit(Enemy* e, const CS230::Player& player) override;
            const char* GetName() const override { return "Patrol"; }
        };
        class State_EngageShooter : public Enemy::State {
        public:
            void Enter(Enemy* e) override;
            void Update(Enemy* e, const CS230::Player& player, double dt, bool isSlow) override;
            void CheckExit(Enemy* e, const CS230::Player& player) override;
            const char* GetName() const override { return "EngageShooter"; }
        };

        CS230::Bullet bullets[MAX_BULLETS]{};
        double cooldownTimer{ 0.0 };
        bool telegraphActive{ false };
        double telegraphTimer{ 0.0 };
        double fleeDashRemaining{ 0.0 };
        int fleeDashDir{ 0 };
        bool hasRetreatedThisCycle{ false };

        State_Patrol state_patrol;
        State_EngageShooter state_shooter;
        enum class Animations { Move = 0, Attack = 1, Hit = 2, Dead = 3, Patrol = 4, Shoot = 5, Idle = 6 };
    };
}