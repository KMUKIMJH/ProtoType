#include "RangedEnemy.h"
#include "Player.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../Engine/ShowCollision.h"
#include "../OpenGL/RGBA.h"
#include "../Engine/Sprite.h" 
#include "../Stage/StageManager.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr double kBulletSpeed = 720.0;
    constexpr double kKnockbackStrength = 30.0;
    constexpr double kBulletWidth = 120.0;
    constexpr double kBulletHeight = 20.0;
}

static std::vector<CS230::RangedEnemy*> g_ranged_registry;
std::vector<CS230::RangedEnemy*>& CS230::RangedEnemy::Registry() { return g_ranged_registry; }
void CS230::RangedEnemy::ClearAllBullets() { for (auto* re : Registry()) if (re) for (auto& b : re->bullets) b.Reset(); }

void CS230::RangedEnemy::State_Patrol::Enter(Enemy* e)
{
    auto* r = static_cast<RangedEnemy*>(e);
    if (auto spr = r->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);
    r->fleeDashRemaining = 0.0; r->fleeDashDir = 0;
    r->hasRetreatedThisCycle = false; r->telegraphActive = false;
}

void CS230::RangedEnemy::State_Patrol::Update(Enemy* e, const CS230::Player&, double dt, bool isSlow)
{
    auto* r = static_cast<RangedEnemy*>(e);
    double dx = r->baseSpeed * (isSlow ? Enemy::SlowTimeScale : 1.0) * r->moveDir * dt;
    dx = r->ClampToPatrol(dx); dx = r->ApplyPlatformEdge(dx);
    r->rect.point_1.x += dx; r->rect.point_2.x += dx;
}

void CS230::RangedEnemy::State_Patrol::CheckExit(Enemy* e, const CS230::Player& player)
{
    if (!player.IsAlive()) return;
    auto* r = static_cast<RangedEnemy*>(e);
    if (r->aggroLockTimer > 0.0 || (std::abs(player.Center().x - r->Center().x) <= RangedEnemy::RecognizeRangeX && !IsBlockedByPlatform(r->Center(), player.Center()))) {
        r->aggroLockTimer = 2.0; r->ChangeState(&r->state_shooter);
    }
}

void CS230::RangedEnemy::State_EngageShooter::Enter(Enemy* e)
{
    auto* r = static_cast<RangedEnemy*>(e);
    if (auto spr = r->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);
    r->hasRetreatedThisCycle = false;
}

