#pragma once

#include "../Engine/Vec2.h"
#include "RGBA.h"
#include <filesystem>
#include <gsl/gsl>

namespace GAME200
{
    class Image
    {
    public:
        explicit Image(const std::filesystem::path& image_path, bool flip_vertical = false);

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&& temporary) noexcept;

        Image& operator=(Image&& temporary) noexcept;

        ~Image();

        [[nodiscard]] const RGBA* data() const noexcept;

        [[nodiscard]] RGBA* data() noexcept;

        [[nodiscard]] Math::ivec2 GetSize() const noexcept;

    private:
        Math::ivec2 dimensions{ 0, 0 };
        RGBA* pixels = nullptr;
    };

}
