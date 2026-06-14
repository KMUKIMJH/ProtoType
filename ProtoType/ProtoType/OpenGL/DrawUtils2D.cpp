
#include "DrawUtils2D.h"

#include <algorithm>
#include <cmath>

namespace GAME200::DrawUtils2D
{
	Math::TransformationMatrix CalculateLineTransform(const Math::TransformationMatrix& transform, const Math::vec2& start_point, const Math::vec2& end_point, double line_width) noexcept
	{
		const Math::vec2 delta = end_point - start_point;
		const double     angle = std::atan2(delta.y, delta.x);
		const Math::vec2 center = { (start_point.x + end_point.x) * 0.5, (start_point.y + end_point.y) * 0.5 };
		const double     length = std::hypot(delta.x, delta.y);

		const Math::ScaleMatrix       scale_matrix({ length, line_width });
		const Math::RotationMatrix    rotation_matrix(angle);
		const Math::TranslationMatrix translation_matrix(center);
		return transform * translation_matrix * rotation_matrix * scale_matrix;
	}
}