void CS230::RangedEnemy::State_EngageShooter::Update(Enemy* e, const CS230::Player& player, double dt, bool isSlow)
{
    auto* r = static_cast<RangedEnemy*>(e);
    if (!player.IsAlive()) { r->ChangeState(&r->state_patrol); return; }

    double ts = isSlow ? Enemy::SlowTimeScale : 1.0;

    if (r->fleeDashRemaining > 0.0 && r->fleeDashDir != 0) {
        double dx = (r->baseSpeed * 3.0) * ts * r->fleeDashDir * dt;
        if (std::abs(dx) > r->fleeDashRemaining) dx = std::copysign(r->fleeDashRemaining, dx);
        dx = r->ApplyPlatformEdge(dx);

        r->rect.point_1.x += dx; r->rect.point_2.x += dx;
        r->fleeDashRemaining -= std::abs(dx);
        if (std::abs(dx) < 1.0 || r->fleeDashRemaining <= 0.0) { r->fleeDashRemaining = 0.0; r->fleeDashDir = 0; }
        return;
    }

    if (!r->hasRetreatedThisCycle) {
        r->fleeDashDir = (player.Center().x >= r->Center().x) ? -1 : 1;
        r->fleeDashRemaining = RangedEnemy::RetreatDistance;
        r->hasRetreatedThisCycle = true; return;
    }

    bool isBulletFlying = false;
    for (const auto& b : r->bullets) {
        if (b.active) { isBulletFlying = true; break; }
    }

    if (r->cooldownTimer > 0.0) {
        r->cooldownTimer -= dt * ts;
        if (r->cooldownTimer <= 0.0) {
            if (auto spr = r->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(RangedEnemy::Animations::Idle));
        }
    }

    if (!isBulletFlying && r->cooldownTimer <= 0.0 && !r->telegraphActive) {
        r->telegraphActive = true;
        r->telegraphTimer = RangedEnemy::TelegraphSec;
        if (auto spr = r->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(RangedEnemy::Animations::Idle));
    }

    if (r->telegraphActive) {
        r->moveDir = (player.Center().x >= r->Center().x) ? 1 : -1;
        r->telegraphTimer -= dt * ts;
        if (r->telegraphTimer <= 0.0) {
            r->telegraphActive = false;
            r->cooldownTimer = 3.0;

            if (auto spr = r->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(1);

            for (auto& b : r->bullets) {
                if (!b.active) {
                    b.Reset(); b.active = true; b.owner = CS230::Bullet::Owner::Enemy;
                    double cx = r->Center().x;

                    double cy = r->rect.Bottom() + r->Height() * 0.6;

                    double bcenX = cx - (r->moveDir * kBulletWidth * 0.45);

                    b.rect = { { bcenX - kBulletWidth * 0.5, cy - kBulletHeight * 0.5 }, { bcenX + kBulletWidth * 0.5, cy + kBulletHeight * 0.5 } };
                    b.spawnCenter = { cx, cy };

                    b.velocity = Math::vec2{ (double)r->moveDir, 0.0 } * kBulletSpeed;
                    b.velocity.y = 150.0;
                    b.gravity = 450.0;

                    b.rotation = (r->moveDir < 0) ? 3.14159265358979323846 : 0.0;

                    b.SetPosition({ bcenX, cy });
                    break;
                }
            }
        }
    }
}

void CS230::RangedEnemy::State_EngageShooter::CheckExit(Enemy* e, const CS230::Player& player)
{
    auto* r = static_cast<RangedEnemy*>(e);

    bool recognized = (std::abs(player.Center().x - r->Center().x) <= RangedEnemy::RecognizeRangeX && !IsBlockedByPlatform(r->Center(), player.Center()));
    if (recognized) r->aggroLockTimer = 2.0;

    if (!player.IsAlive() || (!recognized && r->aggroLockTimer <= 0.0)) {
        r->hasRetreatedThisCycle = false; r->fleeDashRemaining = 0.0; r->ChangeState(&r->state_patrol);
    }
}

void CS230::RangedEnemy::Load(const Math::vec2& pos, int initialHealth)
{
    rect = { pos, pos + Math::vec2{ Enemy::DefaultWidth, Enemy::DefaultHeight } };
    health = (initialHealth > 0) ? initialHealth : DefaultHealth; maxHealth = health;
    hitCooldownSec = 0.0; cooldownTimer = 0.0; active = true; hasRetreatedThisCycle = false;

    for (auto& b : bullets) { b.Reset(); b.rect = { { 0,0 }, { kBulletWidth, kBulletHeight } }; b.InitSprite("Assets/image/Objects/Arrow.spt", { kBulletWidth / 512.0, kBulletHeight / 256.0 }); }
    SetPosition({ rect.Left(), rect.Bottom() });
    AddGOComponent(new CS230::Sprite("Assets/image/Ranged/Ranged.spt", this));
    CS230::RangedEnemy::Registry().push_back(this); RegisterEnemy(); ChangeState(&state_patrol);
}

void CS230::RangedEnemy::Update(const CS230::Player& player, double dt, bool isSlow)
{
    auto& p = const_cast<CS230::Player&>(player);
    if (p.IsSlowMoActive()) isSlow = true;

    if (health <= 0 && active) {
        active = false;
        for (auto& b : bullets) b.Reset();
        ClearGOComponents();
        SpawnDeathReward();
    }
    if (active) {
        if (player.IsAttackActive() && Math::IntersectsRect(player.AttackBox(), GetRect()) && playerHitCooldownSec <= 0.0) {
            health -= 1; playerHitCooldownSec = 0.5; NotifyHit(&player);
            ApplyStunAndKnockbackFromPlayer(player, 0.2, kKnockbackStrength);
        }
        UpdateState(player, dt, isSlow);
    }
    if (playerHitCooldownSec > 0.0) { playerHitCooldownSec -= dt; if (playerHitCooldownSec < 0.0) playerHitCooldownSec = 0.0; }
    if (aggroLockTimer > 0.0) 
    {
        aggroLockTimer -= dt * (isSlow ? Enemy::SlowTimeScale : 1.0);
        if (aggroLockTimer < 0.0) aggroLockTimer = 0.0;
    }
    ApplyGravity(dt);
    UpdateGroggyTimer(dt * (isSlow ? Enemy::SlowTimeScale : 1.0));
    UpdateGOComponents(dt);

    if (auto spr = GetGOComponent<CS230::Sprite>()) {
        Math::vec2 newScale{ 1.0, 1.0 };
        if (spr->GetFrameSize().x > 0) newScale.x = rect.Size().x / spr->GetFrameSize().x;
        if (spr->GetFrameSize().y > 0) newScale.y = rect.Size().y / spr->GetFrameSize().y;
        newScale.x = (moveDir < 0) ? -std::abs(newScale.x) : std::abs(newScale.x); SetScale(newScale);
    }

    for (auto& b : bullets) {
        if (!b.active) continue;
        b.Update(dt * (isSlow ? Enemy::SlowTimeScale : 1.0));

        bool hitPlatform = false;
        auto* sm = dynamic_cast<StageManager*>(Engine::GetGameStateManager().GetCurrentState());

        Math::vec2 bcen = { b.rect.Left() + b.rect.Size().x * 0.5, b.rect.Bottom() + b.rect.Size().y * 0.5 };
        Math::vec2 tipCenter = {
            bcen.x + std::cos(b.rotation) * (b.rect.Size().x * 0.5),
            bcen.y + std::sin(b.rotation) * (b.rect.Size().x * 0.5)
        };
        Math::rect tipRect = { tipCenter - Math::vec2{2.0, 2.0}, tipCenter + Math::vec2{2.0, 2.0} };

        if (sm)
        {
            for (auto* obj : sm->staticObjects)
            {
                if (obj->Type() == GameObjectTypes::Platform)
                {
                    auto* rc = obj->GetGOComponent<CS230::RectCollision>();
                    if (rc && Math::IntersectsRect(tipRect, rc->WorldBoundary()))
                    {
                        hitPlatform = true;
                        break;
                    }
                }
            }
        }

        if (hitPlatform)
        {
            b.active = false;
            continue;
        }

        Math::vec2 hitTipCenter = {
            bcen.x + std::cos(b.rotation) * (b.rect.Size().x * 0.45),
            bcen.y + std::sin(b.rotation) * (b.rect.Size().x * 0.45)
        };
        double tipRadius = b.rect.Size().y * 0.8;

        if (b.owner == CS230::Bullet::Owner::Enemy && CS230::CollisionHelpers::IntersectsCircleAABB(hitTipCenter, tipRadius, player.GetRect())) {
            auto* pNC = const_cast<CS230::Player*>(&player);
            if (pNC->IsParrying() && !pNC->IsRolling() && !pNC->IsInvincible()) {
                pNC->AddTimeGauge(pNC->gaugeGainParry);
                pNC->NotifyInvincibleHit();
                pNC->EndParry();

                if (pNC->IsSlowMoActive()) {
                    b.owner = CS230::Bullet::Owner::Player; 
                    b.reflected = true;
                    b.velocity.x *= -1.5; 
                    b.rotation += 3.14159265358979323846; 
                }
                else {
                    b.active = false;
                }
            }
            else if (pNC->IsRolling()) 
            {
                if (!pNC->move.dodgeRewardClaimed) {
                    pNC->AddTimeGauge(pNC->gaugeGainRoll);
                    pNC->move.dodgeRewardClaimed = true;
                }
            }
            else if (!pNC->IsRolling() && !pNC->IsInvincible() && pNC->CanTakeHit()) {
                pNC->TakeHit();
                pNC->ApplyKnockback(kKnockbackStrength, (pNC->Center().x >= Center().x) ? 1 : -1);
                b.active = false;
            }
        }
        else if (b.owner == CS230::Bullet::Owner::Player) 
        {
            bool hitEnemy = false;
            for (auto* e : CS230::Enemy::Registry()) {
                if (!e || !e->IsActive() || e->GetHealth() <= 0) continue;
                if (e->playerHitCooldownSec > 0.0) continue;

                if (CS230::CollisionHelpers::IntersectsCircleAABB(hitTipCenter, tipRadius, e->GetRect())) {
                    if (e->CanIgnorePlayerAttack(player)) 
                    {
                        e->playerHitCooldownSec = 0.5;
                        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/block.wav");
                    }
                    else {
                        int hp = e->GetHealth();
                        e->SetHealth(hp - 1);
                        e->NotifyHit(&player);
                        e->playerHitCooldownSec = 0.2;
                        e->ApplyStunAndKnockbackFromPlayer(player, 0.2, kKnockbackStrength);
                    }
                    b.active = false;
                    hitEnemy = true;
                    break;
                }
            }
            if (hitEnemy) continue;
        }

        if (std::abs(b.rect.Left() - b.spawnCenter.x) > 3000.0 || b.rect.Bottom() < -1000.0)
        {
            b.active = false;
        }
    }
    SetPosition({ rect.Left(), rect.Bottom() });
}

void CS230::RangedEnemy::Draw(Math::TransformationMatrix camM)
{
    for (auto& b : bullets) if (b.active) b.Draw(camM);
    if (!active) return;
    if (auto spr = GetGOComponent<CS230::Sprite>())
    {
        if (hitCooldownSec > 0.0) {
            spr->Draw(camM * GetMatrix(), GAME200::RED);
        }
        else {
            CS230::GameObject::Draw(camM);
        }
    }
    else
    {
        unsigned int color = GAME200::CYAN;
        if (hitCooldownSec > 0.0) color = GAME200::RED;
        else if (telegraphActive) color = 0xFFFF00FF;
        Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(Center()) * Math::ScaleMatrix(rect.Size()), color, GAME200::BLACK, 1.0);
    }
    if (telegraphActive) DrawTelegraph(camM);
    DrawStatus(camM);
}

void CS230::RangedEnemy::ForceAggro(const Player& p)
{
    if (Enemy::current_state == &state_patrol) { moveDir = (Center().x > p.Center().x) ? -1 : 1; ChangeState(&state_shooter); }
}
