/* Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Rect.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 30, 2025
*/
#pragma once
#include "Vec2.h"
#include <cmath>

namespace Math {
    struct [[nodiscard]] rect {
        Math::vec2 point_1{ 0.0, 0.0 };
        Math::vec2 point_2{ 0.0, 0.0 };

        double Left() const noexcept;
        double Right() const noexcept;
        double Top() const noexcept;
        double Bottom() const noexcept;
        Math::vec2 Size() const noexcept {
            return {
                Right() - Left(),
                std::abs(Top() - Bottom())
            };
        }
    };
    struct [[nodiscard]] irect {
        Math::ivec2 point_1{ 0, 0 };
        Math::ivec2 point_2{ 0, 0 };

        int Left() const noexcept;
        int Right() const noexcept;
        int Top() const noexcept;
        int Bottom() const noexcept;
        Math::ivec2 Size() const noexcept {
            return {
                Right() - Left(),
                std::abs(Top() - Bottom())
            };
        }
    };

    // Rectangle helpers moved from ProtoHelpers
    inline bool IntersectsRect(const rect& a, const rect& b) noexcept
    {
        return !(a.Right() <= b.Left() || a.Left() >= b.Right() ||
                 a.Top() <= b.Bottom() || a.Bottom() >= b.Top());
    }

    inline Math::vec2 ComputeMTV(const rect& pr, const rect& er) noexcept
    {
        const double pushRight = er.Right() - pr.Left();
        const double pushLeft  = pr.Right()  - er.Left();
        const double pushUp    = er.Top()    - pr.Bottom();
        const double pushDown  = pr.Top()    - er.Bottom();

        double minPen = pushRight;
        Math::vec2 mtv{ +pushRight, 0.0 };
        if (pushLeft < minPen)  { minPen = pushLeft;  mtv = { -pushLeft, 0.0 }; }
        if (pushUp   < minPen)  { minPen = pushUp;    mtv = { 0.0, +pushUp }; }
        if (pushDown < minPen)  {  mtv = { 0.0, -pushDown }; }
        return mtv;
    }

    inline Math::vec2 RectCenter(const rect& r) noexcept
    {
        return { r.Left() + (r.Right() - r.Left()) * 0.5, r.Bottom() + (r.Top() - r.Bottom()) * 0.5 };
    }

    inline Math::vec2 RectSize(const rect& r) noexcept
    {
        return { r.Right() - r.Left(), r.Top() - r.Bottom() };
    }

}
