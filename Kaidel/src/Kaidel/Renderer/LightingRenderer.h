#pragma once
#include "Material.h"

#include <glm/glm.hpp>


namespace Kaidel {
	struct CubeLightingVertex;
	class LightingRenderer {
	public:
		LightingRenderer();
		void BeginRendering();
		void DrawCube(const glm::mat4& transform,uint32_t materialIndex);
		void EndRendering();
	private:
		void StartBatch();
		static void StartCubeBatch();
		static void FlushCubes(CubeLightingVertex* start,uint64_t size);
		void Flush();
		friend struct LightingRendererData;
	};
}
