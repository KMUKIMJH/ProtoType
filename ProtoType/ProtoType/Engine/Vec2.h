/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Vec2.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 19, 2025
*/

#pragma once
#include <limits>
#include <cmath>

namespace Math {
    struct vec2 {
        double x{ 0.0 };
        double y{ 0.0 };

        constexpr vec2() = default;
        constexpr vec2(double x, double y) : x(x), y(y) {}

        double Length() const;
        double LengthSquared() const;
        vec2 Normalize() const;          
        vec2 Rotated(double angle) const;
        vec2 Perp() const;                
        vec2 operator-() const;

        bool operator==(const vec2& v) const;
        bool operator!=(const vec2& v) const;

        vec2 operator+(const vec2& v) const;
        vec2& operator+=(const vec2& v);

        vec2 operator-(const vec2& v) const;
        vec2& operator-=(const vec2& v);

        vec2 operator*(double scale) const;
        vec2& operator*=(double scale);

        vec2 operator/(double divisor) const;
        vec2& operator/=(double divisor);
    };

    vec2 operator*(double scale, const vec2& v);

    double Dot(const vec2& a, const vec2& b);
    double Cross(const vec2& a, const vec2& b); 
    vec2 Lerp(const vec2& a, const vec2& b, double t);

    struct ivec2 {
        int x{ 0 };
        int y{ 0 };

        constexpr ivec2() = default;
        constexpr ivec2(int x, int y) : x(x), y(y) {};

        ivec2 operator-() const;

        explicit operator vec2() const {
            return vec2{ static_cast<double>(x),static_cast<double>(y) };
        }

        bool operator==(const ivec2& v) const;
        bool operator!=(const ivec2& v) const;

        ivec2 operator+(const ivec2& v) const;
        ivec2& operator+=(const ivec2& v);

        ivec2 operator-(const ivec2& v) const;
        ivec2& operator-=(const ivec2& v);

        ivec2 operator*(int scale) const;
        ivec2& operator*=(int scale);

        ivec2 operator/(int divisor) const;
        ivec2& operator/=(int divisor);

        vec2 operator*(double scale) const;
        vec2 operator/(double divisor) const;

    };

}
