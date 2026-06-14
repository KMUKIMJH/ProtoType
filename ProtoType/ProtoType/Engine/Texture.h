#pragma once
#include "Matrix.h"
#include "Vec2.h"
#include <filesystem>

namespace CS230
{
    class Font;


    class Texture
    {
    public:
        friend class TextureManager;
        friend class Font;

        void Draw(const Math::TransformationMatrix& display_matrix, unsigned int color = 0xFFFFFFFF);


        void Draw(const Math::TransformationMatrix& display_matrix, Math::ivec2 texel_position, Math::ivec2 frame_size, unsigned int color = 0xFFFFFFFF);

        void Bind() const;

        Math::ivec2 GetSize() const;


        ~Texture();

        [[nodiscard]] unsigned int GetHandle() const
        {
            return textureHandle;
        }

    private:
        explicit Texture(const std::filesystem::path& file_name);
        Texture(unsigned int given_texture, Math::ivec2 the_size);

    public:

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        Texture(Texture&& temporary) noexcept;


        Texture& operator=(Texture&& temporary) noexcept;


    private:
        unsigned int textureHandle{ 0 };
        Math::ivec2 size{ 0, 0 };
    };
}