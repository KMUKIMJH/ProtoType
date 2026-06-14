#pragma once
#include "../Engine/Rect.h"
#include "../Engine/Vec2.h"
#include "../Engine/Matrix.h"
#include <string>
#include "../Engine/GameObject.h"
#include "GameObjectTypes.h"

namespace CS230
{
    class Bullet : public CS230::GameObject
    {
    public:
        enum class Owner
        {
            Enemy,
            Player
        };
        Bullet();

        void Reset();
        void InitSprite(const std::string& spritePath, Math::vec2 spriteScale);

        void Update(double dt) override;
        void Draw(Math::TransformationMatrix camM) override;

        GameObjectTypes Type() override
        {
            return GameObjectTypes::Particle;
        }
        std::string TypeName() override
        {
            return "Bullet";
        }

        Math::rect rect{ {0.0, 0.0}, {0.0, 0.0} };
        Math::vec2 velocity{ 0.0, 0.0 };
        Math::vec2 spawnCenter{ 0.0, 0.0 };
        double gravity{ 0.0 };
        double rotation{ 0.0 };
        bool active{ false };
        bool grazed{ false };
        bool reflected{ false };
        Owner owner{ Owner::Enemy };
    };
}