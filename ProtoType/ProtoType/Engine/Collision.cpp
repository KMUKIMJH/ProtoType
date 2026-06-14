/* Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Collision.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 13, 2025
*/
#include "Collision.h"
#include <cmath>
#include <algorithm>

#ifndef PI
#define PI 3.14159265358979323846
#endif


CS230::RectCollision::RectCollision(Math::irect boundary, GameObject* object) :
    boundary(boundary),
    object(object)
{}

Math::rect CS230::RectCollision::WorldBoundary()
{
    Math::vec2 hotspotAdjust{ 0.0, 0.0 };
    if (auto spr = object->GetGOComponent<CS230::Sprite>())
    {
        Math::ivec2 fs = spr->GetFrameSize();
        Math::ivec2 hs = spr->GetHotSpot(0);
        hotspotAdjust = { static_cast<double>(hs.x) - fs.x * 0.5, static_cast<double>(hs.y) - fs.y * 0.5 };
    }

    Math::TransformationMatrix objMatrix = object->GetMatrix();

    const Math::vec2 p1 = objMatrix * (static_cast<Math::vec2>(boundary.point_1) + hotspotAdjust);
    const Math::vec2 p2 = objMatrix * (static_cast<Math::vec2>(boundary.point_2) + hotspotAdjust);
    const Math::vec2 p3 = objMatrix * (static_cast<Math::vec2>(Math::ivec2{ boundary.point_1.x, boundary.point_2.y }) + hotspotAdjust);
    const Math::vec2 p4 = objMatrix * (static_cast<Math::vec2>(Math::ivec2{ boundary.point_2.x, boundary.point_1.y }) + hotspotAdjust);

    Math::vec2 minP{ std::min(std::min(p1.x, p2.x), std::min(p3.x, p4.x)), std::min(std::min(p1.y, p2.y), std::min(p3.y, p4.y)) };
    Math::vec2 maxP{ std::max(std::max(p1.x, p2.x), std::max(p3.x, p4.x)), std::max(std::max(p1.y, p2.y), std::max(p3.y, p4.y)) };

    return { minP, maxP };
}

void CS230::RectCollision::Draw(Math::TransformationMatrix display_matrix)
{
    Math::rect world_boundary = WorldBoundary();

    Math::vec2 size = { world_boundary.Right() - world_boundary.Left(), world_boundary.Top() - world_boundary.Bottom() };
    Math::vec2 center = { world_boundary.Left() + size.x / 2.0, world_boundary.Bottom() + size.y / 2.0 };

    Math::TransformationMatrix transform =
        display_matrix * Math::TranslationMatrix(center) * Math::ScaleMatrix(size);

    Engine::GetRenderer2D().DrawRectangle(transform, GAME200::CLEAR, GAME200::WHITE, 1.0);
}

bool CS230::RectCollision::IsCollidingWith(GameObject* other_object)
{
    Collision* other_collider = other_object->GetGOComponent<Collision>();

    if (other_collider == nullptr)
    {
        return false;
    }

    if (other_collider->Shape() != CollisionShape::Rect)
    {
        Engine::GetLogger().LogError("Rect vs unsupported type");
        return false;
    }

    Math::rect rectangle_1 = WorldBoundary();
    Math::rect rectangle_2 = static_cast<RectCollision*>(other_collider)->WorldBoundary();

    return Math::IntersectsRect(rectangle_1, rectangle_2);
}

bool CS230::RectCollision::IsCollidingWith(Math::vec2 point)
{
    Math::rect rectangle_1 = WorldBoundary();
    if (rectangle_1.Left() <= point.x && rectangle_1.Right() >= point.x && rectangle_1.Bottom() <= point.y && rectangle_1.Top() >= point.y)
    {
        return true;
    }
    return false;
}

CS230::CircleCollision::CircleCollision(double radius, GameObject* object) :
    radius(radius),
    object(object)
{}

void CS230::CircleCollision::Draw(Math::TransformationMatrix display_matrix)
{
    Math::vec2 center = object->GetPosition();
    if (auto rc = object->GetGOComponent<CS230::RectCollision>())
    {
        Math::rect wb = rc->WorldBoundary();
        Math::vec2 size = { wb.Right() - wb.Left(), wb.Top() - wb.Bottom() };
        center = { wb.Left() + size.x * 0.5, wb.Bottom() + size.y * 0.5 };
    }
    double draw_radius = GetRadius();

    Math::TransformationMatrix transform =
        display_matrix * Math::TranslationMatrix(center) * Math::ScaleMatrix({ draw_radius * 2, draw_radius * 2 });

    Engine::GetRenderer2D().DrawCircle(transform, GAME200::CLEAR, GAME200::ORANGE, 1.0);
}

double CS230::CircleCollision::GetRadius()
{
    return radius;
}

bool CS230::CircleCollision::IsCollidingWith(GameObject* other_object)
{
    Collision* other_collider = other_object->GetGOComponent<Collision>();

    if (other_collider == nullptr)
    {
        return false;
    }
    if (other_collider->Shape() != CollisionShape::Circle)
    {
        Engine::GetLogger().LogError("Circle vs unsupported type");
        return false;
    }

    Math::vec2 position_1 = object->GetPosition();
    Math::vec2 position_2 = other_object->GetPosition();

    double circle_1 = GetRadius();
    double circle_2 = static_cast<CircleCollision*>(other_collider)->GetRadius();

    if ((position_1.x - position_2.x) * (position_1.x - position_2.x) + (position_1.y - position_2.y) * (position_1.y - position_2.y) < (circle_1 + circle_2) * (circle_1 + circle_2))
    {
        return true;
    }
    return false;
}

bool CS230::CircleCollision::IsCollidingWith(Math::vec2 point)
{
    double circle_1 = GetRadius();
    Math::vec2 position_1 = object->GetPosition();
    if ((position_1.x - point.x) * (position_1.x - point.x) + (position_1.y - point.y) * (position_1.y - point.y) <= circle_1 * circle_1)
    {
        return true;
    }
    return false;
}

bool CS230::CollisionHelpers::IntersectsCircleAABB(Math::vec2 circleCenter, double radius, const Math::rect& aabb) noexcept
{
    const double closestX = std::min(std::max(circleCenter.x, aabb.Left()), aabb.Right());
    const double closestY = std::min(std::max(circleCenter.y, aabb.Bottom()), aabb.Top());
    const double dx = circleCenter.x - closestX;
    const double dy = circleCenter.y - closestY;
    return (dx * dx + dy * dy) <= (radius * radius);
}