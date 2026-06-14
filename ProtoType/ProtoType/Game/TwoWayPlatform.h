#pragma once
#include "GameObjectTypes.h"
#include "../Engine/GameObject.h"
#include "../Engine/Rect.h"


class TwoWayPlatform : public CS230::GameObject
{
public:
    TwoWayPlatform(Math::irect boundary)
        : CS230::GameObject(static_cast<Math::vec2>(boundary.point_1))
    {
        AddGOComponent(new CS230::RectCollision({ Math::ivec2{ 0, 0 }, boundary.Size() }, this));
    }

    std::string TypeName() override
    {
        return "TwoWayPlatform";
    }

    GameObjectTypes Type() override
    {
        return GameObjectTypes::TwoWayPlatform;
    }
};