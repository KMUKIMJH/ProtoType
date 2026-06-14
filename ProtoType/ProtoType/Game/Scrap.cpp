#include "Scrap.h"
#include "Player.h"
#include "GameObjectTypes.h"
#include "../Engine/Engine.h"
#include "../Engine/Collision.h"
#include "../Game/Gravity.h"
#include "../Engine/Sprite.h"

namespace CS230
{
    Scrap::Scrap(Math::vec2 pos) : GameObject(pos), position(pos), consumed(false)
    {
        velocity = { 0.0, 300.0 };
        AddGOComponent(new CS230::Sprite("Assets/image/Objects/GearScrap.spt", this));
    }

    void Scrap::Update(double dt)
    {
        if (consumed)
        {
            return;
        }

        double grav = 2400.0;
        if (auto g = Engine::GetGameStateManager().GetGSComponent<Gravity>())
        {
            grav = g->GetValue();
        }

        velocity.y -= grav * dt;
        if (velocity.y < -1200.0)
        {
            velocity.y = -1200.0;
        }

        position.x += velocity.x * dt;
        position.y += velocity.y * dt;

        SetPosition(position);
        UpdateGOComponents(dt);

        if (auto spr = GetGOComponent<CS230::Sprite>())
        {
            Math::ivec2 fs = spr->GetFrameSize();
            if (fs.x > 0 && fs.y > 0)
            {
                SetScale(Math::vec2{ 40.0 / static_cast<double>(fs.x), 40.0 / static_cast<double>(fs.y) });
            }
        }
    }

    void Scrap::Draw(Math::TransformationMatrix cameraMatrix)
    {
        if (consumed)
        {
            return;
        }

        CS230::GameObject::Draw(cameraMatrix);
    }

    bool Scrap::CanDraw() const
    {
        return !consumed;
    }

    bool Scrap::IsConsumed() const
    {
        return consumed;
    }

    Math::rect Scrap::GetRect() const
    {
        return Math::rect{ { position.x - 20.0, position.y - 20.0 }, { position.x + 20.0, position.y + 20.0 } };
    }

    GameObjectTypes Scrap::Type()
    {
        return GameObjectTypes::Count;
    }

    std::string Scrap::TypeName()
    {
        return "Scrap";
    }

    bool Scrap::ResolveCollision(Player* player)
    {
        if (consumed || !player)
        {
            return false;
        }

        Math::rect hr = GetRect();
        Math::rect pr = player->GetRect();

        if (!(pr.Right() < hr.Left() || pr.Left() > hr.Right() || pr.Top() < hr.Bottom() || pr.Bottom() > hr.Top()))
        {
            player->AddScrap(1);
            consumed = true;
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/coin.wav");
            return true;
        }

        return false;
    }

    void Scrap::ResolveCollision(GameObject* other_object)
    {
        if (consumed || !other_object)
        {
            return;
        }

        if (other_object->Type() == GameObjectTypes::Platform || other_object->Type() == GameObjectTypes::TwoWayPlatform)
        {
            auto* otCol = other_object->GetGOComponent<CS230::RectCollision>();
            if (!otCol)
            {
                return;
            }

            Math::rect otR = otCol->WorldBoundary();
            Math::rect myR = GetRect();

            if (!(myR.Right() <= otR.Left() || myR.Left() >= otR.Right() || myR.Top() <= otR.Bottom() || myR.Bottom() >= otR.Top()))
            {
                if (velocity.y < 0.0)
                {
                    double overlapTop = otR.Top() - myR.Bottom();

                    if (overlapTop > 0.0 && overlapTop < 40.0)
                    {
                        position.y += overlapTop;
                        velocity.y = 0.0;
                        SetPosition(position);
                    }
                }
            }
        }
    }
}