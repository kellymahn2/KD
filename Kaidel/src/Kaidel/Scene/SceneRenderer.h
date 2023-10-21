#pragma once
#include <queue>
#include "entt.hpp"
namespace Kaidel {

	class Entity;

	class SceneRenderer {
	public:
		SceneRenderer() = default;
		SceneRenderer(void* scene);
		void Reset();
		void Render();
	private:
		uint64_t m_InsertIndex = 0;
		void* m_Context;
	};
}
