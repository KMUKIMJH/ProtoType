/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Camera.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 30, 2025
*/

#pragma once

#include "Vec2.h"
#include "Rect.h"
#include "Matrix.h"
#include "Componentmanager.h"

namespace CS230 {
    class Camera : public Component {
    public:
        Camera(Math::rect player_zone);
        void SetZoom(double zoom) { zoomOut = zoom; }
        double GetZoom() const { return zoomOut; }
        void SetPosition(Math::vec2 new_position);
        const Math::vec2& GetPosition() const;
        void SetLimit(Math::irect new_limit);
        Math::irect GetLimit() const { return limit; }
        void Update(const Math::vec2& player_position);
        Math::TransformationMatrix GetMatrix();
    private:
        Math::vec2 position;
        Math::rect player_zone;
        Math::irect limit;
        double zoomOut = 1.0;

        double last_x{ 0.0 };
        double last_y{ 0.0 };
        bool isFacingRight{ true };
        double target_y_ratio{ 0.25 };
        bool isFirstUpdate{ true };
    };
}