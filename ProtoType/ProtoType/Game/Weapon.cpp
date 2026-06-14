#include "Weapon.h"
#include "Player.h"
#include "Enemy.h"
#include "ForestBoss.h"
#include "ShieldEnemy.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "Bullet.h"
#include "../Stage/StageManager.h"
#include "KeyBinding.h"
#include <cmath>

namespace CS230
{
    static constexpr double kPlayerWidth = 72.0;

    static std::vector<HitResult> CollectHits(const Player& player, const Math::rect& atkRect, bool strong)
    {
        (void)player;
        std::vector<HitResult> hits;
        for (auto* e : Enemy::Registry())
        {
            if (!e || !e->IsActive() || e->GetHealth() <= 0)
            {
                continue;
            }

            const auto& er = e->GetRect();
            if (Math::IntersectsRect(atkRect, er))
            {
                if (IsBlockedByPlatform(player.Center(), e->Center()))
                {
                    continue;
                }
                if (e->playerHitCooldownSec > 0.0)
                {
                    continue;
                }
                hits.push_back({ e, strong, false, 1.0 });
            }
        }
        return hits;
    }

    void SwordWeapon::Reset()
    {
        comboCount = 0;
        comboTimeout = 0.0;
        lockoutTimer = 0.0;
        attackTimer = 0.0;
        attackActive = false;
    }

    bool SwordWeapon::TryStartAttack(Player& player)
    {
        if (lockoutTimer > 0.0 || attackActive)
        {
            return false;
        }

        if (CS230::KeyBinding::GetInstance().IsActionKeyJustPressed(CS230::Action::Attack))
        {
            attackActive = true;
            attackTimer = 0.2;
            comboTimeout = 1.0;
            comboCount++;

            if (comboCount >= 4 + player.combat.swordComboBonus)
            {
                lockoutTimer = 0.6;
                comboCount = 0;
            }
            return true;
        }
        return false;
    }

    void SwordWeapon::Update(Player& player, double dt)
    {
        (void)player;
        if (attackActive)
        {
            attackTimer -= dt;
            if (attackTimer <= 0.0)
            {
                attackActive = false;
            }
        }
        if (lockoutTimer > 0.0)
        {
            lockoutTimer -= dt;
        }
        if (comboTimeout > 0.0 && !attackActive)
        {
            comboTimeout -= dt;
            if (comboTimeout <= 0.0)
            {
                comboCount = 0;
            }
        }
    }

    Math::rect SwordWeapon::GetAttackBox(const Player& player) const
    {
        const double reach = kPlayerWidth * 1.5 * (1.0 + player.combat.swordRangeBonus);
        const double atkH = player.GetRect().Size().y;
        if (player.IsFacingRight())
        {
            return { { player.GetRect().Right(), player.GetRect().Bottom() }, { player.GetRect().Right() + reach, player.GetRect().Bottom() + atkH } };
        }
        else
        {
            return { { player.GetRect().Left() - reach, player.GetRect().Bottom() }, { player.GetRect().Left(), player.GetRect().Bottom() + atkH } };
        }
    }

    void SwordWeapon::EndAttack(Player& player)
    {
        (void)player;
        attackActive = false;
        attackTimer = 0.0;
    }

    std::vector<HitResult> SwordWeapon::ResolveHits(Player& player)
    {
        if (!attackActive)
        {
            return {};
        }
        return CollectHits(player, GetAttackBox(player), false);
    }

    void HammerWeapon::Reset()
    {
        lockoutTimer = 0.0;
        attackTimer = 0.0;
        attackActive = false;
        isCharging = false;
    }

