/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Font.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    May 3, 2025
*/
#include <GL/glew.h> 
#include "Font.h"
#include "Engine.h"
#include "../OpenGL/Image.h"
#include <algorithm>
#include <cmath>

namespace CS230 {

    Font::Font(const std::filesystem::path& file_name)
    {
        texture = Engine::GetTextureManager().Load(file_name);

        GAME200::Image image(file_name, false);
        constexpr unsigned int white = 0xFFFFFFFFu;

        const unsigned int color = GetPixel(image, { 0, 0 });
        if (color == white)
        {
            FindCharRects(image);
        }
        else
        {
            Engine::GetLogger().LogError("Font " + file_name.string() + " could not be loaded (top-left pixel is not white)");
        }
    }

    void Font::FindCharRects(const GAME200::Image& image)
    {
        unsigned int check_color = GetPixel(image, { 0, 0 });
        unsigned int next_color;

        const int height = image.GetSize().y;

        int x = 1;
        for (int index = 0; index < num_chars; index++)
        {
            int width = 0;

            do
            {
                width++;
                next_color = GetPixel(image, { x + width, 0 });
            } while (check_color == next_color);

            check_color = next_color;

            char_rects[index].point_2 = { x + width - 1, 1 };
            char_rects[index].point_1 = { x, char_rects[index].point_2.y + height - 1 };
            x += width;
        }
    }

    const Math::irect& Font::GetCharRect(char c) const
    {
        if (c >= ' ' && c <= 'z')
        {
            return char_rects[c - ' '];
        }

        Engine::GetLogger().LogError("Char '" + std::to_string(static_cast<int>(c)) + "' not found");
        return char_rects[0];
    }

    // Save/restore GL state used by offscreen font rendering to avoid interference with stage rendering
    static void save_gl_state(GLint prevViewport[4], GLboolean& prevBlend) noexcept
    {
        glGetIntegerv(GL_VIEWPORT, prevViewport);
        prevBlend = glIsEnabled(GL_BLEND);
    }
    static void restore_gl_state(const GLint prevViewport[4], GLboolean prevBlend) noexcept
    {
        glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
        if (prevBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    }

    std::shared_ptr<Texture> Font::PrintToTexture(const std::string& text, unsigned int color)
    {
        const Math::ivec2 text_size = MeasureText(text);
        auto& texture_manager = Engine::GetTextureManager();
        auto& renderer = Engine::GetRenderer2D();

        Math::TransformationMatrix old_camera = renderer.GetViewProjectionMatrix();
        renderer.EndScene();

        GLint prevViewport[4]; GLboolean prevBlend;
        save_gl_state(prevViewport, prevBlend);

        texture_manager.StartRenderTextureMode(text_size.x, text_size.y);

        glViewport(0, 0, text_size.x, text_size.y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Math::TransformationMatrix ortho =
            Math::TranslationMatrix(Math::vec2{ -1.0, 1.0 }) * Math::ScaleMatrix(Math::vec2{ 2.0 / (double)text_size.x, -2.0 / (double)text_size.y });

        renderer.BeginScene(ortho);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (texture) {
            texture->Bind();
        }

        Math::TransformationMatrix matrix;

        for (const char c : text)
        {
            const Math::irect& display_rect = GetCharRect(c);
            const Math::ivec2 size = display_rect.Size();
            if (c != ' ')
            {
                const Math::ivec2 top_left_texel = { display_rect.point_1.x, display_rect.point_2.y };
                texture->Draw(matrix, top_left_texel, size, color);
            }
            matrix *= Math::TranslationMatrix(Math::ivec2{ size.x, 0 });
        }

        renderer.EndScene();
        auto result_texture = texture_manager.EndRenderTextureMode();
        renderer.BeginScene(old_camera);

        restore_gl_state(prevViewport, prevBlend);

        return result_texture;
    }

    std::shared_ptr<Texture> Font::PrintToTextureScaled(const std::string& text, unsigned int color, double scale)
    {
        if (scale <= 0.0) scale = 1.0;

        const Math::ivec2 base_size = MeasureText(text);
        const int target_w = static_cast<int>(std::round(static_cast<double>(base_size.x) * scale));
        const int target_h = static_cast<int>(std::round(static_cast<double>(base_size.y) * scale));

        auto& texture_manager = Engine::GetTextureManager();
        auto& renderer = Engine::GetRenderer2D();

        Math::TransformationMatrix old_camera = renderer.GetViewProjectionMatrix();
        renderer.EndScene();

        GLint prevViewport[4]; GLboolean prevBlend;
        save_gl_state(prevViewport, prevBlend);

        texture_manager.StartRenderTextureMode(target_w, target_h);

        glViewport(0, 0, target_w, target_h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        Math::TransformationMatrix ortho =
            Math::TranslationMatrix(Math::vec2{ -1.0, 1.0 }) * Math::ScaleMatrix(Math::vec2{ 2.0 / (double)target_w, -2.0 / (double)target_h });

        renderer.BeginScene(ortho);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (texture) {
            texture->Bind();
        }

        Math::TransformationMatrix pen;
        const Math::TransformationMatrix S = Math::ScaleMatrix(Math::vec2{ scale, scale });

        for (const char c : text)
        {
            const Math::irect& display_rect = GetCharRect(c);
            const Math::ivec2 size = display_rect.Size();
            if (c != ' ')
            {
                const Math::ivec2 top_left_texel = { display_rect.point_1.x, display_rect.point_2.y };
                const Math::TransformationMatrix model = S * pen;
                texture->Draw(model, top_left_texel, size, color);
            }
            pen *= Math::TranslationMatrix(Math::ivec2{ size.x, 0 });
        }

        renderer.EndScene();
        auto result_texture = texture_manager.EndRenderTextureMode();
        renderer.BeginScene(old_camera);

        restore_gl_state(prevViewport, prevBlend);

        return result_texture;
    }

    unsigned int Font::GetPixel(const GAME200::Image& image, Math::ivec2 texel)
    {
        const Math::ivec2 dim = image.GetSize();
        if (texel.x < 0 || texel.y < 0 || texel.x >= dim.x || texel.y >= dim.y)
        {
            return 0u;
        }
        const GAME200::RGBA* pixels = image.data();
        const GAME200::RGBA rgba = pixels[texel.y * dim.x + texel.x];
        return static_cast<unsigned int>(rgba);
    }

    Math::ivec2 Font::MeasureText(const std::string& text) const
    {
        Math::ivec2 out = { 0, 0 };
        for (char ch : text)
        {
            const Math::ivec2 size = GetCharRect(ch).Size();
            out.x += size.x;
            if (out.y < size.y)
            {
                out.y = size.y;
            }
        }
        return out;
    }

}