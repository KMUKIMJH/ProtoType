#pragma once

#include "../Engine/Matrix.h"
#include "../Engine/Vec2.h"
#include <array>

namespace GAME200::DrawUtils2D
{
    [[nodiscard]] Math::TransformationMatrix CalculateLineTransform(const Math::TransformationMatrix& transform, const Math::vec2& start_point, const Math::vec2& end_point, double line_width) noexcept;
}
