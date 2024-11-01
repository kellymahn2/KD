#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"

#include "Kaidel/Renderer/GraphicsAPI/PerFrameResource.h"

#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Renderer/DescriptorSetPack.h"
#include "Kaidel/Scene/Material.h"
#include "Kaidel/Scene/Model.h"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>

#include "stb_image.h"
#define MAX_LIGHT_COUNT 100

#define SHADOW_MAP_SIZE 4096
#define SHADOW_NEAR 1.0f
#define SHADOW_FAR 1000.0f
#define SHADOW_MAPPING
#define TYPE 2

namespace Kaidel {
	struct DeferredPassData {
		Ref<RenderPass> RenderPass;
		PerFrameResource<Ref<Framebuffer>> Output;
		//Ref<GraphicsPipeline> Pipeline;
		DescriptorSetPack Pack;
	};

#ifdef CSM
	struct ShadowData {
		Ref<RenderPass> RenderPass;
		PerFrameResource<Ref<Framebuffer>> Buffers[4];
		Ref<GraphicsPipeline> Pipeline;
	};

#endif

#ifdef SHADOW_MAPPING
	struct ShadowData {
		Ref<RenderPass> RenderPass;
		Ref<GraphicsPipeline> Pipeline;
		PerFrameResource<Ref<Framebuffer>> Framebuffer[4];
		Ref<Sampler> ShadowSampler;
	};
#endif
	static struct Data {
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<IndexBuffer> CubeIndexBuffer;
		Ref<ComputePipeline> ClusterPipeline;
		PerFrameResource<DescriptorSetPack> ClusterPipelinePack;
		Ref<ComputePipeline> LightCullPipeline;
		PerFrameResource<DescriptorSetPack> LightCullPipelinePack;

		//PerFrameResource<Ref<UniformBuffer>> CameraBuffers;
		//PerFrameResource<Ref<DescriptorSet>> CameraBufferSets;

		PerFrameResource<Ref<StorageBuffer>> Clusters;
		PerFrameResource<Ref<StorageBuffer>> ClusterGrids;

		DeferredPassData DeferredPass;
		
		Ref<StorageBuffer> Instances;

		Ref<DescriptorSet> TransformsSet;

		//Screen pass
		Ref<VertexBuffer> ScreenNDC;
		Ref<RenderPass> ScreenRenderPass;
		PerFrameResource<Ref<Framebuffer>> HDROutputs;
		Ref<GraphicsPipeline> ScreenPipeline;
		PerFrameResource<DescriptorSetPack> ScreenPipelinePack;

		//tonemap pass
		Ref<RenderPass> TonemapRenderPass;
		PerFrameResource<Ref<Framebuffer>> Outputs;
		Ref<GraphicsPipeline> TonemapPipeline;
		PerFrameResource<DescriptorSetPack> TonemapPipelinePack;

		//Material
		Ref<Texture2D> Albedo;
		Ref<Texture2D> Spec;
		Ref<Sampler> GBufferSampler;
		Ref<DescriptorSet> MaterialSet;

		Ref<Texture2D> DefaultWhite;

		PerFrameResource<Ref<UniformBuffer>> DirectionalLight;
		PerFrameResource<Ref<DescriptorSet>> DirectionalLightSet;

		ShadowData Shadows;
		Ref<Sampler> GlobalSampler;
		uint32_t Width = 1280, Height = 720;
	}*s_Data;

	Ref<RenderPass> GetDeferredPassRenderPass() {
		SceneRenderer r(nullptr);
		return s_Data->DeferredPass.RenderPass;
	}

	TextureSamples samples = TextureSamples::x1;

	struct Light {
		glm::vec4 Position;
		glm::vec4 Color;
		float Radius;
	};

	std::vector<Light> lights;

	struct DirectionalLight {
		glm::mat4 ViewProj[4];
		alignas(16) glm::vec3 Direction;
		alignas(16) glm::vec3 Color;
		alignas(16) float SplitDistances[4];
	} DLight;

	struct Cluster {
		glm::vec4 Min;
		glm::vec4 Max;
	};

	struct ClusterGrid {
		uint32_t Count;
		uint32_t Indices[50];
		uint32_t Padding;
	};

	static glm::uvec3 s_ClusterDimensions = { 16,9,24 };
	static uint64_t s_ClusterGridLength = s_ClusterDimensions.x * s_ClusterDimensions.y * s_ClusterDimensions.z;

	std::vector<glm::mat4> cubes;

	#pragma region Light Culling
	static void CreateLightCullResources() {

		s_Data->LightCullPipeline = ComputePipeline::Create(ShaderLibrary::LoadShader("LightCull", "assets/_shaders/ClusterCullComp.shader"));

		for (auto& grid : s_Data->ClusterGrids) {
			grid = StorageBuffer::Create(s_ClusterGridLength * sizeof(ClusterGrid));
		}

		uint32_t i = 0;
		for (auto& pack : s_Data->LightCullPipelinePack) {
			pack =
				DescriptorSetPack(ShaderLibrary::GetNamedShader("LightCull"), 
					{ {0,s_Data->ClusterPipelinePack.GetResources()[i].GetSet(0)} });
			pack.GetSet(1)->Update(StorageBuffer::Create(MAX_LIGHT_COUNT * sizeof(Light)),0)
				.Update(s_Data->ClusterGrids.GetResources()[i],1);
			++i;
		}
	}
	
