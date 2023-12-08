#pragma once
#include <queue>
#include "entt.hpp"
#include "Kaidel\Core\JobSystem.h"
namespace Kaidel {

	class Entity;

	class SceneRenderer {
	public:
		SceneRenderer() = default;
		SceneRenderer(void* scene);
		void Reset();
		void Render();
		SceneRenderer& operator=(void* scene) {
			m_Context = scene;
			Reset();
			return *this;
		}
	private:
		void Bind();
		void DrawQuads();
		void DrawCircles();
		void DrawLines();
		void DrawCubes();
		uint64_t m_QuadInsertIndex = 0;
		uint64_t m_CircleInsertIndex = 0;
		uint64_t m_LineInsertIndex = 0;
		void* m_Context;
		JobSystem m_JobSystem = JobSystem(4);
	};
}
