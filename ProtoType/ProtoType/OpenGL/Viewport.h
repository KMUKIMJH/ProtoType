#pragma once

#include "../Engine/Matrix.h"
#include "../Engine/Vec2.h"

namespace GAME200
{
    inline Math::TransformationMatrix build_viewport_matrix(Math::ivec2 view_size) noexcept
    {
        if (view_size.x <= 0 || view_size.y <= 0)
        {
            Math::TransformationMatrix id{}; 
            id.Reset();
            return id;
        }

        const double sx = 2.0 / static_cast<double>(view_size.x);
        const double sy = 2.0 / static_cast<double>(view_size.y);

        return Math::TranslationMatrix(Math::vec2{ -1.0, -1.0 }) * Math::ScaleMatrix(Math::vec2{ sx, sy });
    }
}