	void SceneRenderer::MakeLightGrids(const glm::mat4& view) {
		s_Data->LightCullPipelinePack->GetSet(1)->GetStorageBufferAtBinding(0)->SetData(lights.data(), lights.size() * sizeof(Light));

		RenderCommand::BindComputePipeline(s_Data->LightCullPipeline);
		s_Data->LightCullPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightCull"), 0,
			view, lights.size());
		RenderCommand::Dispatch(1, 1, 6);
	}
	
	void SceneRenderer::InsertLightGridBarrier() {
		BufferMemoryBarrier barrier{};
		barrier.Buffer = s_Data->LightCullPipelinePack->GetSet(1)->GetStorageBufferAtBinding(1);
		barrier.Offset = 0;
		barrier.Size = -1;
		barrier.Src = AccessFlags_ShaderWrite;
		barrier.Dst = AccessFlags_ShaderRead;
		RenderCommand::PipelineBarrier(
			PipelineStages_ComputeShader,
			PipelineStages_FragmentShader,
			{},
			{ barrier },
			{}
		);
	}
	#pragma endregion

	#pragma region Clusters
	static void CreateClusterResources() {
		s_Data->ClusterPipeline = ComputePipeline::Create(ShaderLibrary::LoadShader("Cluster", "assets/_shaders/ClusterComp.shader"));
		for (auto& cluster : s_Data->Clusters) {
			cluster =
				StorageBuffer::Create(s_ClusterGridLength * sizeof(Cluster));
		}

		uint32_t i = 0;
		for (auto& pack : s_Data->ClusterPipelinePack) {
			pack =
				DescriptorSetPack(ShaderLibrary::GetNamedShader("Cluster"), {}); 
			pack.GetSet(0)->Update(s_Data->Clusters.GetResources()[i],0);
			++i;
		}
	}

	void SceneRenderer::MakeClusters(const glm::mat4& invProj, float zNear, float zFar, const glm::vec2& screenSize) {
		RenderCommand::BindComputePipeline(s_Data->ClusterPipeline);
		s_Data->ClusterPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("Cluster"), 0,
			invProj, zNear, zFar, screenSize);
		RenderCommand::Dispatch(s_ClusterDimensions.x, s_ClusterDimensions.y, s_ClusterDimensions.z);
	}
	
	void SceneRenderer::InsertClusterBarrier() {
		BufferMemoryBarrier barrier{};
		barrier.Buffer = *s_Data->Clusters;
		barrier.Offset = 0;
		barrier.Size = -1;
		barrier.Src = AccessFlags_ShaderWrite;
		barrier.Dst = AccessFlags_ShaderRead;
		RenderCommand::PipelineBarrier(
			PipelineStages_ComputeShader,
			PipelineStages_ComputeShader,
			{},
			{ barrier },
			{}
		);
	}
	#pragma endregion

	#pragma region Screen Pass

	static void CreateScreenPassResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA16F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,TextureSamples::x1,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
			};
			s_Data->ScreenRenderPass = RenderPass::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			for (auto& fb : s_Data->HDROutputs) {
				fb = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs{};
			specs.Input.Bindings = {
				{
					{"a_NDC",Format::RG32F},
					{"a_TexCoords",Format::RG32F}
				}
			};
			specs.Multisample.Samples = TextureSamples::x1;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Subpass = 0;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("LightPass", "assets/_shaders/CubeLightPass.shader");
			s_Data->ScreenPipeline = GraphicsPipeline::Create(specs);
		}

		{
			SamplerState state;
			state.AddressModeU = SamplerAddressMode::ClampToBorder;
			state.AddressModeV = SamplerAddressMode::ClampToBorder;
			state.AddressModeW = SamplerAddressMode::ClampToBorder;
			state.BorderColor = SamplerBorderColor::FloatOpaqueWhite;
			state.MagFilter = SamplerFilter::Nearest;
			state.MinFilter = SamplerFilter::Nearest;
			state.MipFilter = SamplerMipMapMode::Nearest;

			s_Data->Shadows.ShadowSampler = Sampler::Create(state);
		}

		s_Data->DirectionalLight.Construct([](uint32_t i) {return UniformBuffer::Create(sizeof(DirectionalLight)); });

		s_Data->ScreenPipelinePack.Construct([](uint32_t i) {
			Ref<Texture2D> pos, norm, albedo, metalRough, depth;
			
			if (samples == TextureSamples::x1) {
				pos = s_Data->DeferredPass.Output[i]->GetColorAttachment(0);
				norm = s_Data->DeferredPass.Output[i]->GetColorAttachment(1);
				albedo = s_Data->DeferredPass.Output[i]->GetColorAttachment(2);
				metalRough = s_Data->DeferredPass.Output[i]->GetColorAttachment(3);
			}
			else {
				pos = s_Data->DeferredPass.Output[i]->GetResolveAttachment(0);
				norm = s_Data->DeferredPass.Output[i]->GetResolveAttachment(1);
				albedo = s_Data->DeferredPass.Output[i]->GetResolveAttachment(2);
				metalRough = s_Data->DeferredPass.Output[i]->GetResolveAttachment(3);
			}
			depth = s_Data->DeferredPass.Output[i]->GetDepthAttachment();
			
			DescriptorSetPack pack = DescriptorSetPack(ShaderLibrary::GetNamedShader("LightPass"), {});
			pack[0]->Update(s_Data->Clusters[i], 0);
			pack[1]->Update(s_Data->LightCullPipelinePack[i].GetSet(1)->GetStorageBufferAtBinding(0), 0).
				Update(s_Data->ClusterGrids[i], 1);
			//sampler
			pack[2]->Update({}, s_Data->GlobalSampler, {}, 0).
				//positions
				Update(pos, {}, ImageLayout::ShaderReadOnlyOptimal, 1).
				//normals
				Update(norm, {}, ImageLayout::ShaderReadOnlyOptimal, 2).
				//albedo
				Update(albedo, {}, ImageLayout::ShaderReadOnlyOptimal, 3).
				//metallic/roughness
				Update(metalRough, {}, ImageLayout::ShaderReadOnlyOptimal, 4).
				//depths
				Update(depth, {}, ImageLayout::ShaderReadOnlyOptimal, 5);
			pack[3]->Update(s_Data->DirectionalLight[i], 0).
				Update({}, s_Data->Shadows.ShadowSampler, {}, "ShadowSampler").
#ifdef SHADOW_MAPPING
				Update(s_Data->Shadows.Framebuffer[0][i]->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, "DirectionalShadows0").
				Update(s_Data->Shadows.Framebuffer[1][i]->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, "DirectionalShadows1").
				Update(s_Data->Shadows.Framebuffer[2][i]->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, "DirectionalShadows2").
				Update(s_Data->Shadows.Framebuffer[3][i]->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, "DirectionalShadows3");
#endif
			return pack;
		});
	}
	int ShowDebugShadow = false;
	void SceneRenderer::ScreenPass(float zNear, float zFar, const glm::vec3& cameraPos, const glm::mat4& viewMatrix) {
		float scale = 24.0f / (std::log2f(zFar / zNear));
		float bias = -24.0f * (std::log2f(zNear)) / std::log2f(zFar / zNear);
		
		RenderCommand::SetViewport(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::SetScissor(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::BeginRenderPass(s_Data->ScreenRenderPass, *s_Data->HDROutputs,
			{ AttachmentColorClearValue(glm::vec4{0.0f,0.0f,0.0f,0.0f}) });
		RenderCommand::BindGraphicsPipeline(s_Data->ScreenPipeline);
		RenderCommand::BindVertexBuffers({ s_Data->ScreenNDC }, { 0 });
		s_Data->ScreenPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightPass"), 0, viewMatrix,
			glm::vec4(cameraPos, 0.0f), glm::vec2{ s_Data->Width,s_Data->Height }, zNear, zFar, scale, bias, ShowDebugShadow);
		RenderCommand::Draw(6, 1, 0, 0);
		RenderCommand::EndRenderPass();
	}
	
	void SceneRenderer::InsertScreenPassBarrier()
	{
		ImageMemoryBarrier hdrBarrier(
			s_Data->HDROutputs->Get()->GetColorAttachment(0),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput,
			PipelineStages_FragmentShader,
			{},
			{},
			{ hdrBarrier }
		);
	}
	#pragma endregion

	#pragma region Tonemap Pass
	static void CreateTonemapPassResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA8SRGB,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,TextureSamples::x1,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
			};
			s_Data->TonemapRenderPass = RenderPass::Create(specs);
		}
		{
			FramebufferSpecification specs;
			specs.RenderPass = s_Data->TonemapRenderPass;
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			for (auto& fb : s_Data->Outputs) {
				fb = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs{};
			specs.Input.Bindings = {
				{
					{"a_NDC",Format::RG32F},
					{"a_TexCoords",Format::RG32F}
				}
			};
			specs.Multisample.Samples = TextureSamples::x1;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.RenderPass = s_Data->TonemapRenderPass;
			specs.Subpass = 0;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("TonemapPass", "assets/_shaders/TonemapPass.shader");
			s_Data->TonemapPipeline = GraphicsPipeline::Create(specs);
		}
		uint32_t i = 0;
		for (auto& pack : s_Data->TonemapPipelinePack) {
			pack = DescriptorSetPack(ShaderLibrary::GetNamedShader("TonemapPass"), {});
			//sampler
			pack[0]->Update({}, s_Data->GlobalSampler, {}, 0).
				//hdr
				Update(s_Data->HDROutputs[i]->GetColorAttachment(0), {}, ImageLayout::ShaderReadOnlyOptimal, 1);
			++i;
		}
	}
	
	static void TonemapPass() {
		RenderCommand::BeginRenderPass(s_Data->TonemapRenderPass, *s_Data->Outputs, {
			AttachmentClearValue(glm::vec4(0.0f))
			});
		RenderCommand::BindGraphicsPipeline(s_Data->TonemapPipeline);
		RenderCommand::BindVertexBuffers({ s_Data->ScreenNDC }, { 0 });
		//s_Data->TonemapPipelinePack->GetSet(0)->Update(
		//	s_Data->ShadowData.ShadowBuffers->Get()->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, 1
		//);
		s_Data->TonemapPipelinePack->Bind();
		//RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("TonemapPass"));
		RenderCommand::Draw(6, 1, 0, 0);
		RenderCommand::EndRenderPass();
	}
	#pragma endregion

	#pragma region Deferred Pass
	static void CreateGBufferResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors = {
				//Position
				RenderPassAttachment(Format::RGBA32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Normals
				RenderPassAttachment(Format::RGBA32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Albedo
				RenderPassAttachment(Format::RGBA8UN,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store),
				//Mettalic/roughness
				RenderPassAttachment(Format::RG32F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
					AttachmentLoadOp::Clear,AttachmentStoreOp::Store)
			};
			specs.DepthStencil =
				RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, samples,
					AttachmentLoadOp::Clear, AttachmentStoreOp::Store);
			specs.AutoResolve = samples != TextureSamples::x1;
			s_Data->DeferredPass.RenderPass = RenderPass::Create(specs);
		}

		{
			FramebufferSpecification specs{};
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			specs.RenderPass = s_Data->DeferredPass.RenderPass;

			s_Data->DeferredPass.Output.Construct([&specs](auto) {
				return Framebuffer::Create(specs);
			});
		}
	}

	static void DeferredPass(const glm::mat4& viewProj, const entt::registry& sceneReg) {
		//std::vector<AttachmentClearValue> clears;
		//for (auto& clear : s_Data->DeferredPass.RenderPass->GetSpecification().Colors) {
		//	clears.push_back(AttachmentColorClearValue(glm::vec4(0.0f)));
		//}
		//
		//clears.push_back(AttachmentDepthStencilClearValue(1.0f, 0));

		s_Data->DeferredPass.RenderPass->SetDepthClearValue(AttachmentDepthStencilClearValue(1.0f, 0));

		RenderCommand::BeginRenderPass(s_Data->DeferredPass.RenderPass, s_Data->DeferredPass.Output,
			{}
		);

		Ref<VertexBuffer> lastSetVertexBuffer = {};
		Ref<IndexBuffer> lastSetIndexBuffer = {};

		Ref<Material> lastSetMaterial = {};


		//RenderCommand::BindGraphicsPipeline(s_Data->DeferredPass.Pipeline);
		RenderCommand::SetViewport(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::SetScissor(s_Data->Width, s_Data->Height, 0, 0);

		{
			auto view = sceneReg.view<TransformComponent, ModelComponent>();
			for (auto& e : view) {
				auto& [tc,mc] = view.get(e);

				if (!mc.UsedModel)
					continue;
				
				glm::mat4 model = tc.GetTransform();

				for (auto& mesh : mc.UsedModel->GetMeshes()) {
					Ref<VertexBuffer> vb = mesh->GetVertexBuffer();
					Ref<IndexBuffer> ib = mesh->GetIndexBuffer();
					Ref<Material> mat = mesh->GetDefaultMaterial();
					KD_CORE_ASSERT(mat);
					
					if (lastSetVertexBuffer != vb) {
						RenderCommand::BindVertexBuffers({ vb }, { 0 });
						lastSetVertexBuffer = vb;
					}
					if (lastSetIndexBuffer != ib) {
						RenderCommand::BindIndexBuffer(ib, 0);
						lastSetIndexBuffer = ib;
					}
					
					if (!lastSetMaterial) {
						mat->BindValues();
						mat->BindPipeline();
						mat->BindBaseValues(viewProj);
						lastSetMaterial = mat;
					}
					else if (lastSetMaterial != mat) {
						mat->BindValues();
					
						//RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), mesh->GetDefaultMaterial()->GetTextureSet(), 1);
					
						if (lastSetMaterial->GetPipeline() != mat->GetPipeline()) {
							mat->BindPipeline();
							mat->BindBaseValues(viewProj);
						}
					
						lastSetMaterial = mat;
					}
					
					lastSetMaterial->BindTransform(model);
					
					RenderCommand::DrawIndexed(mesh->GetIndexCount(), mesh->GetVertexCount(), 1, 0, 0, 0);
				}
			}
		}
		RenderCommand::EndRenderPass();
		
	}
	
	static void InsertDeferredBarrier() {
		
		if (samples == TextureSamples::x1) {
			ImageMemoryBarrier positionBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(0),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier normalBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(1),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier albedoBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(2),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);
			ImageMemoryBarrier metallicRoughnessBarrier(
				s_Data->DeferredPass.Output->Get()->GetColorAttachment(3),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);
			ImageMemoryBarrier depthBarrier(
				s_Data->DeferredPass.Output->Get()->GetDepthAttachment(),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_DepthStencilWrite,
				AccessFlags_ShaderRead
			);
			RenderCommand::PipelineBarrier(
				PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests | PipelineStages_EarlyFragmentTests,
				PipelineStages_VertexShader,
				{},
				{},
				{ positionBarrier,albedoBarrier,normalBarrier,metallicRoughnessBarrier,depthBarrier }
			);
		}
		else {
			ImageMemoryBarrier positionBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(0),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier normalBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(1),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead);
			ImageMemoryBarrier albedoBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(2),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);
			ImageMemoryBarrier metallicRoughnessBarrier(
				s_Data->DeferredPass.Output->Get()->GetResolveAttachment(3),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_ColorAttachmentWrite,
				AccessFlags_ShaderRead
			);

			ImageMemoryBarrier depthBarrier(
				s_Data->DeferredPass.Output->Get()->GetDepthAttachment(),
				ImageLayout::ShaderReadOnlyOptimal,
				AccessFlags_DepthStencilWrite,
				AccessFlags_ShaderRead
			);

			RenderCommand::PipelineBarrier(
				PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests | PipelineStages_EarlyFragmentTests,
				PipelineStages_VertexShader,
				{},
				{},
				{ positionBarrier,albedoBarrier,normalBarrier,metallicRoughnessBarrier,depthBarrier }
			);
		}
	}

	#pragma endregion
	
	std::array<PerFrameResource<Ref<Texture2D>>*, 4> DepthTexture;
	static void CreateShadowPassResources() {
#ifdef SHADOW_MAPPING
		RenderPassSpecification rpSpecs;
		rpSpecs.DepthStencil =
			RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store);
		s_Data->Shadows.RenderPass = RenderPass::Create(rpSpecs);


		FramebufferSpecification fbSpecs;
		fbSpecs.Width = SHADOW_MAP_SIZE;
		fbSpecs.Height = SHADOW_MAP_SIZE;
		fbSpecs.RenderPass = s_Data->Shadows.RenderPass;
		for (int i = 0; i < 4; ++i) {
			s_Data->Shadows.Framebuffer[i].Construct([&fbSpecs](uint32_t) {
				return Framebuffer::Create(fbSpecs);
				});
		}
		for (int j = 0; j < 4; ++j) {
			DepthTexture[j] = new PerFrameResource<Ref<Texture2D>>;
			DepthTexture[j]->Construct([j](uint32_t i) {
				return s_Data->Shadows.Framebuffer[j][i]->GetDepthAttachment();
				});
		}
		
		GraphicsPipelineSpecification gpSpecs;
		gpSpecs.Shader = ShaderLibrary::LoadOrGetNamedShader("ShadowPass", "assets/_shaders/ShadowPass.glsl");
		gpSpecs.Input.Bindings.push_back(VertexInputBinding({
			{ "a_Position",Format::RGB32F },
			{ sizeof(MeshVertex) - sizeof(MeshVertex::Position) }
			}));
		gpSpecs.Multisample.Samples = TextureSamples::x1;
		gpSpecs.Primitive = PrimitiveTopology::TriangleList;
		gpSpecs.Rasterization.FrontCCW = true;
		gpSpecs.Rasterization.CullMode = PipelineCullMode::None;
		gpSpecs.RenderPass = s_Data->Shadows.RenderPass;
		gpSpecs.Subpass = 0;
		gpSpecs.DepthStencil.DepthTest = true;
		gpSpecs.DepthStencil.DepthWrite = true;
		gpSpecs.DepthStencil.DepthCompareOperator = CompareOp::LessOrEqual;
		
		s_Data->Shadows.Pipeline = GraphicsPipeline::Create(gpSpecs);
