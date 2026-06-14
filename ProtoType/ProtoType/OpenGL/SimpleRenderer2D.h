#pragma once

#include "Canvas2D.h"
#include "../Engine/Matrix.h"
#include "../Engine/Vec2.h"
#include <vector>
#include <cstdint>

namespace GAME200
{
	class SimpleRenderer2D final : public ICanvas2D
	{
	public:
		SimpleRenderer2D() = default;
		SimpleRenderer2D(const SimpleRenderer2D& other) = delete;
		SimpleRenderer2D(SimpleRenderer2D&& other) noexcept;
		SimpleRenderer2D& operator=(const SimpleRenderer2D& other) = delete;
		SimpleRenderer2D& operator=(SimpleRenderer2D&& other) noexcept;
		~SimpleRenderer2D() override;

		void Init() override;
		void Shutdown() override;

		void BeginScene(const Math::TransformationMatrix& view_projection) override;
		void EndScene() override;

		void DrawQuad(const Math::TransformationMatrix& transform, unsigned int texture, Math::vec2 texture_coord_bl, Math::vec2 texture_coord_tr, GAME200::RGBA tintColor) override;
		void DrawCircle(const Math::TransformationMatrix& transform, GAME200::RGBA fill_color, GAME200::RGBA line_color, double line_width) override;
		void DrawRectangle(const Math::TransformationMatrix& transform, GAME200::RGBA fill_color, GAME200::RGBA line_color, double line_width) override;
		void DrawLine(const Math::TransformationMatrix& transform, Math::vec2 start_point, Math::vec2 end_point, GAME200::RGBA line_color, double line_width = 1.0);

		[[nodiscard]] Math::TransformationMatrix GetViewProjectionMatrix() const { return viewProjectionMatrix; }

	private:
		enum class SDFShape : uint8_t { Circle = 0, Rectangle = 1 };

		void DrawSDF(const Math::TransformationMatrix& transform, GAME200::RGBA fill_color, GAME200::RGBA line_color, double line_width, SDFShape shape);
		void startBatch();
		void flush();

		struct QuadInstance
		{
			float transformRow0[3]; 
			float transformRow1[3]; 
			unsigned char tint[4]; 
			float texScale[2]; 
			float texOffset[2]; 
			int textureIndex = 0;
		};

		unsigned int fixedVBO = 0;
		unsigned int instanceVBO = 0; 
		unsigned int indexBuffer = 0;
		unsigned int quadVAO = 0;
		unsigned int quadShader = 0;
		unsigned int cameraUBO = 0; 

		unsigned int sdfVAO = 0;
		unsigned int sdfVBO = 0;
		unsigned int sdfShader = 0;
		int sdf_u_model_matrix = -1;
		int sdf_u_shapeSize = -1;
		int sdf_u_size = -1; 
		int sdf_u_fillColor = -1;
		int sdf_u_lineColor = -1;
		int sdf_u_lineWidth = -1;
		int sdf_u_shapeType = -1;
		int sdf_u_worldPx = -1; 

		std::vector<QuadInstance> instances;
		std::vector<unsigned int> textureSlots;
		size_t activeTextureCount = 0;
		unsigned int maxInstances = 10000;

		unsigned int drawCallCount = 0;
		unsigned int flushCount = 0;
		unsigned int submittedInstanceTotal = 0;

		Math::TransformationMatrix viewProjectionMatrix;
	};
}
