/* Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Collision.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 13, 2025
*/
#pragma once
#include "Rect.h"
#include "Component.h"
#include "GameObject.h"

namespace Math {
    class TransformationMatrix;
}

namespace CS230 {
    class GameObject;

    class Collision : public Component {
    public:
        enum class CollisionShape {
            Rect,
            Circle
        };
        virtual bool IsCollidingWith(GameObject* other_object) = 0;
        virtual bool IsCollidingWith(Math::vec2 point) = 0;
        virtual CollisionShape Shape() = 0;
        virtual void Draw(Math::TransformationMatrix display_matrix) = 0;
    };

    class RectCollision : public Collision {
    public:
        RectCollision(Math::irect boundary, GameObject* object);
        CollisionShape Shape() override {
            return CollisionShape::Rect;
        }
        bool IsCollidingWith(GameObject* other_object) override;
        bool IsCollidingWith(Math::vec2 point) override;
        void Draw(Math::TransformationMatrix display_matrix) override;
        Math::rect WorldBoundary();
    private:
        GameObject* object;
        Math::irect boundary;
    };

    class CircleCollision : public Collision {
    public:
        CircleCollision(double radius, GameObject* object);
        CollisionShape Shape() override {
            return CollisionShape::Circle;
        }
        bool IsCollidingWith(GameObject* other_object) override;
        bool IsCollidingWith(Math::vec2 point) override;
        void Draw(Math::TransformationMatrix display_matrix);
        double GetRadius();
    private:
        GameObject* object;
        double radius;
    };

    namespace CollisionHelpers 
    {
        bool IntersectsCircleAABB(Math::vec2 circleCenter, double radius, const Math::rect& aabb) noexcept;
    }

}