#endif
	}

#if TYPE == 1
	static glm::mat4 CalcLightViewProjectionMatrix(const glm::mat4& cameraViewProjInv, const glm::vec3& normalizedLightDir) {
		glm::vec3 endpoints[8] = {
			glm::vec3(-1.0f,1.0f,0.0f),
			glm::vec3(1.0f,1.0f,0.0f),
			glm::vec3(1.0f,-1.0f,0.0f),
			glm::vec3(-1.0f,-1.0f,0.0f),

			glm::vec3(-1.0f,1.0f,1.0f),
			glm::vec3(1.0f,1.0f,1.0f),
			glm::vec3(1.0f,-1.0f,1.0f),
			glm::vec3(-1.0f,-1.0f,1.0f),
		};

		for (uint32_t j = 0; j < 8; j++) {
			glm::vec4 invCorner = cameraViewProjInv * glm::vec4(endpoints[j], 1.0f);
			endpoints[j] = invCorner / invCorner.w;
		}

		// Get frustum center
		glm::vec3 center = glm::vec3(0.0f);
		for (uint32_t j = 0; j < 8; j++) {
			center += endpoints[j];
		}
		center /= 8.0f;

		float radius = glm::length(endpoints[6] - endpoints[0]);


		glm::vec3 lightPos = center - normalizedLightDir * radius;

		glm::mat4 lightView = glm::lookAt(lightPos, center, glm::vec3(0, 1, 0));

		glm::mat4 lightProj = glm::ortho(-radius, radius, -radius, radius, 0.0f, 2.0f * radius);

		return lightProj * lightView;
	}
