#include "Image.h"
#include "../Engine/Path.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace GAME200
{
    Image::Image(const std::filesystem::path& image_path, bool flip_vertical)
    {
        stbi_set_flip_vertically_on_load(flip_vertical);

        int width = 0, height = 0;
        const auto located = assets::locate_asset(image_path).string();
        pixels = reinterpret_cast<RGBA*>(stbi_load(located.c_str(), &width, &height, nullptr, 4));

        if (!pixels || width <= 0 || height <= 0)
        {
            throw std::runtime_error("Failed to load image: " + image_path.string());
        }
        dimensions = { width, height };
    }

    Image::~Image()
    {
        if (pixels != nullptr)
        {
            stbi_image_free(pixels);
            pixels = nullptr;
        }
    }

    Image::Image(Image&& temporary) noexcept
        : pixels(temporary.pixels), dimensions(temporary.dimensions)
    {
        temporary.pixels = nullptr;
        temporary.dimensions = { 0, 0 };
    }

    Image& Image::operator=(Image&& temporary) noexcept
    {
        std::swap(dimensions, temporary.dimensions);
        std::swap(pixels, temporary.pixels);
        return *this;
    }

    const RGBA* Image::data() const noexcept
    {
        return pixels;
    }

    RGBA* Image::data() noexcept
    {
        return pixels;
    }

    Math::ivec2 Image::GetSize() const noexcept
    {
        return dimensions;
    }

}
