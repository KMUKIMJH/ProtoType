#include "ShieldEnemy.h"
#include "Player.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../Engine/ShowCollision.h"
#include "../OpenGL/RGBA.h"
#include <cmath>

bool CS230::ShieldEnemy::PlayerInRecognition(const Player& player) const
{
    const Math::vec2 delta = CS230::Delta(*this, player);
    if (std::abs(delta.y) > Height() * 1.5) return false;

    if (aggroLockTimer <= 0.0) {
        bool inX = false;
        if (moveDir > 0) inX = (delta.x >= 0.0 && delta.x <= recognizeRangeX);
        else inX = (delta.x <= 0.0 && std::abs(delta.x) <= recognizeRangeX);
        if (!inX) return false;
    }
    else {
        if (std::abs(delta.x) > recognizeRangeX) return false;
    }
    if (IsBlockedByPlatform(Center(), player.Center())) return false;
    return true;
}

void CS230::ShieldEnemy::ForceAggro(const Player& p)
{
    if (Enemy::current_state == &state_patrol) {
        Math::vec2 delta = Center() - p.Center();
        moveDir = (delta.x > 0.0) ? -1 : 1;
        ChangeState(&state_dash);
    }
}

void CS230::ShieldEnemy::State_Patrol::Enter(Enemy* e)
{
    auto* enemy = static_cast<ShieldEnemy*>(e);
    if (auto sprite = enemy->GetGOComponent<CS230::Sprite>()) sprite->PlayAnimation(1);
    enemy->dashing = false; enemy->dashActive = false; enemy->dashHasHit = false;
    enemy->telegraphActive = false; enemy->telegraphTimer = 0.0;

    if (!enemy->patrolSet) {
        enemy->patrolLeft = enemy->rect.Left() - ShieldEnemy::PatrolExtentX;
        enemy->patrolRight = enemy->rect.Right() + ShieldEnemy::PatrolExtentX;
        enemy->patrolSet = true;
    }
}

void CS230::ShieldEnemy::State_Patrol::Update(Enemy* e, const CS230::Player& player, double dt, bool isSlow)
{
    auto* enemy = static_cast<ShieldEnemy*>(e);
    double timeScale = isSlow ? Enemy::SlowTimeScale : 1.0;
    if (!player.IsAlive()) return;

    const Math::vec2 delta = CS230::Delta(*enemy, player);

    bool trackPlayer = (std::abs(delta.x) <= 600.0 && std::abs(delta.y) <= enemy->Height() * 1.5);
    if (trackPlayer) {
        enemy->moveDir = (delta.x > 0) ? 1 : -1;
    }

    bool facingPlayer = ((delta.x >= 0.0 && enemy->moveDir > 0) || (delta.x < 0.0 && enemy->moveDir < 0));
    if (facingPlayer && enemy->PlayerInRecognition(player)) 
    {
        enemy->aggroLockTimer = 2.0; 
        enemy->ChangeState(&enemy->state_dash);
        return;
    }

    double dxMove = ShieldEnemy::BaseSpeed * timeScale * enemy->moveDir * dt;

    if (!trackPlayer) {
        dxMove = enemy->ClampToPatrol(dxMove);
    }

    dxMove = enemy->ApplyPlatformEdge(dxMove);

    if (trackPlayer) {
        enemy->moveDir = (delta.x > 0) ? 1 : -1;
    }

    enemy->rect.point_1.x += dxMove; enemy->rect.point_2.x += dxMove;
}

void CS230::ShieldEnemy::State_Patrol::CheckExit(Enemy* e, const CS230::Player& player)
{
    auto* enemy = static_cast<ShieldEnemy*>(e);
    const Math::vec2 delta = CS230::Delta(*enemy, player);
    bool facingPlayer = ((delta.x >= 0.0 && enemy->moveDir > 0) || (delta.x < 0.0 && enemy->moveDir < 0));
    if (facingPlayer && enemy->PlayerInRecognition(player)) {
        enemy->aggroLockTimer = 2.0; enemy->ChangeState(&enemy->state_dash);
    }
}

void CS230::ShieldEnemy::State_EngageDash::Enter(Enemy* e)
{
    auto* enemy = static_cast<ShieldEnemy*>(e);
    enemy->dashing = true; enemy->dashActive = false; enemy->dashHasHit = false;
    enemy->dashTimer = 0.0; enemy->dashCooldown = 0.0; enemy->dashHitBox = { {0,0}, {0,0} };
    enemy->telegraphActive = true; enemy->telegraphTimer = ShieldEnemy::TelegraphSec;
    if (auto sprite = enemy->GetGOComponent<CS230::Sprite>()) sprite->PlayAnimation(0);
}

