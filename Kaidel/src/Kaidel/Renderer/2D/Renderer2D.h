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
		//X:Increment, Y:Start, Z:End, W:Index
		glm::vec4 PatchDef;
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
		static void DrawBezier(std::vector<glm::vec3> points, glm::vec4 color, float increment = 0.001);
		static void End();
	private:
		static void FlushSprites();
		static void FlushLines();
		friend struct SpriteRendererData;
		friend struct LineRendererData;
	};
}
