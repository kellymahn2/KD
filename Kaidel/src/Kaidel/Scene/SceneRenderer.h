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
	private:
		uint64_t m_QuadInsertIndex = 0;
		uint64_t m_CircleInsertIndex = 0;
		uint64_t m_LineInsertIndex = 0;
		void* m_Context;
		JobSystem m_JobSystem = JobSystem(4);
	};
}
