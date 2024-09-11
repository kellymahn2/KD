#pragma once
#include "Kaidel\Core\JobSystem.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "entt.hpp"
#include <glm/glm.hpp>
#include <queue>
namespace Kaidel {
	extern TextureSamples samples;
	class Entity;

	class SceneRenderer {
	public:
		SceneRenderer() = default;
		SceneRenderer(void* scene);
		void Reset();
		void Render(Ref<Texture2D> outputBuffer, const glm::mat4& cameraViewProj, const glm::vec3& cameraPos);
		SceneRenderer& operator=(void* scene) {
			m_Context = scene;
			Reset();
			return *this;
		}
	private:
		void* m_Context;
	};
}
