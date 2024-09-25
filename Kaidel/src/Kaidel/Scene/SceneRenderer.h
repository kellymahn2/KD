#pragma once
#include "Kaidel\Core\JobSystem.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "entt.hpp"
#include <glm/glm.hpp>
#include <queue>
namespace Kaidel {
	extern TextureSamples samples;
	class Entity;

	struct SceneData {
		glm::mat4 View;
		glm::mat4 Proj;
		glm::mat4 ViewProj;
		glm::vec3 CameraPos;
		glm::uvec2 ScreenSize;
		float zNear, zFar;
	};

	extern glm::vec3 col,dir;

	class SceneRenderer {
	public:
		SceneRenderer() = default;
		SceneRenderer(void* scene);
		void Reset();
		void Render(Ref<Texture2D> outputBuffer, const SceneData& sceneData);
		SceneRenderer& operator=(void* scene) {
			m_Context = scene;
			Reset();
			return *this;
		}
	private:

		bool NeedsRecreation(Ref<Texture2D> output);
		void RecreateSizeOrSampleDependent();
		void ResolveToOutput(Ref<Texture2D> outputBuffer);
		void ScreenPass(float zNear, float zFar, const glm::vec3& cameraPos);
		void InsertScreenPassBarrier();
		void MakeClusters(const glm::mat4& invProj, float zNear, float zFar, const glm::vec2& screenSize);
		void InsertClusterBarrier();
		void MakeLightGrids(const glm::mat4& view);
		void InsertLightGridBarrier();
	private:
		void* m_Context;
	};
}
