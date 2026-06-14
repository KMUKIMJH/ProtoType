#include "MeleeEnemy.h"
#include "Player.h"
#include "Gravity.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../OpenGL/RGBA.h"
#include <string>
#include "../Engine/ShowCollision.h"

bool CS230::MeleeEnemy::PlayerInRecognition(const Player& player) const
{
    const Math::vec2 delta = CS230::Delta(*this, player);
    if (std::abs(delta.y) > Height() * 1.5) return false;

    if (aggroLockTimer <= 0.0) {
        bool facingPlayer = ((delta.x >= 0.0 && moveDir > 0) || (delta.x < 0.0 && moveDir < 0));
        if (!facingPlayer) return false;
    }

    if (std::abs(delta.x) > MeleeEnemy::ChaseRangeX) return false;
    if (IsBlockedByPlatform(Center(), player.Center())) return false;
    return true;
}

void CS230::MeleeEnemy::ForceAggro(const Player& p)
{
    if (Enemy::current_state == &state_patrol) {
        Math::vec2 delta = Center() - p.Center();
        moveDir = (delta.x > 0.0) ? -1 : 1;
        ChangeState(&state_melee);
    }
}

void CS230::MeleeEnemy::State_Patrol::Enter(Enemy* e)
{
    auto* enemy = static_cast<MeleeEnemy*>(e);
    enemy->Enemy::velocity.x = 0.0;
    enemy->allowLeavePlatform = false;
    if (auto spr = enemy->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(MeleeEnemy::Animations::Patrol));
    enemy->CancelAttack();
    enemy->CancelTelegraphAndDebuff();
    enemy->waitingAfterHit = false;
    enemy->SetPosition({ enemy->rect.Left(), enemy->rect.Bottom() });
}

void CS230::MeleeEnemy::State_Patrol::Update(Enemy* e, const CS230::Player&, double dt, bool isSlow)
{
    auto* enemy = static_cast<MeleeEnemy*>(e);
    double dx = enemy->baseSpeed * dt * (isSlow ? Enemy::SlowTimeScale : 1.0) * enemy->moveDir;
    dx = enemy->ClampToPatrol(dx);
    dx = enemy->ApplyPlatformEdge(dx);

    if (std::abs(dx) > 1e-6) {
        enemy->rect.point_1.x += dx; enemy->rect.point_2.x += dx;
        enemy->SetPosition({ enemy->rect.Left(), enemy->rect.Bottom() });
    }
}

void CS230::MeleeEnemy::State_Patrol::CheckExit(Enemy* e, const CS230::Player& player)
{
    if (!player.IsAlive()) return;
    auto* enemy = static_cast<MeleeEnemy*>(e);
    if (enemy->PlayerInRecognition(player)) {
        enemy->aggroLockTimer = 2.0;
        enemy->ChangeState(&enemy->state_melee);
    }
}

void CS230::MeleeEnemy::State_EngageMelee::Enter(Enemy* e)
{
    auto* enemy = static_cast<MeleeEnemy*>(e);
    if (auto spr = enemy->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(MeleeEnemy::Animations::Idle));

    enemy->CancelAttack();
    enemy->meleeAttackPauseTimer = 0.0;
    enemy->CancelTelegraphAndDebuff();
    enemy->attackHitOccurred = false;
    enemy->waitingAfterHit = false;
}

