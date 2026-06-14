#pragma once
#include "../Engine/Rect.h"
#include "../Engine/Matrix.h"
#include "Bullet.h"
#include <vector>
#include <memory>

namespace CS230
{
    class Player;
    class Enemy;

    enum class WeaponType
    {
        Sword,
        Hammer,
        Arrow
    };

    struct HitResult
    {
        Enemy* enemy;
        bool strong;
        bool isProjectile;
        double damageMult{ 1.0 };
    };

    class Weapon
    {
    public:
        virtual ~Weapon() = default;
        virtual void Update(Player& player, double dt) = 0;
        virtual bool TryStartAttack(Player& player) = 0;
        virtual void EndAttack(Player& player) = 0;
        virtual Math::rect GetAttackBox(const Player& player) const = 0;
        virtual std::vector<HitResult> ResolveHits(Player& player) = 0;
        virtual void Reset() = 0;
        virtual WeaponType GetType() const = 0;
        virtual void Draw(Math::TransformationMatrix camM)
        {
            (void)camM;
        }
        virtual bool IsAttackActive() const = 0;
    };

    class SwordWeapon : public Weapon
    {
    public:
        void Update(Player& player, double dt) override;
        bool TryStartAttack(Player& player) override;
        void EndAttack(Player& player) override;
        Math::rect GetAttackBox(const Player& player) const override;
        std::vector<HitResult> ResolveHits(Player& player) override;
        void Reset() override;

        WeaponType GetType() const override { return WeaponType::Sword; }
        bool IsAttackActive() const override { return attackActive; }

        double swordRangeMult{ 1.0 };
        double swordDmgBonus{ 0.0 };
        int maxCombo{ 4 };

    private:
        bool attackActive{ false };
        double attackTimer{ 0.0 };
        double lockoutTimer{ 0.0 };
        int comboCount{ 0 };
        double comboTimeout{ 0.0 };
    };

    class HammerWeapon : public Weapon
    {
    public:
        void Update(Player& player, double dt) override;
        bool TryStartAttack(Player& player) override;
        void EndAttack(Player& player) override;
        Math::rect GetAttackBox(const Player& player) const override;
        std::vector<HitResult> ResolveHits(Player& player) override;
        void Reset() override;

        WeaponType GetType() const override { return WeaponType::Hammer; }
        bool IsAttackActive() const override { return attackActive; }
        bool IsCharging() const { return isCharging; }
        double hammerRangeMult{ 1.0 };
        double hammerDmgBonus{ 0.0 };

    private:
        bool attackActive{ false };
        double attackTimer{ 0.0 };
        double lockoutTimer{ 0.0 };
        bool isCharging{ false };
    };

    class ArrowWeapon : public Weapon
    {
    public:
        static constexpr int MAX_AMMO = 15;

        ArrowWeapon();
        void Update(Player& player, double dt) override;
        bool TryStartAttack(Player& player) override;
        void EndAttack(Player& player) override;
        Math::rect GetAttackBox(const Player& player) const override;
        std::vector<HitResult> ResolveHits(Player& player) override;
        void Reset() override;

        WeaponType GetType() const override { return WeaponType::Arrow; }
        void Draw(Math::TransformationMatrix camM) override;
        bool IsAttackActive() const override { return meleeMode; }
        bool IsShooting() const { return shootTimer > 0.0; }

        int GetAmmo() const { return currentAmmo; }
        void SetAmmo(int a) { currentAmmo = a; }
        int GetMaxAmmo() const { return maxAmmoRuntime; }

        int maxAmmoRuntime{ 5 };
        double arrowMeleeRangeMult{ 1.0 };
        double arrowMeleeDmgBonus{ 0.0 };

    private:
        int currentAmmo{ 5 };
        bool meleeMode{ false };
        double meleeTimer{ 0.0 };
        double meleeLockoutTimer{ 0.0 };
        bool meleeEmpty{ false };
        double shootTimer{ 0.0 };

        CS230::Bullet bullets[MAX_AMMO];

        struct StuckArrow
        {
            Enemy* target;
            Math::vec2 offset;
            double rotation;
            int hitEnemyDir;
        };
        std::vector<StuckArrow> stuckArrows;

        struct DroppedArrow
        {
            Math::vec2 pos;
            double rotation;
            bool isStuckToWall{ false };
            Math::vec2 velocity{ 0.0, 0.0 };
        };
        std::vector<DroppedArrow> droppedArrows;
    };

    std::unique_ptr<Weapon> MakeWeapon(WeaponType t);
}