#endif

#ifdef TYPE == 2
#undef near
#undef far

#define CONCAT(x,y) x ## (y)

	static void GetEndpoints(const glm::mat4& invViewProjMatrix, glm::vec3* endpoints) {

		endpoints[0] = glm::vec3(-1.0f,  1.0f, 0.0f);
		endpoints[1] = glm::vec3(1.0f,  1.0f, 0.0f);
		endpoints[2] = glm::vec3(1.0f, -1.0f, 0.0f);
		endpoints[3] = glm::vec3(-1.0f, -1.0f, 0.0f);
		endpoints[4] = glm::vec3(-1.0f,  1.0f, 1.0f);
		endpoints[5] = glm::vec3(1.0f,  1.0f, 1.0f);
		endpoints[6] = glm::vec3(1.0f, -1.0f, 1.0f);
		endpoints[7] = glm::vec3(-1.0f, -1.0f, 1.0f);

		for (int i = 0; i < 8; ++i) {
			glm::vec4 corner = invViewProjMatrix * glm::vec4(endpoints[i],1.0f);
			endpoints[i] = corner / corner.w;
		}
	}

	static glm::vec3 GetCenter(const glm::vec3* endpoints) {
		glm::vec3 center = glm::vec3(0, 0, 0);
		for (int i = 0; i < 8; ++i) {
			center += endpoints[i];
		}

		return center / 8.0f;
	}

	static float GetRadius(const glm::vec3* endpoints, const glm::vec3& center) {
		float radius = 0.0f;

		for (int i = 0; i < 8; ++i) {
			float d = glm::length(endpoints[i] - center);
			if (d > radius)
				radius = d;
		}

		return radius;
	}

	double Snapped(double p_value, double p_step) {
		if (p_step != 0) {
			p_value = glm::floor(p_value / p_step + 0.5) * p_step;
		}
		return p_value;
	}

	static std::vector<glm::mat4> ComputeLightViewProjections(
		const std::vector<float>& cascadeSplits,
		int numCascades,
		float nearPlane,
		float farPlane,
		float fovDegrees,
		float aspectRatio,
		const glm::mat4& cameraViewMatrix,
		const glm::vec3& lightDir,
		const glm::mat4& lightRotation,
		int shadowMapResolution)
	{
		std::vector<glm::mat4> lightViewProjections;
		lightViewProjections.reserve(numCascades);

		for (int i = 0; i < numCascades; ++i) {
			float near = cascadeSplits[i];
			float far = cascadeSplits[i + 1];

			glm::mat4 cameraSplitProjection = glm::perspective(glm::radians(fovDegrees), aspectRatio, near, far);

			glm::vec3 endpoints[8];
			GetEndpoints(glm::inverse(cameraSplitProjection * cameraViewMatrix), endpoints);

			glm::vec3 center = GetCenter(endpoints);

			float radius = GetRadius(endpoints, center);

			radius = std::ceil(radius);

			auto lightPos = center - radius * lightDir;

			KD_CORE_INFO("{},{},{}", lightPos.x, lightPos.y, lightPos.z);

			glm::mat4 lightViewMatrix = glm::lookAt(-lightDir, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));

			glm::vec3 maxOrtho = glm::vec3(radius, radius, 2.0f * radius);
			glm::vec3 minOrtho = glm::vec3(-radius, -radius, 0.0f);

			glm::mat4 lightOrthoMatrix = glm::ortho(minOrtho.x, maxOrtho.x, minOrtho.y, maxOrtho.y, minOrtho.z, maxOrtho.z);

			//glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;

			float texelsPerUnit = (float)shadowMapResolution / (2.0f * radius);

			glm::mat4 scaler = glm::scale(glm::mat4(1.0f), glm::vec3(texelsPerUnit));
			lightViewMatrix = scaler * lightViewMatrix;

			glm::vec4 newCenter = lightViewMatrix * glm::vec4(center,1.0);
			float w = newCenter.w;
			newCenter /= w;
			newCenter.x = (float)floor(newCenter.x);
			newCenter.y = (float)floor(newCenter.y);
			newCenter *= w;
			center = glm::inverse(lightViewMatrix) * newCenter;

			glm::vec3 eye = center - (lightDir * radius * 1.0f);

			lightViewMatrix = glm::lookAt(eye, center, glm::vec3(0, 1, 0));
			lightOrthoMatrix = glm::ortho(-radius, radius, -radius, radius, -1.0f * radius, 1.0f * radius);

			lightViewProjections.push_back(lightOrthoMatrix * lightViewMatrix);
		}
		//how do i stabalize a directional light's split frustum's light view projection using snapping to texel size increments in c++(im using vulkan with glm)
		return lightViewProjections;
	}