    bool HammerWeapon::TryStartAttack(Player& player)
    {
        if (lockoutTimer > 0.0 || attackActive || isCharging)
        {
            return false;
        }

        if (CS230::KeyBinding::GetInstance().IsActionKeyJustPressed(CS230::Action::Attack))
        {
            if (player.combat.hammerPullOnCharge)
            {
                isCharging = true;
                return true;
            }
            else
            {
                attackActive = true;
                attackTimer = 0.15 * (1.0 - player.combat.hammerChargeReduction); 
                lockoutTimer = 0.8 * (1.0 - player.combat.hammerChargeReduction);
                return true;
            }
        }
        return false;
    }

    void HammerWeapon::Update(Player& player, double dt)
    {
        (void)player;
        if (isCharging)
        {
            if (!CS230::KeyBinding::GetInstance().IsActionKeyDown(CS230::Action::Attack))
            {
                isCharging = false;
                attackActive = true;
                attackTimer = 0.25 * (1.0 - player.combat.hammerChargeReduction); 
                lockoutTimer = 0.8 * (1.0 - player.combat.hammerChargeReduction);
            }
        }
        else if (attackActive)
        {
            attackTimer -= dt;
            if (attackTimer <= 0.0)
            {
                attackActive = false;
            }
        }

        if (lockoutTimer > 0.0)
        {
            lockoutTimer -= dt;
        }
    }

    Math::rect HammerWeapon::GetAttackBox(const Player& player) const
    {
        const double reach = kPlayerWidth * 3.0 * (1.0 + player.combat.hammerRangeBonus);
        const double atkH = player.GetRect().Size().y;
        if (player.IsFacingRight())
        {
            return { { player.GetRect().Right(), player.GetRect().Bottom() }, { player.GetRect().Right() + reach, player.GetRect().Bottom() + atkH } };
        }
        else
        {
            return { { player.GetRect().Left() - reach, player.GetRect().Bottom() }, { player.GetRect().Left(), player.GetRect().Bottom() + atkH } };
        }
    }

    void HammerWeapon::EndAttack(Player& player)
    {
        (void)player;
        attackActive = false;
        attackTimer = 0.0;
    }

    std::vector<HitResult> HammerWeapon::ResolveHits(Player& player)
    {
        if (!attackActive)
        {
            return {};
        }
        return CollectHits(player, GetAttackBox(player), true);
    }

    ArrowWeapon::ArrowWeapon()
    {
        for (int i = 0; i < MAX_AMMO; ++i)
        {
            bullets[i].Reset();
            bullets[i].rect = { { 0, 0 }, { 60.0, 15.0 } };
            bullets[i].InitSprite("Assets/image/Objects/Arrow.spt", { 60.0 / 512.0, 15.0 / 256.0 });
            bullets[i].owner = Bullet::Owner::Player;
        }
    }

    void ArrowWeapon::Reset()
    {
        meleeMode = false;
        meleeTimer = 0.0;
        meleeLockoutTimer = 0.0;
        shootTimer = 0.0;
        meleeEmpty = false;
        for (int i = 0; i < MAX_AMMO; ++i)
        {
            bullets[i].active = false;
        }
        stuckArrows.clear();
        droppedArrows.clear();
    }

