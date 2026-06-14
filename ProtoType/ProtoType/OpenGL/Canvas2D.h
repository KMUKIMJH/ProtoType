#pragma once

#include "../Engine/Matrix.h"
#include "../Engine/Vec2.h"
#include "RGBA.h"

namespace GAME200
{
	class ICanvas2D
	{
	public:
		virtual ~ICanvas2D() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginScene(const Math::TransformationMatrix& view_projection) = 0;
		virtual void EndScene() = 0;

		[[nodiscard]] virtual Math::TransformationMatrix GetViewProjectionMatrix() const = 0;

		virtual void DrawQuad(const Math::TransformationMatrix& transform, unsigned int texture, Math::vec2 texture_coord_bl, Math::vec2 texture_coord_tr, GAME200::RGBA tintColor) = 0;
		virtual void DrawCircle(const Math::TransformationMatrix& transform, GAME200::RGBA fill_color, GAME200::RGBA line_color, double line_width) = 0;
		virtual void DrawRectangle(const Math::TransformationMatrix& transform, GAME200::RGBA fill_color, GAME200::RGBA line_color, double line_width) = 0;
		virtual void DrawLine(const Math::TransformationMatrix& transform, Math::vec2 start_point, Math::vec2 end_point, GAME200::RGBA line_color, double line_width = 1.0) = 0;
	};
}