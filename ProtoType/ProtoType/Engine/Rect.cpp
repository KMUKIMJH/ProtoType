/* Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Rect.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 13, 2025
*/

#include "Rect.h"
#include <algorithm>

namespace Math
{
    double rect::Left() const noexcept {
        return std::min(point_1.x, point_2.x);
    }
    double rect::Right() const noexcept {
        return std::max(point_1.x, point_2.x);
    }
    double rect::Top() const noexcept {
        return std::max(point_1.y, point_2.y);
    }
    double rect::Bottom() const noexcept {
        return std::min(point_1.y, point_2.y);
    }
    int irect::Left() const noexcept {
        return std::min(point_1.x, point_2.x);
    }
    int irect::Right() const noexcept {
        return std::max(point_1.x, point_2.x);
    }
    int irect::Top() const noexcept {
        return std::max(point_1.y, point_2.y);
    }
    int irect::Bottom() const noexcept {
        return std::min(point_1.y, point_2.y);
    }
}