void CS230::MeleeEnemy::State_EngageMelee::Update(Enemy* e, const CS230::Player& player, double dt, bool isSlow)
{
    auto* enemy = static_cast<MeleeEnemy*>(e);
    if (!player.IsAlive()) { enemy->ChangeState(&enemy->state_patrol); return; }

    const bool playerInRecognize = enemy->PlayerInRecognition(player);
    if (playerInRecognize) enemy->aggroLockTimer = 2.0;

    bool isCommitted = enemy->jumpActive || enemy->meleeAttackActive;
    if (!playerInRecognize && !isCommitted && enemy->aggroLockTimer <= 0.0) 
    {
        if (enemy->telegraphActive) { enemy->CancelTelegraphAndDebuff(); enemy->jumpActive = false; return; }
        enemy->ChangeState(&enemy->state_patrol); return;
    }

    if (enemy->meleeAttackPauseTimer > 0.0)
    {
        enemy->meleeAttackPauseTimer -= dt * (isSlow ? Enemy::SlowTimeScale : 1.0);
        if (enemy->meleeAttackPauseTimer < 0.0) enemy->meleeAttackPauseTimer = 0.0;
        return;
    }

    if (!enemy->meleeAttackActive && enemy->waitingAfterHit) 
    {
        enemy->waitingAfterHit = false;
        enemy->meleeAttackPauseTimer = MeleeEnemy::AttackPauseSec;
        return;
    }

    if (enemy->jumpActive) 
    {
        enemy->leapTimer -= dt * (isSlow ? Enemy::SlowTimeScale : 1.0);
        if (enemy->leapTimer <= 0.0 || (enemy->Enemy::velocity.x == 0.0 && enemy->leapTimer < 0.3)) {
            enemy->jumpActive = false;
            enemy->Enemy::velocity = { 0.0, 0.0 };
            enemy->attackHitOccurred = false;
            enemy->moveDir = (player.Center().x >= enemy->Center().x) ? 1 : -1;
            enemy->StartAttack();
            enemy->waitingAfterHit = true;
        }
        return;
    }

    if (enemy->meleeAttackActive) 
    {
        Player& modPlayer = const_cast<Player&>(player);
        if (player.IsParrying() && modPlayer.GetGroggyTimer() <= 0.0) 
        {
            if (Math::IntersectsRect(enemy->GetAttackBox(), player.GetRect()) && !enemy->attackHitOccurred) 
            {
                enemy->meleeAttackActive = false; enemy->Enemy::velocity = { 0.0,0.0 };
                enemy->waitingAfterHit = false; enemy->meleeAttackPauseTimer = MeleeEnemy::AttackPauseSec;
                enemy->ApplyParryDebuff(2.0); enemy->OnParriedByPlayer(modPlayer); return;
            }
        }
        if (!enemy->attackHitOccurred && Math::IntersectsRect(enemy->GetAttackBox(), player.GetRect()) && !player.IsRolling() && modPlayer.CanTakeHit()) 
        {
            modPlayer.TakeHit();
            modPlayer.combat.health -= 1;
            if (modPlayer.combat.health < 0) modPlayer.combat.health = 0;

            enemy->OnSuccessfulHitPlayer();
            modPlayer.ApplyKnockback(120.0, (modPlayer.Center().x >= enemy->Center().x) ? 1 : -1);
        }
        return;
    }

    if (!enemy->telegraphActive)
    {
        enemy->telegraphActive = true;
        enemy->telegraphTimer = MeleeEnemy::TelegraphSec;
        enemy->attackHitOccurred = false;
        enemy->moveDir = (player.Center().x >= enemy->Center().x) ? 1 : -1;
        enemy->Enemy::velocity = { 0.0, 0.0 };

        if (auto spr = enemy->GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(MeleeEnemy::Animations::Idle));
        return;
    }

    enemy->telegraphTimer -= dt * (isSlow ? Enemy::SlowTimeScale : 1.0);
    if (enemy->telegraphTimer <= 0.0) 
    {
        enemy->telegraphActive = false;
        enemy->jumpActive = true;
        enemy->leapTimer = 0.35;

        double distX = 0.0;
        if (enemy->moveDir > 0) 
        {
            distX = player.GetRect().Left() - enemy->GetRect().Right();
        }
        else 
        {
            distX = enemy->GetRect().Left() - player.GetRect().Right();
        }

        double attackReach = enemy->GetRect().Size().x * 0.6;
        distX = std::max(0.0, distX - attackReach);

        double requiredSpeed = (distX / 0.35) * 2.5;

        if (requiredSpeed > enemy->leapHorizontalSpeed) 
        {
            requiredSpeed = enemy->leapHorizontalSpeed;
        }
        enemy->Enemy::velocity.x = enemy->moveDir * requiredSpeed;
    }
}

void CS230::MeleeEnemy::State_EngageMelee::CheckExit(Enemy* e, const CS230::Player& player)
{
    auto* enemy = static_cast<MeleeEnemy*>(e);
    if (!player.IsAlive()) { enemy->ChangeState(&enemy->state_patrol); return; }
    if (enemy->jumpActive || enemy->meleeAttackActive) return;
    if (!enemy->PlayerInRecognition(player) && enemy->aggroLockTimer <= 0.0) enemy->ChangeState(&enemy->state_patrol);
}

void CS230::MeleeEnemy::OnSuccessfulHitPlayer()
{
    meleeAttackTimer = 0.2;
    Enemy::velocity.x = 0.0;
    jumpActive = false; waitingAfterHit = true; attackHitOccurred = true;
}

void CS230::MeleeEnemy::StartAttack()
{
    meleeAttackActive = true;
    meleeAttackTimer = AttackDurationSec;
    if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(MeleeEnemy::Animations::Attack));
}
void CS230::MeleeEnemy::CancelTelegraphAndDebuff() { telegraphActive = false; telegraphTimer = 0.0; }

void CS230::MeleeEnemy::Load(const Math::vec2& position_, int initialHealth)
{
    rect = { position_, position_ + Math::vec2{ Enemy::DefaultWidth, Enemy::DefaultHeight } };
    health = (initialHealth > 0) ? initialHealth : 3;
    maxHealth = health; active = true; hitCooldownSec = 0.0; moveDir = 1; patrolSet = false;
    baseSpeed = 180.0;
    CancelAttack(); CancelTelegraphAndDebuff(); waitingAfterHit = false;

    SetPosition({ rect.Left(), rect.Bottom() });
    AddGOComponent(new CS230::Sprite("Assets/image/Melee/Melee.spt", this));
    ChangeState(&state_patrol);
}

