/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Vec2.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 19, 2025
*/
#include <cmath>
#include "Vec2.h"

namespace Math
{

vec2 vec2::operator+(const vec2& v) const { return { x + v.x, y + v.y }; }
vec2& vec2::operator+=(const vec2& v) { x += v.x; y += v.y; return *this; }
vec2 vec2::operator-(const vec2& v) const { return { x - v.x, y - v.y }; }
vec2& vec2::operator-=(const vec2& v) { x -= v.x; y -= v.y; return *this; }
vec2 vec2::operator*(double s) const { return { x * s, y * s }; }
vec2& vec2::operator*=(double s) { x *= s; y *= s; return *this; }
vec2 vec2::operator/(double d) const { return { x / d, y / d }; }
vec2& vec2::operator/=(double d) { x /= d; y /= d; return *this; }

bool vec2::operator==(const vec2& v) const { return x == v.x && y == v.y; }
bool vec2::operator!=(const vec2& v) const { return !(*this == v); }

vec2 operator*(double scale, const vec2& v) { return { v.x * scale, v.y * scale }; }

double vec2::Length() const { return std::sqrt(x * x + y * y); }
double vec2::LengthSquared() const { return x * x + y * y; }
vec2 vec2::Normalize() const { const double len = Length(); return len > 0.0 ? vec2{x / len, y / len} : vec2{0.0, 0.0}; }
vec2 vec2::Rotated(double angle) const { const double c = std::cos(angle), s = std::sin(angle); return vec2(x * c - y * s, x * s + y * c); }
vec2 vec2::Perp() const { return vec2(-y, x); }
vec2 vec2::operator-() const { return vec2(-x, -y); }

double Dot(const vec2& a, const vec2& b) { return a.x * b.x + a.y * b.y; }
double Cross(const vec2& a, const vec2& b) { return a.x * b.y - a.y * b.x; }
vec2 Lerp(const vec2& a, const vec2& b, double t) { return a + (b - a) * t; }

ivec2 ivec2::operator+(const ivec2& v) const { return { x + v.x, y + v.y }; }
ivec2& ivec2::operator+=(const ivec2& v) { x += v.x; y += v.y; return *this; }
ivec2 ivec2::operator-(const ivec2& v) const { return { x - v.x, y - v.y }; }
ivec2& ivec2::operator-=(const ivec2& v) { x -= v.x; y -= v.y; return *this; }
ivec2 ivec2::operator*(int s) const { return { x * s, y * s }; }
ivec2& ivec2::operator*=(int s) { x *= s; y *= s; return *this; }
ivec2 ivec2::operator/(int d) const { return { x / d, y / d }; }
ivec2& ivec2::operator/=(int d) { x /= d; y /= d; return *this; }

bool ivec2::operator==(const ivec2& v) const { return x == v.x && y == v.y; }
bool ivec2::operator!=(const ivec2& v) const { return !(*this == v); }

vec2 ivec2::operator*(double s) const { return { s * x, s * y }; }
vec2 ivec2::operator/(double d) const { return { x / d, y / d }; }

ivec2 ivec2::operator-() const { return ivec2(-x, -y); }

} 
