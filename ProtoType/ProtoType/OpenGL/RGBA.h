#pragma once

#include <array>
#include <cstdint>
#include <algorithm>

namespace GAME200
{
    using RGBA = uint32_t;

    inline constexpr RGBA CLEAR    = 0x00000000u;
    inline constexpr RGBA WHITE    = 0xFFF5F5FFu; 
    inline constexpr RGBA RED      = 0xFF0000FFu;
    inline constexpr RGBA ORANGE   = 0xFFA500FFu;
    inline constexpr RGBA GRAY     = 0x808080FFu;
    inline constexpr RGBA DARKGRAY = 0x404040FFu;
    inline constexpr RGBA BLACK    = 0x000000FFu;
    inline constexpr RGBA SKYBLUE  = 0x87CEEBFFu;
    inline constexpr RGBA DKBLUE   = 0x00008BFFu;
    inline constexpr RGBA GOLD     = 0xFFD700FFu;
    inline constexpr RGBA YELLOW   = 0xFFFF00FFu;
    inline constexpr RGBA MAROON   = 0x800000FFu;
    inline constexpr RGBA PURPLE   = 0x800080FFu;
    inline constexpr RGBA GREEN    = 0x00FF00FFu;
    inline constexpr RGBA DKGREEN = 0x006400FFu;
    inline constexpr RGBA LIGHTGRAY= 0xC8C8C8FFu;
    inline constexpr RGBA BROWN    = 0x8B4513FFu; 
    inline constexpr RGBA BLUE     = 0x0000FFFFu;
    inline constexpr RGBA CYAN     = 0x00FFFFFFu;

    inline constexpr std::array<float, 4> unpack_color(RGBA rgba) noexcept
    {
        constexpr float inv255 = 1.0f / 255.0f;
        const uint32_t r = (rgba >> 24) & 0xFFu;
        const uint32_t g = (rgba >> 16) & 0xFFu;
        const uint32_t b = (rgba >> 8)  & 0xFFu;
        const uint32_t a = (rgba >> 0)  & 0xFFu;
        return { r * inv255, g * inv255, b * inv255, a * inv255 };
    }

    inline constexpr RGBA pack_color(const std::array<float, 4>& color) noexcept
    {
        auto clamp01 = [](float v) constexpr -> float { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };
        const uint32_t R = static_cast<uint32_t>(clamp01(color[0]) * 255.0f + 0.5f) & 0xFFu;
        const uint32_t G = static_cast<uint32_t>(clamp01(color[1]) * 255.0f + 0.5f) & 0xFFu;
        const uint32_t B = static_cast<uint32_t>(clamp01(color[2]) * 255.0f + 0.5f) & 0xFFu;
        const uint32_t A = static_cast<uint32_t>(clamp01(color[3]) * 255.0f + 0.5f) & 0xFFu;
        return (R << 24) | (G << 16) | (B << 8) | (A << 0);
    }

    inline constexpr uint32_t rgba_to_abgr(RGBA rgba) noexcept
    {
        const uint32_t r = (rgba >> 24) & 0xFFu;
        const uint32_t g = (rgba >> 16) & 0xFFu;
        const uint32_t b = (rgba >> 8)  & 0xFFu;
        const uint32_t a = (rgba >> 0)  & 0xFFu;
        return (a << 24) | (b << 16) | (g << 8) | (r << 0);
    }
}
