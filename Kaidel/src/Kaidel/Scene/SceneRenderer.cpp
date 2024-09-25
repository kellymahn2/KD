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

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>

#define MAX_LIGHT_COUNT 100

namespace Kaidel {
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

		Ref<RenderPass> GBufferRenderPass;
		PerFrameResource<Ref<Framebuffer>> GBuffers;
		Ref<GraphicsPipeline> GBufferPipeline;
		DescriptorSetPack GBufferPipelinePack;
		Ref<StorageBuffer> GBufferInstances;

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

		Ref<Sampler> GlobalSampler;
		uint32_t Width = 1280, Height = 720;
	}*s_Data;

	TextureSamples samples = TextureSamples::x1;

	struct Light {
		glm::vec4 Position;
		glm::vec4 Color;
		float Radius;
	};

	std::vector<Light> lights;

	struct DirectionalLight {
		alignas(16) glm::vec3 Direction;
		alignas(16) glm::vec3 Color;
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

#include "temp.h"

	static Model* model;

	static void CreateTestSponzaModel() {
		model = new Model("assets/models/Sponza/Sponza.gltf");
	}

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
			s_Data->GBufferRenderPass = RenderPass::Create(specs);
		}

		{
			FramebufferSpecification specs{};
			specs.Width = s_Data->Width;
			specs.Height = s_Data->Height;
			specs.RenderPass = s_Data->GBufferRenderPass;
			for (auto& buffer : s_Data->GBuffers) {
				buffer = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs;

			specs.Input.Bindings.push_back(VertexInputBinding({
				{"a_Position",Format::RGB32F},
				{"a_TexCoords",Format::RG32F},
				{"a_Normal",Format::RGB32F},
				{"a_Tangent",Format::RGB32F},
				{"a_BiTangent",Format::RGB32F},
			}));
			specs.Multisample.Samples = samples;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.Rasterization.FrontCCW = true;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("DeferredGBufferGen", "assets/_shaders/DeferredGBufferGen.shader");
			specs.RenderPass = s_Data->GBufferRenderPass;
			specs.Subpass = 0;
			specs.DepthStencil.DepthTest = true;
			specs.DepthStencil.DepthWrite = true;
			specs.DepthStencil.DepthCompareOperator = CompareOp::LessOrEqual;
			s_Data->GBufferPipeline = GraphicsPipeline::Create(specs);
		}
	}

	static void CreateScreenPassResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA16F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
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
			specs.Multisample.Samples = samples;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.RenderPass = s_Data->ScreenRenderPass;
			specs.Subpass = 0;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::LoadShader("LightPass", "assets/_shaders/CubeLightPass.shader");
			s_Data->ScreenPipeline = GraphicsPipeline::Create(specs);
		}
		uint32_t i = 0;
		for (auto& pack : s_Data->ScreenPipelinePack) {
			pack = DescriptorSetPack(ShaderLibrary::GetNamedShader("LightPass"), {});
			pack[0]->Update(s_Data->Clusters[i], 0);
			pack[1]->Update(s_Data->LightCullPipelinePack[i].GetSet(1)->GetStorageBufferAtBinding(0), 0).
				Update(s_Data->ClusterGrids[i], 1);
			//sampler
			pack[2]->Update({}, s_Data->GlobalSampler, {}, 0).
				//positions
				Update(s_Data->GBuffers[i]->GetColorAttachment(0), {}, ImageLayout::ShaderReadOnlyOptimal, 1).
				//normals
				Update(s_Data->GBuffers[i]->GetColorAttachment(1), {}, ImageLayout::ShaderReadOnlyOptimal, 2).
				//albedo
				Update(s_Data->GBuffers[i]->GetColorAttachment(2), {}, ImageLayout::ShaderReadOnlyOptimal, 3).
				//metallic/roughness
				Update(s_Data->GBuffers[i]->GetColorAttachment(3), {},ImageLayout::ShaderReadOnlyOptimal,4).
				//depths
				Update(s_Data->GBuffers[i]->GetDepthAttachment(), {}, ImageLayout::ShaderReadOnlyOptimal, 5);
			pack[3]->Update(s_Data->DirectionalLight[i], 0);
			++i;
		}
	}

	static void CreateTonemapPassResources() {
		{
			RenderPassSpecification specs{};
			specs.Colors =
			{
				RenderPassAttachment(Format::RGBA8SRGB,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,samples,
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
			specs.Multisample.Samples = samples;
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

	static void CreateMaterialResources() {
		s_Data->Albedo = TextureLibrary::Load("assets/textures/container2.png", ImageLayout::ShaderReadOnlyOptimal, Format::RGBA8UN);
		s_Data->Spec = TextureLibrary::Load("assets/textures/container2_specular.png", ImageLayout::ShaderReadOnlyOptimal, Format::R8UN);
		
	}

	static void DeferredPass(const glm::mat4& viewProj, Scene* scene) {

		std::vector<AttachmentClearValue> clears;
		for (auto& clear : s_Data->GBufferRenderPass->GetSpecification().Colors) {
			clears.push_back(AttachmentColorClearValue(glm::vec4(0.0f)));
		}

		clears.push_back(AttachmentDepthStencilClearValue(1.0f, 0));

		RenderCommand::BeginRenderPass(s_Data->GBufferRenderPass, *s_Data->GBuffers,
			std::initializer_list<AttachmentClearValue>(clears.data(),clears.data() + clears.size())
		);

		Ref<VertexBuffer> lastSetVertexBuffer = s_Data->CubeVertexBuffer;
		Ref<IndexBuffer> lastSetIndexBuffer = s_Data->CubeIndexBuffer;

		RenderCommand::BindGraphicsPipeline(s_Data->GBufferPipeline);
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), s_Data->TransformsSet, 0);
		RenderCommand::SetViewport(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::SetScissor(s_Data->Width, s_Data->Height, 0, 0);
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0, viewProj);
		for (auto& mesh : model->GetMeshes()) {
			RenderCommand::BindVertexBuffers({ mesh->GetVertexBuffer() }, {0});
			RenderCommand::BindIndexBuffer(mesh->GetIndexBuffer(), 0);
			RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), mesh->GetDefaultMaterial()->GetTextureSet(), 1);
			RenderCommand::DrawIndexed(mesh->GetIndexCount(), mesh->GetVertexCount(), cubes.size(), 0, 0, 0);
		}
		RenderCommand::EndRenderPass();
	}

	//Barrier for sync of Deferred passes
	static void InsertDeferredBarrier() {
		ImageMemoryBarrier positionBarrier(
			s_Data->GBuffers->Get()->GetColorAttachment(0),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead);
		ImageMemoryBarrier normalBarrier(
			s_Data->GBuffers->Get()->GetColorAttachment(1),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead);
		ImageMemoryBarrier albedoBarrier(
			s_Data->GBuffers->Get()->GetColorAttachment(2),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead
		);
		ImageMemoryBarrier metallicRoughnessBarrier(
			s_Data->GBuffers->Get()->GetColorAttachment(3),
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_ColorAttachmentWrite,
			AccessFlags_ShaderRead
		);
		ImageMemoryBarrier depthBarrier(
			s_Data->GBuffers->Get()->GetDepthAttachment(),
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

	static void TonemapPass() {
		RenderCommand::BeginRenderPass(s_Data->TonemapRenderPass, *s_Data->Outputs, {
			AttachmentClearValue(glm::vec4(0.0f))
			});
		RenderCommand::BindGraphicsPipeline(s_Data->TonemapPipeline);
		RenderCommand::BindVertexBuffers({ s_Data->ScreenNDC }, { 0 });
		s_Data->TonemapPipelinePack->Bind();
		//RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("TonemapPass"));
		RenderCommand::Draw(6, 1, 0, 0);
		RenderCommand::EndRenderPass();
	}


#if 0
	template<typename T>
	void CopyToBuffer(uint8_t* buffer,const T& value) {
		std::memcpy(buffer, &value, sizeof(T));
	}

	template<typename T,typename...Args>
	void CopyToBuffer(uint8_t* buffer, const T& value, const Args&... args) {
		std::memcpy(buffer, &value, sizeof(T));
		CopyToBuffer(buffer + sizeof(T), args...);
	}
	template<typename T,typename... Args>
	const uint8_t* MakePushConstants(const T& value, const Args&... args) {
		static uint8_t Buffer[sizeof(T) + (sizeof(Args) + ...)];
		CopyToBuffer(Buffer, value, args...);
		return Buffer;
	}

	template<typename T>
	const uint8_t* MakePushConstants(const T& value) {
		static uint8_t Buffer[sizeof(T)];
		CopyToBuffer(Buffer, value);
		return Buffer;
	}
#endif
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

			//s_Data->CubeVertexBuffer = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(Vertex));
			//s_Data->CubeIndexBuffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint16_t), IndexType::Uint16);
			{
				glm::vec2 screenVertices[] =
				{
					//NDC, TexCoords
					{-1.0,-1.0},{0,0},
					{1.0,-1.0},{1,0},
					{1,1},{1,1},
					{1,1},{1,1},
					{-1,1},{0,1},
					{-1.0,-1.0},{0,0},
				};

				s_Data->ScreenNDC = VertexBuffer::Create(screenVertices, sizeof(screenVertices));
			}
			{
				SamplerState state;
				state.Aniso = false;
				state.MagFilter = SamplerFilter::Linear;
				state.MinFilter = SamplerFilter::Linear;
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

			s_Data->DirectionalLight.Construct([](uint32_t) {return UniformBuffer::Create(sizeof(DirectionalLight)); });
			
			s_Data->DirectionalLightSet.Construct([](uint32_t i) {
				auto set = DescriptorSet::Create(DescriptorSetLayoutSpecification({ {DescriptorType::UniformBuffer,ShaderStage_FragmentShader} }));
				set->Update(s_Data->DirectionalLight[i], 0);
				return set;
			});

			CreateScreenPassResources();
			
			CreateTonemapPassResources();
			CreateTestSponzaModel();
			
			cubes.resize(1);
			cubes[0] = glm::scale(glm::mat4(1.0f),glm::vec3(.1f));

			s_Data->GBufferInstances = StorageBuffer::Create(cubes.size() * sizeof(glm::mat4));
			s_Data->GBufferInstances->SetData(cubes.data(), cubes.size() * sizeof(glm::mat4));

			CreateMaterialResources();	

			s_Data->TransformsSet = DescriptorSet::Create(ShaderLibrary::GetNamedShader("DeferredGBufferGen"), 0);
			s_Data->TransformsSet->Update(s_Data->GBufferInstances, 0);

			Light l{};
			l.Color = glm::vec4(1.0f, 1, 1, 1);
			l.Position = glm::vec4(0,5,0.0f,0.0f);
			l.Radius = 100.0f;

			lights.push_back(l);
		}
	}

	//TODO: implement to ECS, make hierarchy and frustum culling.
	//TODO: implement shadows.
	void SceneRenderer::Render(Ref<Texture2D> outputBuffer, const SceneData& sceneData)
	{
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
		DeferredPass(viewProj,(Scene*)m_Context);
		InsertLightGridBarrier();
		InsertDeferredBarrier();

		ScreenPass(zNear,zFar,sceneData.CameraPos);
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

	glm::vec3 col = glm::vec3(1.0), dir = glm::vec3(0.0f,-5.0f,1.33f);
	void SceneRenderer::ScreenPass(float zNear, float zFar, const glm::vec3& cameraPos) {
		float scale = 24.0f / (std::log2f(zFar / zNear));
		float bias = -24.0f * (std::log2f(zNear)) / std::log2f(zFar / zNear);

		DLight.Color = col;
		DLight.Direction = dir;

		s_Data->DirectionalLight->Get()->SetData(&DLight, sizeof(DLight));

		RenderCommand::BeginRenderPass(s_Data->ScreenRenderPass, *s_Data->HDROutputs,
			{ AttachmentColorClearValue(glm::vec4{0.0f,0.0f,0.0f,0.0f}) });
		RenderCommand::BindGraphicsPipeline(s_Data->ScreenPipeline);
		RenderCommand::BindVertexBuffers({ s_Data->ScreenNDC }, { 0 });
		s_Data->ScreenPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightPass"), 0,
			glm::vec4(cameraPos, 0.0f), glm::vec2{ s_Data->Width,s_Data->Height }, zNear, zFar, scale, bias);
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
	void SceneRenderer::MakeClusters(const glm::mat4& invProj, float zNear, float zFar, const glm::vec2& screenSize) {
		RenderCommand::BindComputePipeline(s_Data->ClusterPipeline);
		s_Data->ClusterPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("Cluster"), 0,
			invProj, zNear, zFar, screenSize);
		RenderCommand::Dispatch(s_ClusterDimensions.x, s_ClusterDimensions.y, s_ClusterDimensions.z);
	}
	//Barrier for sync of ClusterAABB writes
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
	void SceneRenderer::MakeLightGrids(const glm::mat4& view) {
		s_Data->LightCullPipelinePack->GetSet(1)->GetStorageBufferAtBinding(0)->SetData(lights.data(), lights.size() * sizeof(Light));

		RenderCommand::BindComputePipeline(s_Data->LightCullPipeline);
		s_Data->LightCullPipelinePack->Bind();
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("LightCull"), 0,
			view, lights.size());
		RenderCommand::Dispatch(1, 1, 6);
	}
	//Barrier for sync of light grid creation
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
}
