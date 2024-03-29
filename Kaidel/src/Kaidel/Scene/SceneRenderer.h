#pragma once
#include "Kaidel\Core\JobSystem.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Mesh/Mesh.h"
#include "entt.hpp"
#include <glm/glm.hpp>
#include <queue>
namespace Kaidel {

	class Entity;

	class SceneRenderer {
	public:
		SceneRenderer() = default;
		SceneRenderer(void* scene);
		void Reset();
		void Render(Ref<Framebuffer> _3DOutputFramebuffer, Ref<Framebuffer> _2DOutputFramebuffer, const glm::mat4& cameraViewProj, const glm::vec3& cameraPos);
		SceneRenderer& operator=(void* scene) {
			m_Context = scene;
			Reset();
			return *this;
		}
	private:
		void RenderMesh(Ref<Mesh> mesh);
		void* m_Context;
	};
}
