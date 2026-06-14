#include "Bullet.h"
#include "../Engine/Engine.h"
#include "../Engine/ShowCollision.h"
#include "../Engine/Sprite.h"
#include "../OpenGL/RGBA.h"

using namespace CS230;

Bullet::Bullet() : CS230::GameObject({ 0,0 }) {}

void Bullet::Reset()
{
    rect = { {0.0,0.0}, {0.0,0.0} };
    velocity = { 0.0,0.0 };
    spawnCenter = { 0.0,0.0 };
    active = false;
    grazed = false;
    reflected = false;
    owner = Owner::Enemy;
}

void Bullet::InitSprite(const std::string& spritePath, Math::vec2 spriteScale)
{
    ClearGOComponents();
    AddGOComponent(new CS230::Sprite(spritePath, this));
    SetScale(spriteScale);
}

void Bullet::Update(double dt)
{
    if (!active) return;

    velocity.y -= gravity * dt;

    rect.point_1.x += velocity.x * dt;
    rect.point_2.x += velocity.x * dt;
    rect.point_1.y += velocity.y * dt;
    rect.point_2.y += velocity.y * dt;

    rotation = std::atan2(velocity.y, velocity.x);

    Math::vec2 bsize = rect.Size();
    Math::vec2 bcenter = { rect.Left() + bsize.x * 0.5, rect.Bottom() + bsize.y * 0.5 };
    SetPosition(bcenter);
    UpdateGOComponents(dt);
}

void Bullet::Draw(Math::TransformationMatrix camM)
{
    if (!active)
    {
        return;
    }

    const Math::vec2 bsize{ rect.Size() };
    const Math::vec2 bcenter{ rect.Left() + bsize.x * 0.5, rect.Bottom() + bsize.y * 0.5 };

    Math::TransformationMatrix rotatedCamM = camM
        * Math::TranslationMatrix(bcenter)
        * Math::RotationMatrix(rotation)
        * Math::TranslationMatrix(Math::vec2{ -bcenter.x, -bcenter.y });

    CS230::GameObject::Draw(rotatedCamM);

#ifdef _DEBUG
    bool showDebug = false;
    if (auto sc = ::Engine::GetGameStateManager().GetGSComponent<CS230::ShowCollision>())
    {
        showDebug = sc->Enabled();
    }
    if (showDebug)
    {
        Math::TransformationMatrix debugM = camM * Math::TranslationMatrix(bcenter) * Math::RotationMatrix(rotation) * Math::ScaleMatrix(bsize);
        ::Engine::GetRenderer2D().DrawRectangle(debugM, GAME200::CLEAR, GAME200::RED, 1.0);
    }
#endif
}