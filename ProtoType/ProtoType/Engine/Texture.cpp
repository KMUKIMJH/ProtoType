#include <GL/glew.h>
#include "Texture.h"

#include "../OpenGL/Canvas2D.h"
#include "../OpenGL/Image.h"
#include "Engine.h"
#include <utility>

namespace CS230
{
    Texture::Texture(const std::filesystem::path& file_name)
    {
        GAME200::Image image(file_name, false);
        size = image.GetSize();

        glGenTextures(1, &textureHandle);
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    Texture::Texture(unsigned int given_texture, Math::ivec2 the_size)
        : textureHandle(given_texture), size(the_size)
    {
    }

    Texture::~Texture()
    {
        if (textureHandle != 0)
        {
            glDeleteTextures(1, &textureHandle);
        }
    }

    Texture::Texture(Texture&& temporary) noexcept
        : textureHandle(temporary.textureHandle), size(temporary.size)
    {
        temporary.textureHandle = 0;
        temporary.size = { 0, 0 };
    }

    Texture& Texture::operator=(Texture&& temporary) noexcept
    {
        std::swap(textureHandle, temporary.textureHandle);
        std::swap(size, temporary.size);
        return *this;
    }

    Math::ivec2 Texture::GetSize() const
    {
        return size;
    }

    void Texture::Bind() const
    {
        glBindTexture(GL_TEXTURE_2D, textureHandle);
    }

    void Texture::Draw(const Math::TransformationMatrix& model_matrix, unsigned int color)
    {
        Draw(model_matrix, { 0, 0 }, GetSize(), color);
    }

    void Texture::Draw(const Math::TransformationMatrix& model_matrix, Math::ivec2 texel_position, Math::ivec2 frame_size, unsigned int color)
    {
        const Math::TransformationMatrix pixel_model =
            model_matrix *
            Math::TranslationMatrix(Math::vec2{ static_cast<double>(frame_size.x) * 0.5, static_cast<double>(frame_size.y) * 0.5 }) *
            Math::ScaleMatrix(Math::vec2{ static_cast<double>(frame_size.x), static_cast<double>(frame_size.y) });

        const double W = static_cast<double>(GetSize().x);
        const double H = static_cast<double>(GetSize().y);
        const double halfTexelU = 0.5 / W;
        const double halfTexelV = 0.5 / H;

        const double u0 = (static_cast<double>(texel_position.x) + halfTexelU) / W;
        const double v1 = (static_cast<double>(texel_position.y) + halfTexelV) / H;
        const double u1 = (static_cast<double>(texel_position.x + frame_size.x) - halfTexelU) / W;
        const double v0 = (static_cast<double>(texel_position.y + frame_size.y) - halfTexelV) / H;

        const Math::vec2 uv_bottom_left(u0, v0);
        const Math::vec2 uv_top_right(u1, v1);

        Engine::GetRenderer2D().DrawQuad(pixel_model, GetHandle(), uv_bottom_left, uv_top_right, color);
    }
}