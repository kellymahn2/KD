#pragma once

#include "Kaidel/Renderer/Framebuffer.h"
#include "Kaidel/Renderer/Material.h"
#include "Kaidel/Mesh/Model.h"

#include <glm/glm.hpp>
namespace Kaidel {

	struct Renderer3DBeginData {
		glm::mat4 CameraVP;
		glm::vec3 CameraPosition;
		Ref<Framebuffer> OutputBuffer;
	};

	class Renderer3D {
	public:
		static void Init();
		static void Shutdown();
		static void Begin(const Renderer3DBeginData& beginData);

		static void DrawCube(const glm::mat4& transform, Ref<Material> material);
		static void DrawModel(const glm::mat4& transform, Ref<Model> model,Ref<Material> material);
		
		static void End();
	private:
		static void RenderingPipeLine();
		friend struct Renderer3DData;
	};
}
