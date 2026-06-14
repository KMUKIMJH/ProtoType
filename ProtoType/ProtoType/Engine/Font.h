/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Font.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 3, 2025
*/

#pragma once
#include "Rect.h"
#include "Texture.h"
#include "Vec2.h"
#include <filesystem>
#include <memory>
#include <string>
#include "../OpenGL/Image.h"

namespace CS230 {
    class Font {
    public:
        Font(const std::filesystem::path& file_name);
        std::shared_ptr<Texture> PrintToTexture(const std::string& text, unsigned int color = 0xFFFFFFFF);
        std::shared_ptr<Texture> PrintToTextureScaled(const std::string& text, unsigned int color, double scale);

    private:
        void FindCharRects(const GAME200::Image& image);
        const Math::irect& GetCharRect(char c) const;
        Math::ivec2  MeasureText(const std::string& text) const;
        static unsigned int GetPixel(const GAME200::Image& image, Math::ivec2 texel);

        std::shared_ptr<Texture> texture;
        static constexpr int num_chars = 'z' - ' ' + 1;
        Math::irect char_rects[num_chars]{};
    };
}
