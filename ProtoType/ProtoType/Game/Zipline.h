#pragma once
#include "GameObjectTypes.h"
#include "../Engine/GameObject.h"
#include "../Engine/Rect.h"
#include "../Engine/Engine.h"
#include "../OpenGL/SimpleRenderer2D.h"

class Zipline : public CS230::GameObject
{
public:
    Zipline(Math::irect boundary)
        : CS230::GameObject(static_cast<Math::vec2>(boundary.point_1))
    {
        int originalWidth = boundary.Size().x;
        int height = boundary.Size().y;

        int newHitboxWidth = 20;

        int offsetX = (originalWidth - newHitboxWidth) / 2;

        AddGOComponent(new CS230::RectCollision({ Math::ivec2{ offsetX, 0 }, Math::ivec2{ offsetX + newHitboxWidth, height } }, this));
    }

    void Draw(Math::TransformationMatrix camM) override
    {
        auto* rc = GetGOComponent<CS230::RectCollision>();
        if (rc)
        {
            Math::rect r = rc->WorldBoundary();
            Math::vec2 sz = r.Size();
            Math::vec2 c = { r.Left() + sz.x * 0.5, r.Bottom() + sz.y * 0.5 };

            Math::vec2 lineSize = { 10.0, sz.y };

            ::Engine::GetRenderer2D().DrawRectangle(camM * Math::TranslationMatrix(c) * Math::ScaleMatrix(lineSize),
                GAME200::ORANGE, GAME200::ORANGE, 1.0);
        }
    }

    std::string TypeName() override
    {
        return "Zipline";
    }

    GameObjectTypes Type() override
    {
        return GameObjectTypes::Zipline;
    }
};