void CS230::MeleeEnemy::Update(const CS230::Player& player, double dt, bool isSlow)
{
    if (player.IsSlowMoActive()) isSlow = true;

    if (health <= 0 && active) { active = false; CancelAttack(); ClearGOComponents(); SpawnDeathReward(); }
    if (hitCooldownSec > 0.0) { hitCooldownSec -= dt; if (hitCooldownSec < 0.0) hitCooldownSec = 0.0; }
    if (active) {
        ApplyGravity(dt * (isSlow ? Enemy::SlowTimeScale : 1.0));
        bool wasGroggy = IsGroggy();
        UpdateGroggyTimer(dt * (isSlow ? Enemy::SlowTimeScale : 1.0));
        if (!wasGroggy) UpdateState(player, dt, isSlow);
        if (wasGroggy && !IsGroggy()) ChangeState(&state_patrol);
    }
    if (playerHitCooldownSec > 0.0) { playerHitCooldownSec -= dt; if (playerHitCooldownSec < 0.0) playerHitCooldownSec = 0.0; }

    if (aggroLockTimer > 0.0) 
    {
        aggroLockTimer -= dt * (isSlow ? Enemy::SlowTimeScale : 1.0);
        if (aggroLockTimer < 0.0) aggroLockTimer = 0.0;
    }
    if (meleeAttackTimer > 0.0) {
        meleeAttackTimer -= dt * (isSlow ? Enemy::SlowTimeScale : 1.0);
        if (meleeAttackTimer <= 0.0) 
        {
            meleeAttackActive = false;
            if (auto spr = GetGOComponent<CS230::Sprite>()) spr->PlayAnimation(static_cast<int>(MeleeEnemy::Animations::Idle));
        }
    }

    SetPosition({ rect.Left(), rect.Bottom() });
    UpdateGOComponents(dt);

    if (auto spr = GetGOComponent<CS230::Sprite>()) {
        Math::vec2 newScale{ 1.0, 1.0 };
        if (spr->GetFrameSize().x > 0) newScale.x = rect.Size().x / spr->GetFrameSize().x;
        if (spr->GetFrameSize().y > 0) newScale.y = rect.Size().y / spr->GetFrameSize().y;
        newScale.x = (moveDir < 0) ? -std::abs(newScale.x) : std::abs(newScale.x);
        SetScale(newScale);
    }
}

void CS230::MeleeEnemy::Draw(Math::TransformationMatrix camM)
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
        unsigned int color = GAME200::BLUE;
        if (hitCooldownSec > 0.0) color = GAME200::RED;
        else if (telegraphActive) color = 0xFFFF00FF;
        else if (meleeAttackActive || jumpActive) color = GAME200::ORANGE;
        Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(Center()) * Math::ScaleMatrix(rect.Size()), color, GAME200::BLACK, 1.0);
    }

    if (telegraphActive) DrawTelegraph(camM);

#ifdef _DEBUG
    if (auto showCollision = Engine::GetGameStateManager().GetGSComponent<CS230::ShowCollision>())
    {
        if (showCollision->Enabled() && meleeAttackActive)
        {
            Math::vec2 sz = GetAttackBox().Size();
            Math::vec2 cen{ GetAttackBox().Left() + sz.x * 0.5, GetAttackBox().Bottom() + sz.y * 0.5 };
            Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(cen) * Math::ScaleMatrix(sz), 0xFF000080, 0xFF000080, 0.0);
        }
    }
#endif

    DrawStatus(camM);
}

void CS230::MeleeEnemy::Unload() { active = false; CancelAttack(); ClearGOComponents(); }

void CS230::MeleeEnemy::ResolveCollision(CS230::GameObject* other_object)
{
    if (!active || !other_object) return;
    auto* otCol = other_object->GetGOComponent<CS230::RectCollision>();
    if (!otCol) return;

    Math::rect otRect = otCol->WorldBoundary();
    if (!Math::IntersectsRect(GetRect(), otRect)) return;

    if (other_object->Type() == GameObjectTypes::Platform) {
        Enemy::ResolveCollision(GameObjectTypes::Platform, &otRect);
    }
    else if (other_object->Type() == GameObjectTypes::Player) {
        auto* player = dynamic_cast<CS230::Player*>(other_object);
        if (!player || player->IsRolling()) return;

        if (jumpActive && Math::IntersectsRect(player->GetRect(), GetRect()) && player->CanTakeHit()) {
            player->TakeHit();
            player->combat.health -= 1;
            if (player->combat.health < 0) player->combat.health = 0;
            player->ApplyKnockback(120.0, (player->Center().x >= Center().x) ? 1 : -1);
            OnSuccessfulHitPlayer();
        }
    }
}

void CS230::MeleeEnemy::OnParriedByPlayer(Player& player)
{
    player.NotifyInvincibleHit();
    player.AddTimeGauge(player.gaugeGainParry);
    player.EndParry();
}