#endif
			//// Now apply texel snapping  
			//float texelSizeX = 1.0f / shadowMapResolution;
			//float texelSizeY = 1.0f / shadowMapResolution;

			//// Computing snapped light space position (assuming we want the center of the cascade)  
			//glm::vec4 lightSpacePos = shadowMatrix * glm::vec4(center, 1.0f);
			//lightSpacePos /= lightSpacePos.w; // Convert to normalized device coordinates (NDC)  

			//float snappedX = std::floor(lightSpacePos.x * shadowMapResolution) * texelSizeX;
			//float snappedY = std::floor(lightSpacePos.y * shadowMapResolution) * texelSizeY;

			//// Create a new shadow matrix adjusting with the snapped values  
			//shadowMatrix = glm::translate(shadowMatrix, glm::vec3(snappedX - lightSpacePos.x, snappedY - lightSpacePos.y, 0.0f));

	static glm::vec4 NearZeroToZero(const glm::vec4& vec, float epsilon) {
		glm::vec4 ret = vec;
		
		if (glm::epsilonEqual(ret.x, 0.0f, epsilon)) {
			ret.x = 0.0f;
		}

		if (glm::epsilonEqual(ret.y, 0.0f, epsilon)) {
			ret.y = 0.0f;
		}

		if (glm::epsilonEqual(ret.z, 0.0f, epsilon)) {
			ret.z = 0.0f;
		}

		if (glm::epsilonEqual(ret.w, 0.0f, epsilon)) {
			ret.w = 0.0f;
		}

		return ret;
	}

	static std::vector<float> GetSplitDistances(uint32_t count, float near, float far) {
		std::vector<float> cascadeSplits(count + 1);

		float minZ = near;
		float maxZ = far;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		cascadeSplits[0] = near;

		for (uint32_t i = 0; i < 4; i++) {
			float p = (i + 2) / static_cast<float>(count);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = 0.5f* (log - uniform) + uniform;
			cascadeSplits[i + 1] = (d - near);
		}

		cascadeSplits[count] = far;

		return cascadeSplits;
	}

	static void ShadowPass(const SceneData& sceneData, const entt::registry& sceneReg) {

		{
			auto view = sceneReg.view<TransformComponent, DirectionalLightComponent>();
			entt::entity e = *view.begin();
			auto& [tc, dlc] = view.get<TransformComponent, DirectionalLightComponent>(e);

			float maxDistance = glm::min(sceneData.zFar, dlc.Size);
			maxDistance = glm::max(maxDistance, sceneData.zNear + 0.001f);

			float minDistance = glm::min(sceneData.zNear, maxDistance);

			DLight.Color = dlc.Color;
#if TYPE == 2
			glm::mat4 lightRotation = glm::toMat4(glm::quat(tc.Rotation));

			glm::vec3 lightDir = lightRotation * glm::vec4(1,0,0,1);
			
			lightDir = glm::normalize(NearZeroToZero(glm::vec4(lightDir, 0.0f), glm::epsilon<float>()));

			lightDir = glm::normalize(dlc.Direction);

			DLight.Direction = lightDir;

			std::vector<float> splits = GetSplitDistances(4, minDistance, maxDistance);

			auto viewProjs = ComputeLightViewProjections(splits, 4, minDistance, maxDistance, sceneData.FOV, sceneData.AspectRatio, sceneData.View, lightDir, lightRotation, SHADOW_MAP_SIZE);
			DLight.ViewProj[0] = viewProjs[0];
			DLight.ViewProj[1] = viewProjs[1];
			DLight.ViewProj[2] = viewProjs[2];
			DLight.ViewProj[3] = viewProjs[3];
			
			DLight.SplitDistances[0] = splits[1];
			DLight.SplitDistances[1] = splits[2];
			DLight.SplitDistances[2] = splits[3];
			DLight.SplitDistances[3] = splits[4];
#endif
#if TYPE == 1
			glm::mat4 invViewProj = glm::perspective(glm::radians(sceneData.FOV), sceneData.AspectRatio, minDistance, maxDistance) * sceneData.View;

			invViewProj = glm::inverse(invViewProj);
			DLight.ViewProj = CalcLightViewProjectionMatrix(invViewProj,glm::normalize(dlc.Direction));
#endif
		}

		s_Data->DirectionalLight->Get()->SetData(&DLight, sizeof(DLight));

		Ref<VertexBuffer> lastSetVertexBuffer = {};
		Ref<IndexBuffer> lastSetIndexBuffer = {};

		s_Data->Shadows.RenderPass->SetDepthClearValue({ 1.0f,0 });
		RenderCommand::BindGraphicsPipeline(s_Data->Shadows.Pipeline);
		RenderCommand::SetViewport(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,0,0);
		RenderCommand::SetScissor(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,0,0);

		
		for (int i = 0; i < 4; ++i) {
			RenderCommand::BeginRenderPass(s_Data->Shadows.RenderPass, s_Data->Shadows.Framebuffer[i], {});

			glm::mat4 vp = DLight.ViewProj[i];
			{
				auto view = sceneReg.view<TransformComponent, ModelComponent>();
				for (auto& e : view) {
					auto& [tc, mc] = view.get(e);

					if (!mc.UsedModel)
						continue;

					glm::mat4 model = tc.GetTransform();

					for (auto& mesh : mc.UsedModel->GetMeshes()) {
						Ref<VertexBuffer> vb = mesh->GetVertexBuffer();
						Ref<IndexBuffer> ib = mesh->GetIndexBuffer();
						if (lastSetVertexBuffer != vb) {
							RenderCommand::BindVertexBuffers({ vb }, { 0 });
							lastSetVertexBuffer = vb;
						}
						if (lastSetIndexBuffer != ib) {
							RenderCommand::BindIndexBuffer(ib, 0);
							lastSetIndexBuffer = ib;
						}
						glm::mat4 mvp = vp * model;
						RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("ShadowPass"), 0, mvp);

						RenderCommand::DrawIndexed(mesh->GetIndexCount(), mesh->GetVertexCount(), 1, 0, 0, 0);
					}
				}
			}

			RenderCommand::EndRenderPass();
		}
	}
	
	static void InsertShadowPassBarrier() {

#ifdef SHADOW_MAPPING
		ImageMemoryBarrier depthBarrier0(
			s_Data->Shadows.Framebuffer[0]->Get()->GetDepthAttachment(),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_DepthStencilWrite,
			AccessFlags_ShaderRead
		);
		ImageMemoryBarrier depthBarrier1(
			s_Data->Shadows.Framebuffer[1]->Get()->GetDepthAttachment(),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_DepthStencilWrite,
			AccessFlags_ShaderRead
		);
		ImageMemoryBarrier depthBarrier2(
			s_Data->Shadows.Framebuffer[2]->Get()->GetDepthAttachment(),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_DepthStencilWrite,
			AccessFlags_ShaderRead
		);
		ImageMemoryBarrier depthBarrier3(
			s_Data->Shadows.Framebuffer[3]->Get()->GetDepthAttachment(),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_DepthStencilWrite,
			AccessFlags_ShaderRead
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests | PipelineStages_EarlyFragmentTests,
			PipelineStages_VertexShader,
			{},
			{},
			{ depthBarrier0,depthBarrier1,depthBarrier2,depthBarrier3}
		);
#endif
	}

	#pragma endregion


	bool SceneRenderer::NeedsRecreation(Ref<Texture2D> output) {
		const auto& specs = output->GetTextureSpecification();
		return specs.Width != s_Data->Width ||
			specs.Height != s_Data->Height;
	}
	
	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
		if (!s_Data) {
			s_Data = new Data;

			{
				glm::vec2 screenVertices[] =
				{
					//NDC, TexCoords
					{-1.0,-1.0},{0,1},
					{1.0,-1.0},{1,1},
					{1,1},{1,0},
					{1,1},{1,0},
					{-1,1},{0,0},
					{-1.0,-1.0},{0,1},
				};

				s_Data->ScreenNDC = VertexBuffer::Create(screenVertices, sizeof(screenVertices));
			}
			
			{
				SamplerState state;
				state.Aniso = false;
				state.MagFilter = SamplerFilter::Linear;
				state.MinFilter = SamplerFilter::Linear;
				state.MipFilter = SamplerMipMapMode::Linear;
				state.AddressModeU = SamplerAddressMode::Repeat;
				state.AddressModeV = SamplerAddressMode::Repeat;
				state.AddressModeW = SamplerAddressMode::Repeat;
				s_Data->GlobalSampler = Sampler::Create(state);
			}

			{
				SamplerState state;
				state.AddressModeU = SamplerAddressMode::Repeat;
				state.AddressModeV = SamplerAddressMode::Repeat;
				state.AddressModeW = SamplerAddressMode::Repeat;
				state.MagFilter = SamplerFilter::Linear;
				state.MinFilter = SamplerFilter::Linear;
				state.MipFilter = SamplerMipMapMode::Linear;

				s_Data->GBufferSampler = Sampler::Create(state);
			}

			{
				Texture2DSpecification specs;
				specs.Width = 1;
				specs.Height = 1;
				specs.Depth = 1;
				specs.Mips = 1;
				specs.Layers = 1;
				specs.Layout = ImageLayout::ShaderReadOnlyOptimal;
				specs.Samples = TextureSamples::x1;
				specs.Swizzles[0] = TextureSwizzle::Red;
				specs.Swizzles[1] = TextureSwizzle::Green;
				specs.Swizzles[2] = TextureSwizzle::Blue;
				specs.Swizzles[3] = TextureSwizzle::Alpha;

				uint32_t data = 0xFFFFFFFF;

				TextureData init{};
				init.Layer = 0;
				init.Data = &data;
				specs.InitialDatas.push_back(init);
				specs.Format = Format::RGBA8UN;
				s_Data->DefaultWhite = Texture2D::Create(specs);
			}

			CreateClusterResources();
			CreateLightCullResources();
			CreateGBufferResources();

			CreateShadowPassResources();
			
			CreateScreenPassResources();
			
			CreateTonemapPassResources();
			
			cubes.resize(1);
			cubes[0] = glm::scale(glm::mat4(1.0f),glm::vec3(.1f));

			//s_Data->Instances = StorageBuffer::Create(cubes.size() * sizeof(glm::mat4));
			//s_Data->Instances->SetData(cubes.data(), cubes.size() * sizeof(glm::mat4));

			//s_Data->TransformsSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0);
			//s_Data->TransformsSet->Update(s_Data->Instances, 0);

			Light l{};
			l.Color = glm::vec4(1.0f, 0, 0, 1);
			l.Position = glm::vec4(0,5,0.0f,0.0f);
			l.Radius = 100.0f;

			lights.push_back(l);
		}
	}

	//TODO: implement to ECS, make hierarchy and frustum culling.
	//TODO: shadows.
	//TODO: forward render.
	//TODO: make it easy to switch render modes, deferred/forward renderer.
	void SceneRenderer::Render(Ref<Texture2D> outputBuffer, const SceneData& sceneData)
	{
		
		Scene* scene = (Scene*)m_Context;
		if (NeedsRecreation(outputBuffer)) {
			RenderCommand::DeviceWaitIdle();
			s_Data->Width = outputBuffer->GetTextureSpecification().Width;
			s_Data->Height = outputBuffer->GetTextureSpecification().Height;
			RecreateSizeOrSampleDependent();
		}

		glm::mat4 proj = sceneData.Proj;
		glm::mat4 view = sceneData.View;

		glm::mat4 viewProj = sceneData.ViewProj;

		glm::mat4 invProj = glm::inverse(proj);

		float zNear = sceneData.zNear;
		float zFar = sceneData.zFar;

		MakeClusters(invProj,zNear,zFar,sceneData.ScreenSize);
		InsertClusterBarrier();
		MakeLightGrids(view);
		DeferredPass(viewProj,scene->m_Registry);
		InsertLightGridBarrier();
		InsertDeferredBarrier();

		ShadowPass(sceneData, scene->m_Registry);
		InsertShadowPassBarrier();

		ScreenPass(zNear,zFar,sceneData.CameraPos,view);
		InsertScreenPassBarrier();

		TonemapPass();

		ResolveToOutput(outputBuffer);
	}

	void SceneRenderer::RecreateSizeOrSampleDependent()
	{
		CreateGBufferResources();
		CreateScreenPassResources();
		CreateTonemapPassResources();
	}

	void SceneRenderer::ResolveToOutput(Ref<Texture2D> outputBuffer)
	{
		//Screen pass barrier
		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					s_Data->Outputs->Get()->GetColorAttachment(0),
					ImageLayout::TransferSrcOptimal,
					AccessFlags_ColorAttachmentWrite,
					AccessFlags_TransferRead
				);
			RenderCommand::PipelineBarrier(
				PipelineStages_ColorAttachmentOutput,
				PipelineStages_Transfer,
				{},
				{},
				{ barrier }
			);
		}

		ImageLayout outputLayout = outputBuffer->GetTextureSpecification().Layout;

		//Transition Output to copy layout
		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					outputBuffer,
					ImageLayout::TransferDstOptimal,
					AccessFlags_None,
					AccessFlags_TransferWrite
				);

			barrier.OldLayout = ImageLayout::None;

			RenderCommand::PipelineBarrier(
				PipelineStages_TopOfPipe,
				PipelineStages_Transfer,
				{},
				{},
				{ barrier }
			);
		}

		RenderCommand::CopyTexture(s_Data->Outputs->Get()->GetColorAttachment(0), 0, 0, outputBuffer, 0, 0);

		{
			ImageMemoryBarrier barrier =
				ImageMemoryBarrier(
					outputBuffer,
					ImageLayout::ShaderReadOnlyOptimal,
					AccessFlags_TransferWrite,
					AccessFlags_ShaderRead
				);

			RenderCommand::PipelineBarrier(
				PipelineStages_Transfer,
				PipelineStages_VertexShader,
				{},
				{},
				{ barrier }
			);
		}
	}

	glm::vec3 col = glm::vec3(1.0), dir = glm::vec3(0,0,1),pos(0,0,-500);

}
