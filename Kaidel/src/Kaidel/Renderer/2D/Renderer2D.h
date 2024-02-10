#pragma once
#include "Kaidel/Renderer/Framebuffer.h"
#include "Kaidel/Renderer/Material2D.h"
#include <glm/glm.hpp>

namespace Kaidel {

	struct SpriteVertex {
		glm::vec3 Position;
		glm::vec2 TexCoords;
		int32_t MaterialID;
	};

	struct LineVertex {
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct BezierVertex {
		glm::vec3 Position;
	};

	struct PointVertex {
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct Renderer2DBeginData {
		glm::mat4 CameraVP;
		Ref<Framebuffer> OutputBuffer;
	};

	class Renderer2D {
	public:
		static void Init();
		static void Shutdown();
		static void Begin(const Renderer2DBeginData& beginData);
		static void DrawSprite(const glm::mat4& transform, Ref<Material2D> material);
		static void DrawBezier(const glm::mat4& transform,const std::vector<glm::vec3>& points, const glm::vec4& color, float increment = 0.001);
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color);
		static void DrawPoint(const glm::vec3& position, const glm::vec4& color);
		static void End();
	private:
		static void FlushSprites();
		static void FlushLines();
		static void FlushPoints();
		friend struct SpriteRendererData;
		friend struct LineRendererData;
		friend struct PointRendererData;

	};
}
