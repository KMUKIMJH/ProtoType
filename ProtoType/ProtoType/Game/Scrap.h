#pragma once
#include "../Engine/GameObject.h"
#include "../Engine/Vec2.h"
#include <string>

namespace CS230
{
    class Player;

    class Scrap : public GameObject
    {
    public:
        Scrap(Math::vec2 pos);
        void Update(double dt) override;
        void Draw(Math::TransformationMatrix cameraMatrix) override;

        bool CanDraw() const;
        bool IsConsumed() const;
        Math::rect GetRect() const;
        GameObjectTypes Type() override;
        std::string TypeName() override;

        bool ResolveCollision(Player* player);
        void ResolveCollision(GameObject* other_object) override;

    private:
        bool consumed{ false };
        Math::vec2 position;
        Math::vec2 velocity{ 0.0, 0.0 };
    };
}