    bool ArrowWeapon::TryStartAttack(Player& player)
    {
        if (meleeTimer > 0.0 || meleeLockoutTimer > 0.0 || shootTimer > 0.0) return false;

        if (CS230::KeyBinding::GetInstance().IsActionKeyJustPressed(CS230::Action::Attack))
        {
            bool enemyInMelee = false;
            const double meleeReach = kPlayerWidth * 1.0 * (1.0 + player.combat.arrowMeleeRangeBonus);
            const double atkH = player.GetRect().Size().y;
            Math::rect mBox;

            if (player.IsFacingRight()) mBox = { { player.GetRect().Right(), player.GetRect().Bottom() }, { player.GetRect().Right() + meleeReach, player.GetRect().Bottom() + atkH } };
            else mBox = { { player.GetRect().Left() - meleeReach, player.GetRect().Bottom() }, { player.GetRect().Left(), player.GetRect().Bottom() + atkH } };

            for (auto* e : Enemy::Registry())
            {
                if (!e || !e->IsActive() || e->GetHealth() <= 0) continue;

                if (Math::IntersectsRect(mBox, e->GetRect()))
                {
                    enemyInMelee = true;
                    break;
                }

                if (auto* fb = dynamic_cast<CS230::ForestBoss*>(e))
                {
                    for (int i = 0; i < 5; ++i)
                    {
                        if (fb->orbs[i].active && Math::IntersectsRect(mBox, fb->orbs[i].rect))
                        {
                            enemyInMelee = true;
                            break;
                        }
                    }
                }

                if (enemyInMelee) break;
            }

            if (enemyInMelee || currentAmmo <= 0)
            {
                meleeMode = true;
                meleeTimer = 0.2; 
                meleeLockoutTimer = 0.5; 
                meleeEmpty = (currentAmmo <= 0);
                return true;
            }

            for (int i = 0; i < MAX_AMMO; ++i)
            {
                if (!bullets[i].active)
                {
                    currentAmmo--;
                    auto& b = bullets[i];
                    b.Reset();
                    b.active = true;
                    b.owner = Bullet::Owner::Player;

                    double cx = player.Center().x;
                    double cy = player.Center().y;
                    int shootDir = player.IsFacingRight() ? 1 : -1;

                    double heightOffset = 14.0;
                    cy += heightOffset;

                    double bcenX = cx - (shootDir * 60.0 * 0.45);

                    b.rect = { { bcenX - 30.0, cy - 7.5 }, { bcenX + 30.0, cy + 7.5 } };
                    b.spawnCenter = { cx, cy };

                    b.velocity = Math::vec2((double)shootDir, 0.0) * (25.0 * 60.0);
                    b.velocity.y = 300.0;
                    b.gravity = 1510.0;

                    b.rotation = (shootDir < 0) ? 3.14159265358979323846 : 0.0;

                    b.SetPosition({ bcenX, cy });

                    shootTimer = 0.25;
                    return true;
                }
            }
        }
        return false;
    }

    void ArrowWeapon::Update(Player& player, double dt)
    {
        if (meleeTimer > 0.0)
        {
            meleeTimer -= dt;
            if (meleeTimer <= 0.0) meleeMode = false;
        }
        if (meleeLockoutTimer > 0.0)
        {
            meleeLockoutTimer -= dt;
        }
        if (shootTimer > 0.0)
        {
            shootTimer -= dt;
        }

        auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());
        double timeScale = player.IsSlowMoActive() ? Enemy::SlowTimeScale : 1.0;

        for (int i = 0; i < MAX_AMMO; ++i)
        {
            auto& b = bullets[i];
            if (!b.active) continue;

            b.Update(dt * timeScale);
            bool hitWall = false;

            Math::vec2 bcen = { b.rect.Left() + b.rect.Size().x * 0.5, b.rect.Bottom() + b.rect.Size().y * 0.5 };
            Math::vec2 tipCenter = {
                bcen.x + std::cos(b.rotation) * (b.rect.Size().x * 0.45),
                bcen.y + std::sin(b.rotation) * (b.rect.Size().x * 0.45)
            };
            Math::rect tipRect = { tipCenter - Math::vec2{2.0, 2.0}, tipCenter + Math::vec2{2.0, 2.0} };

            if (sm)
            {
                for (auto* obj : sm->staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
                    {
                        auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                        if (rc && Math::IntersectsRect(tipRect, rc->WorldBoundary()))
                        {
                            hitWall = true;
                            break;
                        }
                    }
                }
            }

            if (hitWall || std::abs(b.rect.Left() - b.spawnCenter.x) > 3000.0 || b.rect.Bottom() < -1000.0)
            {
                b.active = false;

                DroppedArrow da;
                da.pos = bcen;
                da.rotation = b.rotation;
                da.isStuckToWall = hitWall;
                da.velocity = { 0.0, -400.0 };
                droppedArrows.push_back(da);
            }
        }