void CS230::ShieldEnemy::State_EngageDash::Update(Enemy* e, const CS230::Player& player, double dt, bool isSlow)
{
    auto* enemy = static_cast<ShieldEnemy*>(e);
    double timeScale = isSlow ? Enemy::SlowTimeScale : 1.0;
    if (!player.IsAlive()) { enemy->ChangeState(&enemy->state_patrol); return; }

    if (enemy->dashCooldown > 0.0) {
        enemy->dashCooldown -= dt * timeScale;
        if (enemy->dashCooldown <= 0.0) {
            enemy->dashCooldown = 0.0; enemy->dashActive = false;
            enemy->dashing = false; enemy->dashHasHit = false; enemy->telegraphActive = false;
        }
        return;
    }

    if (!enemy->dashActive) {
        if (!enemy->telegraphActive) {
            enemy->telegraphActive = true; enemy->telegraphTimer = ShieldEnemy::TelegraphSec;
            enemy->moveDir = (CS230::Delta(*enemy, player).x >= 0.0) ? 1 : -1;
            enemy->dashStartDir = enemy->moveDir;
            if (auto spr = enemy->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(0);
        }
        enemy->telegraphTimer -= dt; enemy->allowLeavePlatform = false;

        if (enemy->telegraphTimer <= 0.0) {
            enemy->telegraphActive = false;
            enemy->dashTimer = (ShieldEnemy::RecognizeRangeX * enemy->dashDistanceMul) / (ShieldEnemy::BaseSpeed * enemy->dashSpeedMul);
            enemy->dashActive = true; enemy->dashing = true; enemy->dashHasHit = false;
            enemy->dashStartDir = enemy->moveDir; enemy->dashHitBox = enemy->rect;
        }
    }

    if (enemy->dashActive) {
        double dx = ShieldEnemy::BaseSpeed * enemy->dashSpeedMul * dt * timeScale * enemy->moveDir;

        dx = enemy->ApplyPlatformEdge(dx);

        enemy->rect.point_1.x += dx; enemy->rect.point_2.x += dx;

        bool hitWall = (enemy->moveDir != enemy->dashStartDir);
        if (hitWall) {
            enemy->moveDir = enemy->dashStartDir;
            enemy->EnterGroggyState(ShieldEnemy::GroggyDuration);
            enemy->CancelDash();
            return;
        }

        enemy->dashHitBox = enemy->rect;
        enemy->HandleDashCollision(const_cast<Player&>(player));
        enemy->dashTimer -= dt * timeScale;

        if (enemy->dashTimer <= 0.0) {
            enemy->dashActive = false; enemy->dashing = false; enemy->dashHasHit = false;
            enemy->dashHitBox = { {0,0}, {0,0} }; enemy->dashCooldown = ShieldEnemy::DashCooldownSec;
            enemy->EnterGroggyState(ShieldEnemy::GroggyDuration);
        }
    }
    else {
        enemy->dashHitBox = { {0,0}, {0,0} };
    }
}

void CS230::ShieldEnemy::State_EngageDash::CheckExit(Enemy* e, const CS230::Player& player)
{
    auto* enemy = static_cast<ShieldEnemy*>(e);
    if (!player.IsAlive()) { enemy->ChangeState(&enemy->state_patrol); return; }
    bool recognized = enemy->PlayerInRecognition(player);
    if (recognized) enemy->aggroLockTimer = 2.0;
    if (!enemy->dashActive && !enemy->telegraphActive && enemy->dashCooldown <= 0.0 && !enemy->IsGroggy()) {
        if (!recognized && enemy->aggroLockTimer <= 0.0) enemy->ChangeState(&enemy->state_patrol);
    }
}

void CS230::ShieldEnemy::OnSuccessfulHitPlayer() { EnterGroggyState(GroggyDuration); }

void CS230::ShieldEnemy::Load(const Math::vec2& position_, int)
{
    rect = { position_, position_ + Math::vec2{ Enemy::DefaultWidth, Enemy::DefaultHeight } };
    health = ShieldEnemy::DefaultHealth; maxHealth = ShieldEnemy::DefaultHealth;
    active = true; dashing = false; dashActive = false; dashHasHit = false;
    dashTimer = 0.0; dashCooldown = 0.0; recognizeRangeX = ShieldEnemy::RecognizeRangeX;
    attackPauseTimer = 0.0;
    SetPosition({ rect.Left(), rect.Bottom() });
    AddGOComponent(new CS230::Sprite("Assets/image/Shield/Shield.spt", this));
    if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(ShieldEnemy::Animations::Move));
    ChangeState(&state_patrol);
}

