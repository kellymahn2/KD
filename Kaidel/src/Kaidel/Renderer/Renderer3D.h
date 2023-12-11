#pragma once
#include "Kaidel/Renderer/OrthographicCamera.h"
#include "KDpch.h"
#include "Kaidel/Renderer/Texture.h"

#include "Kaidel/Renderer/Camera.h"
#include "Kaidel/Renderer/EditorCamera.h"

#include "Kaidel/Scene/Components.h"
#include <future>

namespace Kaidel {
	struct CubeVertex;
	class Renderer3D {
	public:
		static void Init();
		static void Shutdown();
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera); // TODO: Remove
		static void EndScene();
		static void Flush();
		static void FlushCubes(CubeVertex* start, uint64_t size);

		//Primitives
		static void DrawCube(const glm::mat4& transform, Ref<Material> material, int entityID = -1);
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t CubeCount = 0;
			uint32_t GetTotalVertexCount() const { return CubeCount * 8; }
			uint32_t GetTotalIndexCount() const { return CubeCount * 36; }
		};
		static void ResetStats();
		static Statistics GetStats();
	private:
		static void StartBatch();
		static void StartCubeBatch();
		static void NextBatch();
		friend struct Renderer3DData;
	};
}
