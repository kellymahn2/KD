#pragma once
#include "Renderer3DFlusher.h"
namespace Kaidel {
	class GeometryFlusher : public Renderer3DFlusher {
	public:
		GeometryFlusher(const glm::mat4& viewProjectionMatrix, const glm::vec3& viewPosition);
		~GeometryFlusher();
		virtual void Begin();
		virtual void End();
		virtual void Flush();
	private:
		static void FlushCubes(CubeVertex* start, uint64_t size);
		static void StartCubeBatch();
		static void StartBatch();
		friend struct GeometryFlusherData;
	};
}
