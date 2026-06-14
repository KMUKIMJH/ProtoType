#pragma once
#include "../Engine/GameObject.h"
#include "../Engine/Vec2.h"
#include <string>

namespace CS230
{
    class Player;

    class StoryItem : public GameObject
    {
    public:
        StoryItem(Math::vec2 pos, bool isBossDrop = false); 
        void Update(double dt) override;
        void Draw(Math::TransformationMatrix cameraMatrix) override;

        bool CanDraw() const;
        bool IsConsumed() const;
        Math::rect GetRect() const;
        GameObjectTypes Type() override;
        std::string TypeName() override;

        bool ResolveCollision(Player* player);
        void ResolveCollision(GameObject* other_object) override;

        bool IsBossItem() const { return isBossDrop; }

    private:
        bool consumed{ false };
        bool isBossDrop{ false };
        Math::vec2 position;
        Math::vec2 velocity{ 0.0, 0.0 };
    };
}