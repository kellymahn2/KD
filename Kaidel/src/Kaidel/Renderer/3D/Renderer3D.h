#pragma once

#include "Kaidel/Renderer/Framebuffer.h"
#include "Kaidel/Renderer/Material.h"
#include "Kaidel/Mesh/Model.h"
#include "Kaidel/Assets/Asset.h"
#include <glm/glm.hpp>
namespace Kaidel {

	struct Renderer3DBeginData {
		glm::mat4 CameraVP;
		glm::vec3 CameraPosition;
		Ref<Framebuffer> OutputBuffer;
	};

	struct Renderer3DStats {
		uint64_t GeometryPassDrawCount = 0;
		uint64_t ShadowPassDrawCount = 0;
		uint64_t PushCount = 0;
	};


	class Renderer3D {
	public:
		static void Init();
		static void Shutdown();
		static void Begin(const Renderer3DBeginData& beginData);

		static void DrawMesh(const glm::mat4& transform, AssetHandle<Mesh> mesh, Ref<Material> material);
		static void End();

		static void RenderingPipeLine();
		static void ResetStats();
		static Renderer3DStats& GetStats();
	private:
		static void SetupPrimitives();
		static void FlushMesh(AssetHandle<Mesh>  mesh);
		friend struct Renderer3DData;
	};
}
