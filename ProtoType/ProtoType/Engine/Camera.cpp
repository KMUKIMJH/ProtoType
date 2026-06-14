/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Camera.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 30, 2025
*/
#include "Camera.h"
#include "Engine.h"
#include "Window.h"

void CS230::Camera::Update(const Math::vec2& player_position)
{
    double current_zoom = (zoomOut <= 0.0) ? 1.0 : zoomOut;

    const double virtual_width = 1920.0;
    const double virtual_height = 1080.0;
    const double view_width = virtual_width / current_zoom;
    const double view_height = virtual_height / current_zoom;

    if (isFirstUpdate)
    {
        position.x = player_position.x - view_width * 0.5;
        position.y = player_position.y - view_height * 0.5;
        isFirstUpdate = false;
    }

    Math::vec2 cam_center;
    cam_center.x = position.x + view_width * 0.5;
    cam_center.y = position.y + view_height * 0.5;

    const double deadzone_w = view_width * 0.02;
    const double deadzone_h = view_height * 0.05;

    Math::vec2 target_center = cam_center;

    if (player_position.x > cam_center.x + deadzone_w * 0.5)
    {
        target_center.x = player_position.x - deadzone_w * 0.5;
    }
    else if (player_position.x < cam_center.x - deadzone_w * 0.5)
    {
        target_center.x = player_position.x + deadzone_w * 0.5;
    }

    if (player_position.y > cam_center.y + deadzone_h * 0.5)
    {
        target_center.y = player_position.y - deadzone_h * 0.5;
    }
    else if (player_position.y < cam_center.y - deadzone_h * 0.5)
    {
        target_center.y = player_position.y + deadzone_h * 0.5;
    }

    const double lerp_factor_x = 0.3;
    const double lerp_factor_y = 0.25;

    cam_center.x += (target_center.x - cam_center.x) * lerp_factor_x;
    cam_center.y += (target_center.y - cam_center.y) * lerp_factor_y;

    position.x = cam_center.x - view_width * 0.5;
    position.y = cam_center.y - view_height * 0.5;

    if (position.x < limit.Left())
    {
        position.x = limit.Left();
    }
    if (position.x > limit.Right())
    {
        position.x = limit.Right();
    }
    if (position.y < limit.Bottom())
    {
        position.y = limit.Bottom();
    }
    if (position.y > limit.Top())
    {
        position.y = limit.Top();
    }
}

CS230::Camera::Camera(Math::rect player_zone) : player_zone(player_zone), position{ 0, 0 }
{}

void CS230::Camera::SetPosition(Math::vec2 new_position)
{
    position = new_position;
}

const Math::vec2& CS230::Camera::GetPosition() const
{
    return position;
}

void CS230::Camera::SetLimit(Math::irect new_limit)
{
    limit = new_limit;

    isFirstUpdate = true;
    target_y_ratio = 0.25;
    isFacingRight = true;
}

Math::TransformationMatrix CS230::Camera::GetMatrix()
{
    return Math::ScaleMatrix(zoomOut) * Math::TranslationMatrix(-position);
}