#pragma once

#include "GraphicsAPI/Texture.h"
#include "GraphicsAPI/RenderPass.h"
#include "GraphicsAPI/DescriptorSet.h"
#include "GraphicsAPI/GraphicsPipeline.h"
#include "GraphicsAPI/PerFrameResource.h"
#include "GraphicsAPI/Framebuffer.h"
#include "Kaidel/Scene/Model.h"
#include "Kaidel/Renderer/EnvironmentMap.h"
#include "Kaidel/Scene/Visibility.h"
#include "Kaidel/Core/Application.h"

#include <glm/glm.hpp>

namespace Kaidel
{
	struct DirectionalLightData {
		glm::mat4 ViewProjection[4];
		glm::vec4 Direction;
		glm::vec4 Color;
		glm::vec4 SplitDistances;
		float FadeStart;
		float LightSize;
	};

	struct SceneData {
		glm::mat4 View;
		glm::mat4 Proj;
		glm::mat4 ViewProj;
		glm::vec4 CameraPos;
		glm::uvec2 ScreenSize;
		float zNear, zFar;
		float FOV;
		float AspectRatio;
		float Time;
		float DeltaTime;

		SceneData() {
			Time = Application::Get().GetRunningTime();
			DeltaTime = Application::Get().GetDeltaTime();
		}
	};

	struct Renderer3DStats {
		uint64_t DrawCalls = 0;
		uint64_t InstanceCount = 0;
		uint64_t VertexCount = 0;
		uint64_t IndexCount = 0;
		uint64_t ShadowVertexCount = 0;
		uint64_t ShadowIndexCount = 0;
	};

	enum class Renderer3DState
	{
		None = 0,
		Shadow,
		Color
	};

	struct Renderer3DRenderParams
	{
		Ref<VertexBuffer> VB;
		Ref<IndexBuffer> IB;
		uint32_t VertexOffset = 0;
		uint32_t IndexOffset = 0;
		uint32_t InstanceCount = 1;
		uint32_t VertexCount;
		uint32_t IndexCount;
		uint32_t InstanceOffset = 0;

		bool operator==(const Renderer3DRenderParams& rhs) const
		{
			return VB == rhs.VB &&
				IB == rhs.IB;
		}

		bool operator!=(const Renderer3DRenderParams& rhs) const
		{
			return !(*this == rhs);
		}
	};

	class Renderer3D
	{
	public:

		static void Init();
		static void Shutdown();

		static void Begin(Ref<Texture2D> output, 
			const Visibility& vis,
			const SceneData& sceneData,
			const DirectionalLightData& lightData,
			const std::string& viewportName = "Main");
		
		static void BeginShadow(const DirectionalLightData& lightData);

		static void BeginDirectionalShadow(uint32_t cascade);
		
		static void BeginColor(Ref<EnvironmentMap> environment);

		static InstanceData* ReserveInstanceData(uint64_t instanceCount);

		static void Draw(const Renderer3DRenderParams& params, Ref<MaterialInstance> material);
		
		static void BeginSubmesh();
		static void DrawSubmesh(const Renderer3DRenderParams& params, Ref<MaterialInstance> material);
		static void EndSubmesh();

		static void Flush();
		
		static void EndColor();

		static void EndShadow();

		static void End();

		static Ref<RenderPass> GetDeferredPassRenderPass();

		static Renderer3DStats GetStats();

	private:
		static void UploadInstanceData(const std::vector<InstanceData>& instances, Ref<StorageBuffer> buffer, Ref<DescriptorSet> set);

		static void CreatePerViewportData(const std::string& viewportName, uint32_t viewportWidth, uint32_t viewportHeight);
		static void CalculateDirectionalLightData();

		static void DrawBatch(bool increment);
	private:
		static bool NeedsRecreation(Ref<Texture2D> output, const std::string& viewportName);
	};
}