        for (auto it = stuckArrows.begin(); it != stuckArrows.end();)
        {
            bool targetExists = false;
            for (auto* e : Enemy::Registry())
            {
                if (e == it->target) { targetExists = true; break; }
            }

            if (!targetExists || !it->target->IsActive() || it->target->GetHealth() <= 0)
            {
                if (targetExists)
                {
                    double currentOffsetX = (it->target->moveDir == it->hitEnemyDir) ? it->offset.x : -it->offset.x;
                    Math::vec2 dropPos = it->target->Center() + Math::vec2{ currentOffsetX, it->offset.y };

                    double currentRotation = (it->target->moveDir == it->hitEnemyDir) ? it->rotation : (3.14159265358979323846 - it->rotation);

                    DroppedArrow da;
                    da.pos = dropPos;
                    da.rotation = currentRotation;
                    da.isStuckToWall = false;
                    da.velocity = { 0.0, -400.0 };
                    droppedArrows.push_back(da);
                }
                it = stuckArrows.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (auto& da : droppedArrows)
        {
            if (da.isStuckToWall) continue;

            if (da.velocity.x != 0.0 || da.velocity.y > 0.0)
            {
                da.velocity.y -= 1300.0 * dt * timeScale;
            }
            else
            {
                da.velocity.y = -400.0;
            }

            double nextX = da.pos.x + da.velocity.x * dt * timeScale;
            double nextY = da.pos.y + da.velocity.y * dt * timeScale;

            bool isOnFloor = false;
            double highestFloorY = -999999.0;

            if (sm)
            {
                for (auto* obj : sm->staticObjects)
                {
                    if (obj->Type() == GameObjectTypes::Platform || obj->Type() == GameObjectTypes::TwoWayPlatform)
                    {
                        auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                        if (rc)
                        {
                            Math::rect wallRect = rc->WorldBoundary();

                            if (nextX >= wallRect.Left() && nextX <= wallRect.Right())
                            {
                                if (da.pos.y >= wallRect.Top() - 10.0 && nextY <= wallRect.Top())
                                {
                                    if (wallRect.Top() > highestFloorY)
                                    {
                                        highestFloorY = wallRect.Top();
                                        isOnFloor = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (isOnFloor)
            {
                da.pos.x = nextX;
                da.pos.y = highestFloorY + 7.5;
                da.rotation = (std::cos(da.rotation) >= 0) ? 0.0 : 3.14159265358979323846;
                da.isStuckToWall = true;
                da.velocity = { 0.0, 0.0 };
            }
            else
            {
                da.pos.x = nextX;
                da.pos.y = nextY;

                if (da.velocity.x != 0.0)
                {
                    da.rotation += (da.velocity.x > 0.0 ? 5.0 : -5.0) * dt * timeScale;
                }
                else
                {
                    da.rotation -= (std::cos(da.rotation) >= 0) ? (2.0 * dt * timeScale) : (-2.0 * dt * timeScale);
                }
            }
        }

        Math::rect pRect = player.GetRect();
        for (auto it = droppedArrows.begin(); it != droppedArrows.end();)
        {
            Math::rect dropBox = { it->pos - Math::vec2{ 25.0, 25.0 }, it->pos + Math::vec2{ 25.0, 25.0 } };
            if (Math::IntersectsRect(pRect, dropBox))
            {
                int maxCap = 5 + player.combat.arrowMaxAmmoBonus;
                if (maxCap > MAX_AMMO) maxCap = MAX_AMMO;

                if (currentAmmo < maxCap && currentAmmo < MAX_AMMO)
                {
                    currentAmmo++;
                    Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/collect.wav");
                }
                it = droppedArrows.erase(it);
            }
            else ++it;
        }
    }

    Math::rect ArrowWeapon::GetAttackBox(const Player& player) const
    {
        if (meleeMode)
        {
            const double reach = kPlayerWidth * 1.0 * (1.0 + player.combat.arrowMeleeRangeBonus);
            const double atkH = player.GetRect().Size().y;
            if (player.IsFacingRight()) return { { player.GetRect().Right(), player.GetRect().Bottom() }, { player.GetRect().Right() + reach, player.GetRect().Bottom() + atkH } };
            else return { { player.GetRect().Left() - reach, player.GetRect().Bottom() }, { player.GetRect().Left(), player.GetRect().Bottom() + atkH } };
        }
        return { { 0, 0 }, { 0, 0 } };
    }

    void ArrowWeapon::EndAttack(Player& player)
    {
        (void)player;
        meleeMode = false;
        meleeTimer = 0.0;
        shootTimer = 0.0;
    }

    std::vector<HitResult> ArrowWeapon::ResolveHits(Player& player)
    {
        std::vector<HitResult> results;

        if (meleeMode)
        {
            auto meleeHits = CollectHits(player, GetAttackBox(player), false);
            for (auto& hit : meleeHits) hit.damageMult = meleeEmpty ? 0.5 : 1.0;
            results.insert(results.end(), meleeHits.begin(), meleeHits.end());
        }

        for (int i = 0; i < MAX_AMMO; ++i)
        {
            auto& b = bullets[i];
            if (!b.active) continue;

            Math::vec2 bcen = { b.rect.Left() + b.rect.Size().x * 0.5, b.rect.Bottom() + b.rect.Size().y * 0.5 };
            Math::vec2 tipCenter = {
                bcen.x + std::cos(b.rotation) * (b.rect.Size().x * 0.45),
                bcen.y + std::sin(b.rotation) * (b.rect.Size().x * 0.45)
            };
            double tipRadius = b.rect.Size().y * 0.8;

            bool hitOrb = false;
            for (auto* e : Enemy::Registry())
            {
                if (auto* fb = dynamic_cast<CS230::ForestBoss*>(e))
                {
                    for (int o = 0; o < 5; ++o)
                    {
                        if (fb->orbs[o].active && !fb->orbs[o].exploding &&
                            CS230::CollisionHelpers::IntersectsCircleAABB(tipCenter, tipRadius, fb->orbs[o].rect))
                        {
                            b.active = false; 

                            DroppedArrow da;
                            da.pos = bcen;
                            da.isStuckToWall = false;
                            da.velocity.x = (b.velocity.x > 0.0) ? -200.0 : 200.0;
                            da.velocity.y = 150.0;
                            droppedArrows.push_back(da);

                            fb->orbs[o].grounded = false;
                            fb->orbs[o].velocity.x = (b.velocity.x > 0.0) ? 1200.0 : -1200.0;
                            fb->orbs[o].velocity.y = 400.0;

                            hitOrb = true;
                            break; 
                        }
                    }
                }
                if (hitOrb) break;
            }

            if (hitOrb) continue;


            for (auto* e : Enemy::Registry())
            {
                if (!e || !e->IsActive() || e->GetHealth() <= 0) continue;
                if (e->playerHitCooldownSec > 0.0) continue;

                Math::rect hitRect = e->GetRect();
                bool isShieldBlock = false;

                if (ShieldEnemy* se = dynamic_cast<ShieldEnemy*>(e))
                {
                    bool arrowFromFront = ((b.velocity.x > 0.0 && se->moveDir < 0) || (b.velocity.x < 0.0 && se->moveDir > 0));
                    if (arrowFromFront && !se->IsGroggy())
                    {
                        isShieldBlock = true;
                        if (se->moveDir < 0) hitRect.point_1.x -= 45.0;
                    }
                }

                if (CS230::CollisionHelpers::IntersectsCircleAABB(tipCenter, tipRadius, hitRect))
                {
                    if (!IsBlockedByPlatform(b.spawnCenter, e->Center()))
                    {
                        bool isBlocked = false;
                        if (ShieldEnemy* se = dynamic_cast<ShieldEnemy*>(e))
                        {
                            isBlocked = isShieldBlock;
                        }
                        else
                        {
                            isBlocked = e->CanIgnorePlayerAttack(player);
                        }

                        if (isBlocked)
                        {
                            b.active = false;

                            DroppedArrow da;
                            da.pos = bcen;

                            if (ShieldEnemy* se = dynamic_cast<ShieldEnemy*>(e)) {
                                if (se->moveDir < 0) da.pos.x -= 30.0;
                                else da.pos.x += 15.0;
                            }

                            da.isStuckToWall = false;
                            da.velocity.x = e->moveDir * 100.0;
                            da.velocity.y = 150.0;
                            droppedArrows.push_back(da);

                            results.push_back({ e, false, true, 0.0 });
                        }
                        else
                        {
                            b.active = false;
                            double enemyEmbedDepth = 20.0;
                            Math::vec2 embeddedCenter = bcen + Math::vec2{ std::cos(b.rotation) * enemyEmbedDepth, std::sin(b.rotation) * enemyEmbedDepth };
                            Math::vec2 hitOffset = embeddedCenter - e->Center();

                            StuckArrow sa;
                            sa.target = e;
                            sa.offset = hitOffset;
                            sa.rotation = b.rotation;
                            sa.hitEnemyDir = static_cast<int>(e->moveDir);
                            stuckArrows.push_back(sa);

                            results.push_back({ e, false, true, 1.0 });
                        }
                        break;
                    }
                }
            }
        }
        return results;
    }

    void ArrowWeapon::Draw(Math::TransformationMatrix camM)
    {
        for (int i = 0; i < MAX_AMMO; ++i)
        {
            if (bullets[i].active) bullets[i].Draw(camM);
        }

        auto spr = bullets[0].GetGOComponent<CS230::Sprite>();
        if (!spr) return;

        Math::vec2 targetSize{ 60.0, 15.0 };
        Math::ivec2 fs = spr->GetFrameSize();
        if (fs.x <= 0 || fs.y <= 0) return;

        Math::vec2 scale{ targetSize.x / static_cast<double>(fs.x), targetSize.y / static_cast<double>(fs.y) };

        for (const auto& da : droppedArrows)
        {
            Math::TransformationMatrix m = camM * Math::TranslationMatrix(da.pos) * Math::RotationMatrix(da.rotation) * Math::TranslationMatrix(Math::vec2{ -30.0, -7.5 }) * Math::ScaleMatrix(scale);
            spr->Draw(m);
        }

        for (const auto& sa : stuckArrows)
        {
            bool targetExists = false;
            for (auto* e : Enemy::Registry())
            {
                if (e == sa.target) { targetExists = true; break; }
            }

            if (targetExists && sa.target->IsActive())
            {
                double currentOffsetX = (sa.target->moveDir == sa.hitEnemyDir) ? sa.offset.x : -sa.offset.x;
                Math::vec2 pos = sa.target->Center() + Math::vec2{ currentOffsetX, sa.offset.y };
                double currentRotation = (sa.target->moveDir == sa.hitEnemyDir) ? sa.rotation : (3.14159265358979323846 - sa.rotation);

                Math::TransformationMatrix m = camM * Math::TranslationMatrix(pos) * Math::RotationMatrix(currentRotation) * Math::TranslationMatrix(Math::vec2{ -30.0, -7.5 }) * Math::ScaleMatrix(scale);
                spr->Draw(m);
            }
        }
    }

    std::unique_ptr<Weapon> MakeWeapon(WeaponType t)
    {
        switch (t)
        {
        case WeaponType::Sword:
            return std::make_unique<SwordWeapon>();
        case WeaponType::Hammer:
            return std::make_unique<HammerWeapon>();
        case WeaponType::Arrow:
            return std::make_unique<ArrowWeapon>();
        }
        return nullptr;
    }
}