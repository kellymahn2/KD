#pragma once
#include <glm/glm.hpp>
namespace Kaidel {
	struct CubeShadowVertex;

	class ShadowRenderer{
	public:
		ShadowRenderer();
		void BeginRendering(int lightIndex);
		void DrawCube(const glm::mat4& transform);
		void EndRendering();
	private:
		void StartBatch();
		static void StartCubeBatch();
		static void FlushCubes(CubeShadowVertex* start, uint64_t size);
		void Flush();
		friend struct ShadowRendererData;
	};
}