void CS230::ShieldEnemy::Update(const CS230::Player& player, double dt, bool isSlow)
{
    if (player.IsSlowMoActive()) isSlow = true;

    if (health <= 0) {
        if (active) {
            active = false; dashing = false; dashActive = false; dashHasHit = false;
            ClearGOComponents(); SpawnDeathReward();
        }
    }
    if (hitCooldownSec > 0.0) { hitCooldownSec -= dt; if (hitCooldownSec < 0.0) hitCooldownSec = 0.0; }
    if (attackPauseTimer > 0.0) { attackPauseTimer -= dt; if (attackPauseTimer < 0.0) attackPauseTimer = 0.0; }

    if (active) {
        bool wasGroggy = IsGroggy();
        if (!wasGroggy) UpdateState(player, dt, isSlow);

        double timeScale = isSlow ? Enemy::SlowTimeScale : 1.0;
        UpdateGroggyTimer(dt * timeScale);
        ApplyGravity(dt * timeScale);

        if (wasGroggy && !IsGroggy()) ChangeState(&state_patrol);
    }
    if (playerHitCooldownSec > 0.0) { playerHitCooldownSec -= dt; if (playerHitCooldownSec < 0.0) playerHitCooldownSec = 0.0; }

    if (aggroLockTimer > 0.0) 
    {
        aggroLockTimer -= dt * (isSlow ? Enemy::SlowTimeScale : 1.0);
        if (aggroLockTimer < 0.0) aggroLockTimer = 0.0;
    }

    SetPosition({ rect.Left(), rect.Bottom() });

    UpdateGOComponents(dt);
    if (auto spr = GetGOComponent<CS230::Sprite>()) {
        Math::vec2 newScale{ 1.0, 1.0 };
        Math::ivec2 fs = spr->GetFrameSize();
        if (fs.x > 0) newScale.x = rect.Size().x / static_cast<double>(fs.x);
        if (fs.y > 0) newScale.y = rect.Size().y / static_cast<double>(fs.y);
        newScale.x = (moveDir < 0) ? -std::abs(newScale.x) : std::abs(newScale.x);
        SetScale(newScale);
    }
}

void CS230::ShieldEnemy::Draw(Math::TransformationMatrix camM)
{
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
        CS230::GameObject::Draw(camM);
    }

    if (telegraphActive) DrawTelegraph(camM);

    DrawStatus(camM);
}

void CS230::ShieldEnemy::CancelDash()
{
    dashing = false; dashActive = false; dashHasHit = false;
    dashTimer = 0.0; dashCooldown = DashCooldownSec; dashHitBox = { {0,0}, {0,0} };
}

bool CS230::ShieldEnemy::CanIgnorePlayerAttack(const Player& player) const
{
    bool enemyFacingPlayer = ((player.Center().x >= Center().x && moveDir > 0) || (player.Center().x < Center().x && moveDir < 0));
    return enemyFacingPlayer && !IsGroggy();
}

void CS230::ShieldEnemy::HandleDashCollision(Player& player)
{
    if (!dashing || !dashActive || !Math::IntersectsRect(GetAttackBox(), player.GetRect())) return;

    if (player.IsParrying() && player.GetGroggyTimer() <= 0.0) {
        ApplyParryDebuff(2.0); player.NotifyInvincibleHit(); player.AddTimeGauge(player.gaugeGainParry);
        player.EndParry(); return;
    }

    if (player.IsRolling()) {
        if (!dashHasHit) { player.AddTimeGauge(player.gaugeGainRoll); dashHasHit = true; } return;
    }

    if (!dashHasHit) {
        if (player.CanTakeHit()) {
            player.TakeHit();
            player.SetGroggyTimer(1.0);
        }
        dashHasHit = true; EnterGroggyState(ShieldEnemy::GroggyDuration);
    }
}

void CS230::ShieldEnemy::Unload() { if (active) active = false; ClearGOComponents(); }
void CS230::ShieldEnemy::ApplyParryDebuff(double sec) { Enemy::ApplyParryDebuff(sec); CancelDash(); dashCooldown = 0.0; attackPauseTimer = attackPauseSec; groggyTimer = sec; }
void CS230::ShieldEnemy::EnterGroggyState(double sec) { groggyTimer = std::max(groggyTimer, sec); CancelDash(); telegraphActive = false; attackPauseTimer = attackPauseSec; Enemy::velocity.x = 0.0; }
