#pragma once
#include "GameObjectTypes.h"
#include "../Engine/GameObject.h"
#include "../Engine/Rect.h"

class Platform : public CS230::GameObject
{
public:
    Platform(Math::irect boundary)
        : CS230::GameObject(static_cast<Math::vec2>(boundary.point_1))
    {
        AddGOComponent(new CS230::RectCollision({ Math::ivec2{ 0, 0 }, boundary.Size() }, this));
    }

    std::string TypeName() override
    {
        return "Platform";
    }
    GameObjectTypes Type() override
    {
        return GameObjectTypes::Platform;
    }
};
