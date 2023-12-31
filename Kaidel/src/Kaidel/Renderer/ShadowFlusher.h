#pragma once

#include "Renderer3DFlusher.h"


namespace Kaidel {
	class ShadowFlusher : public Renderer3DFlusher {
	public:
		ShadowFlusher(const glm::mat4& viewProjectionMatrix, const glm::vec3& viewPosition);
		~ShadowFlusher();
		virtual void Begin();
		virtual void End();
		virtual void Flush();
		virtual void PushCubeVertex(CubeVertex* begin, CubeVertex* end);
	private:
		static void FlushCubes(CubeVertex* start, uint64_t size);
		static void StartCubeBatch();
		static void StartBatch();
		friend struct ShadowFlusherData;
	};
}
