#include "HPStation.h"
#include "../Engine/Engine.h"
#include "../OpenGL/RGBA.h"
#include "../Engine/Sprite.h" 

namespace CS230
{
    HPStation::HPStation(Math::vec2 pos) : GameObject(pos)
    {
        rect = Math::rect{ Math::vec2{ pos.x - 75.0, pos.y }, Math::vec2{ pos.x + 75.0, pos.y + 200.0 } };
        AddGOComponent(new CS230::Sprite("Assets/image/Objects/HPStation.spt", this));
    }

    void HPStation::Update(double dt)
    {
        GameObject::Update(dt);
    }

    void HPStation::Draw(Math::TransformationMatrix camM)
    {
        GameObject::Draw(camM);
    }
}