#include "KDpch.h"
#include "Renderer3D.h"

#include "Kaidel/Core/DebugUtils.h"

#include <glm/gtx/compatibility.hpp>

namespace Kaidel {
	static const constexpr uint32_t DirectionalShadowmapWidth = 4096;
	static const constexpr uint32_t DirectionalShadowmapHeight = 4096;
	
	static const constexpr glm::vec3 SkyboxVertices[36] = {
		// Front face
		glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),

		// Back face
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),

		// Left face
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),

		// Right face
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),

		// Top face
		glm::vec3(-1.0f,  1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f,  1.0f),
		glm::vec3(1.0f,  1.0f, -1.0f),
		glm::vec3(-1.0f,  1.0f, -1.0f),

		// Bottom face
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
	};

	template<typename T>
	struct PerViewportData
	{
		std::unordered_map<std::string, T> Data;

		T& operator[](const std::string& name)
		{
			return Data[name];
		}
	};

	struct Pass
	{
		Ref<RenderPass> Pass;
		Ref<GraphicsPipeline> Pipeline;
		Ref<Kaidel::Shader> Shader;
	};

	struct ShadowPass : Pass
	{
		PerFrameResource<Ref<Framebuffer>> Cascades[4];	
		uint32_t CurrentCascade = 0;
	};

	struct DeferredPass : Pass
	{
		PerFrameResource<Ref<Framebuffer>> GBuffers;
	};

	struct ScreenPass : Pass 
	{
		PerFrameResource<DescriptorSetPack> Descriptors;
		PerFrameResource<Ref<Framebuffer>> HDR;
	};

	struct TonemapPass : Pass
	{
		PerFrameResource<DescriptorSetPack> Descriptors;
		PerFrameResource<Ref<Framebuffer>> SDR;
	};

	struct SkyboxPass : Pass
	{
		Ref<VertexBuffer> Vertices;
	};

	struct ViewportStorageBuffers {
		PerFrameResource<Ref<StorageBuffer>> InstanceDataBuffer;
		PerFrameResource<Ref<DescriptorSet>> InstanceDataSet;

		PerFrameResource<Ref<StorageBuffer>> ShadowInstanceDataBuffer[4];
		PerFrameResource<Ref<DescriptorSet>> ShadowInstanceDataSet[4];
	};

	//Per-screen
	struct Renderer3DScreenSize
	{
		uint32_t RenderWidth = 1280, RenderHeight = 720;
	};

	struct Renderer3DData
	{
		PerViewportData<Renderer3DScreenSize> ViewportSize;

		ShadowPass Shadow;
		PerViewportData<DeferredPass> Deferred;
		PerViewportData<ScreenPass> Screen;
		PerViewportData<TonemapPass> Tonemap;
		SkyboxPass Skybox;

		DirectionalLightData LightData;
		PerFrameResource<Ref<UniformBuffer>> DirectionalLightBuffer;
		PerFrameResource<Ref<DescriptorSet>> DirectionalLightSet;

		Kaidel::SceneData SceneData;
		PerFrameResource<Ref<UniformBuffer>> SceneDataBuffer;
		PerFrameResource<Ref<DescriptorSet>> SceneDataSet;

		std::string CurrentViewport;

		Renderer3DState RendererState;

		Ref<EnvironmentMap> Environment;

		Ref<Texture> Output;

		Ref<Material> m_LastMat;
		Renderer3DRenderParams m_LastParams;

		PerViewportData<ViewportStorageBuffers> StorageBuffers;

		uint64_t InstanceCount = 0;

		uint64_t InstanceOffset = 0;
		uint64_t CurrentBatchSize = 0;

		Renderer3DStats Stats;
	};

	static Renderer3DData* s_Data = nullptr;

	void Renderer3D::Init()
	{
		auto shaders =
			ShaderLibrary::BatchLoad({
				{"ShadowPass", "assets/_shaders/ShadowPass.glsl"},
				{"DeferredPass", "assets/_shaders/DeferredGBufferGen.glsl"},
				{"TonemapPass", "assets/_shaders/TonemapPass.glsl"},
				{"SkyboxPass", "assets/_shaders/SkyboxPass.glsl"},
				{"FlatShading", "assets/_shaders/FlatShading.glsl"},
				{"LightPass", "assets/_shaders/LightPass.glsl"}
			});

		s_Data = new Renderer3DData;

		//Directional light data
		s_Data->DirectionalLightBuffer.Construct([](uint32_t) {
			return UniformBuffer::Create(sizeof(DirectionalLightData));
		});

		s_Data->DirectionalLightSet.Construct([](uint32_t i) {
			DescriptorSetLayoutSpecification specs(
				{ {DescriptorType::UniformBuffer, ShaderStage_All} }
			);

			Ref<DescriptorSet> set = DescriptorSet::Create(specs);
			set->Update(s_Data->DirectionalLightBuffer[i], 0);
			return set;
		});

		//Scene data
		s_Data->SceneDataBuffer.Construct([](uint32_t) {
			return UniformBuffer::Create(sizeof(Kaidel::SceneData));
		});

		s_Data->SceneDataSet.Construct([](uint32_t i) {
			DescriptorSetLayoutSpecification specs(
				{ {DescriptorType::UniformBuffer, ShaderStage_All} }
			);

			Ref<DescriptorSet> set = DescriptorSet::Create(specs);
			set->Update(s_Data->SceneDataBuffer[i], 0);
			return set;
		});

		//Shadow pass
		RenderPassSpecification shadowRenderPass;
		shadowRenderPass.DepthStencil =
			RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, TextureSamples::x1, 
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store
			);
		s_Data->Shadow.Pass = RenderPass::Create(shadowRenderPass);
		
		s_Data->Shadow.Shader = shaders["ShadowPass"];

		GraphicsPipelineSpecification shadowGraphicsPipeline;
		shadowGraphicsPipeline.Input.Bindings =
		{
			{
				{ "a_Position", Format::RGB32F },
				{sizeof(MeshVertex) - sizeof(glm::vec3)}
			}
		};
		shadowGraphicsPipeline.RenderPass = s_Data->Shadow.Pass;
		shadowGraphicsPipeline.Primitive = PrimitiveTopology::TriangleList;
		shadowGraphicsPipeline.Rasterization.DepthClamp = true;
		shadowGraphicsPipeline.DepthStencil.DepthTest = true;
		shadowGraphicsPipeline.DepthStencil.DepthWrite = true;
		shadowGraphicsPipeline.Shader = s_Data->Shadow.Shader;

		s_Data->Shadow.Pipeline = GraphicsPipeline::Create(shadowGraphicsPipeline);

		FramebufferSpecification shadowFramebuffer;
		shadowFramebuffer.Width = DirectionalShadowmapWidth;
		shadowFramebuffer.Height = DirectionalShadowmapHeight;
		shadowFramebuffer.RenderPass = s_Data->Shadow.Pass;
		for (uint32_t i = 0; i < 4; ++i)
		{
			s_Data->Shadow.Cascades[i].Construct([&shadowFramebuffer](uint32_t) {
				return Framebuffer::Create(shadowFramebuffer);
			});
		}

		//Deferred pass
		RenderPassSpecification deferredRenderPass;
		deferredRenderPass.Colors = 
		{
			//Position
			RenderPassAttachment(Format::RGBA32F, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Normals
			RenderPassAttachment(Format::RGBA32F, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Albedo
			RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Metallic/roughness
			RenderPassAttachment(Format::RG32F, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Emissive
			RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
		};
		deferredRenderPass.DepthStencil = 
			RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store);
		s_Data->Deferred["Main"].Pass = RenderPass::Create(deferredRenderPass);

		s_Data->Deferred["Main"].Shader = shaders["DeferredPass"];

		//No pipeline, comes from material

		FramebufferSpecification deferredFramebuffer;
		deferredFramebuffer.Width = s_Data->ViewportSize["Main"].RenderWidth;
		deferredFramebuffer.Height = s_Data->ViewportSize["Main"].RenderHeight;
		deferredFramebuffer.RenderPass = s_Data->Deferred["Main"].Pass;
		s_Data->Deferred["Main"].GBuffers.Construct([&deferredFramebuffer](uint32_t) {
			return Framebuffer::Create(deferredFramebuffer);
		});

		//Screen pass
		RenderPassSpecification screenRenderPass;
		screenRenderPass.Colors =
		{
			RenderPassAttachment(Format::RGBA16F,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
		};
		screenRenderPass.DepthStencil = 
			RenderPassAttachment(Format::Depth32F,ImageLayout::DepthAttachmentOptimal, ImageLayout::DepthAttachmentOptimal,TextureSamples::x1,
				AttachmentLoadOp::Load, AttachmentStoreOp::Store);
		s_Data->Screen["Main"].Pass = RenderPass::Create(screenRenderPass);

		s_Data->Screen["Main"].Shader = shaders["LightPass"];

		GraphicsPipelineSpecification screenPipeline;
		screenPipeline.Primitive = PrimitiveTopology::TriangleList;
		screenPipeline.RenderPass = s_Data->Screen["Main"].Pass;
		screenPipeline.DepthStencil.DepthWrite = false;
		screenPipeline.DepthStencil.DepthTest = true;
		screenPipeline.Shader = s_Data->Screen["Main"].Shader;
		s_Data->Screen["Main"].Pipeline = GraphicsPipeline::Create(screenPipeline);

		FramebufferSpecification screenFramebuffer;
		screenFramebuffer.Width = s_Data->ViewportSize["Main"].RenderWidth;
		screenFramebuffer.Height = s_Data->ViewportSize["Main"].RenderHeight;
		screenFramebuffer.RenderPass = s_Data->Screen["Main"].Pass;
		s_Data->Screen["Main"].HDR.Construct([&screenFramebuffer](uint32_t i)	{
			screenFramebuffer.OverrideDepth = s_Data->Deferred["Main"].GBuffers[i]->GetDepthAttachment();
			return Framebuffer::Create(screenFramebuffer);
		});

		s_Data->Screen["Main"].Descriptors.Construct([&shaders, &deferredRenderPass](uint32_t i) {
			DescriptorSetPack pack = DescriptorSetPack(s_Data->Screen["Main"].Shader, { {0, RendererGlobals::GetSamplerSet() }, {2, {}}});

			for (uint32_t col = 0; col < deferredRenderPass.Colors.size(); ++col)
			{
				Ref<Texture> color = s_Data->Deferred["Main"].GBuffers[i]->GetColorAttachment(col);
				pack[1]->Update(color, {}, ImageLayout::ShaderReadOnlyOptimal, col);
			}

			pack[3]->Update(s_Data->DirectionalLightBuffer[i], 0);
			
			for (uint32_t cascade = 0; cascade < 4; ++cascade)
			{
				Ref<Texture> depth = s_Data->Shadow.Cascades[cascade][i]->GetDepthAttachment();
				pack[3]->Update(depth, {}, ImageLayout::ShaderReadOnlyOptimal, 1, cascade);
			}

			pack[4]->Update(s_Data->SceneDataBuffer[i], 0);

			return pack;
		});

		//Skybox pass
		s_Data->Skybox.Pass = s_Data->Screen["Main"].Pass;
		
		s_Data->Skybox.Shader = shaders["SkyboxPass"];
		GraphicsPipelineSpecification specs;
		specs.Input.Bindings = {
			{
				{"a_Position", Format::RGB32F}
			}
		};
		specs.Shader = s_Data->Skybox.Shader;
		specs.RenderPass = s_Data->Screen["Main"].Pass;
		specs.Primitive = PrimitiveTopology::TriangleList;
		specs.DepthStencil.DepthTest = true;
		specs.DepthStencil.DepthCompareOperator = LessOrEqual;
		s_Data->Skybox.Pipeline = GraphicsPipeline::Create(specs);

		s_Data->Skybox.Vertices = VertexBuffer::Create(SkyboxVertices, sizeof(SkyboxVertices));

		//Tonemap pass
		RenderPassSpecification tonemapRenderPass;
		tonemapRenderPass.Colors =
		{
			RenderPassAttachment(Format::RGBA8UN,ImageLayout::None,ImageLayout::ColorAttachmentOptimal,TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
		};
		s_Data->Tonemap["Main"].Pass = RenderPass::Create(tonemapRenderPass);

		s_Data->Tonemap["Main"].Shader = shaders["TonemapPass"];

		GraphicsPipelineSpecification tonemapPipeline;
		tonemapPipeline.Primitive = PrimitiveTopology::TriangleList;
		tonemapPipeline.RenderPass = s_Data->Tonemap["Main"].Pass;
		tonemapPipeline.Shader = s_Data->Tonemap["Main"].Shader;
		s_Data->Tonemap["Main"].Pipeline = GraphicsPipeline::Create(tonemapPipeline);

		FramebufferSpecification tonemapFramebuffer;
		tonemapFramebuffer.Width = s_Data->ViewportSize["Main"].RenderWidth;
		tonemapFramebuffer.Height = s_Data->ViewportSize["Main"].RenderHeight;
		tonemapFramebuffer.RenderPass = s_Data->Tonemap["Main"].Pass;
		s_Data->Tonemap["Main"].SDR.Construct([&tonemapFramebuffer](uint32_t) {
			return Framebuffer::Create(tonemapFramebuffer);
		});

		s_Data->Tonemap["Main"].Descriptors.Construct([](uint32_t i) {
			DescriptorSetPack pack = DescriptorSetPack(ShaderLibrary::GetNamedShader("TonemapPass"), { {0, RendererGlobals::GetSamplerSet()} });
			pack[1]->Update(s_Data->Screen["Main"].HDR[i]->GetColorAttachment(0), {}, ImageLayout::ShaderReadOnlyOptimal, 0);

			return pack;
		});

		s_Data->StorageBuffers["Main"].InstanceDataBuffer.Construct([](uint32_t) {
			return StorageBuffer::Create(nullptr, sizeof(glm::mat4));
			});

		s_Data->StorageBuffers["Main"].InstanceDataSet.Construct([](uint32_t i) {
			DescriptorSetLayoutSpecification specs({ {DescriptorType::StorageBuffer, ShaderStage_All} });
			Ref<DescriptorSet> set = DescriptorSet::Create(specs);

			set->Update(s_Data->StorageBuffers["Main"].InstanceDataBuffer[i], 0);
			return set;
			});

		for (uint32_t c = 0; c < 4; ++c)
		{
			s_Data->StorageBuffers["Main"].ShadowInstanceDataBuffer[c].Construct([](uint32_t) {
				return StorageBuffer::Create(nullptr, sizeof(glm::mat4));
				});

			s_Data->StorageBuffers["Main"].ShadowInstanceDataSet[c].Construct([c](uint32_t i) {
				DescriptorSetLayoutSpecification specs({ {DescriptorType::StorageBuffer, ShaderStage_All} });
				Ref<DescriptorSet> set = DescriptorSet::Create(specs);

				set->Update(s_Data->StorageBuffers["Main"].ShadowInstanceDataBuffer[c][i], 0);
				return set;
				});
		}

		//CreatePerViewportData("Mat", 640, 480);
	}

	void Renderer3D::Shutdown()
	{
		delete s_Data;
	}

	void Renderer3D::Begin(Ref<Texture2D> output,
		const Visibility& vis,
		const SceneData& sceneData, 
		const DirectionalLightData& lightData, 
		const std::string& viewportName /*= "Main"*/)
	{
		s_Data->SceneData = sceneData;
		s_Data->SceneDataBuffer->Get()->SetData(&s_Data->SceneData, sizeof(Kaidel::SceneData));
		
		s_Data->CurrentViewport = viewportName;

		if (NeedsRecreation(output, s_Data->CurrentViewport))
		{
			CreatePerViewportData(s_Data->CurrentViewport, output->GetTextureSpecification().Width, output->GetTextureSpecification().Height);
		}

		s_Data->Output = output;

		s_Data->RendererState = Renderer3DState::None;

		s_Data->m_LastMat = {};

		s_Data->Stats = {};

		{
			//SCOPED_ACCU_TIMER("SceneRenderer::Upload");

			UploadInstanceData(vis.Instances, 
				s_Data->StorageBuffers[s_Data->CurrentViewport].InstanceDataBuffer, 
				s_Data->StorageBuffers[s_Data->CurrentViewport].InstanceDataSet);

			for (uint32_t i = 0; i < 4; ++i)
			{
				const ShadowVisibilityCascade& cascade = vis.Directional.Cascades[i];
				UploadInstanceData(cascade.Instances, 
					s_Data->StorageBuffers[s_Data->CurrentViewport].ShadowInstanceDataBuffer[i],
					s_Data->StorageBuffers[s_Data->CurrentViewport].ShadowInstanceDataSet[i]);
			}
		}

		s_Data->LightData = lightData;
		s_Data->DirectionalLightBuffer->Get()->SetData(&s_Data->LightData, sizeof(DirectionalLightData));
	}

	void Renderer3D::BeginShadow(const DirectionalLightData& lightData)
	{
		
	}

	void Renderer3D::BeginDirectionalShadow(uint32_t cascade)
	{
		RenderCommand::BindGraphicsPipeline(s_Data->Shadow.Pipeline);
		RenderCommand::BindPushConstants(s_Data->Shadow.Shader, 0, s_Data->LightData.ViewProjection[cascade]);
		RenderCommand::BindDescriptorSet(s_Data->Shadow.Shader, 
			s_Data->StorageBuffers[s_Data->CurrentViewport].ShadowInstanceDataSet[cascade], 0);

		s_Data->Shadow.Pass->SetDepthClearValue(AttachmentDepthStencilClearValue(1.0f, 0));
		RenderCommand::BeginRenderPass(s_Data->Shadow.Pass, s_Data->Shadow.Cascades[cascade], {});
		
		RenderCommand::SetViewport(DirectionalShadowmapWidth, DirectionalShadowmapHeight, 0, 0);
		RenderCommand::SetScissor(DirectionalShadowmapWidth, DirectionalShadowmapHeight, 0, 0);

		s_Data->RendererState = Renderer3DState::Shadow;

		s_Data->Shadow.CurrentCascade = cascade;

		s_Data->InstanceOffset = 0;
		s_Data->CurrentBatchSize = 0;

		s_Data->m_LastParams = {};
	}


	void Renderer3D::BeginColor(Ref<EnvironmentMap> environment)
	{
		auto& deferred = s_Data->Deferred[s_Data->CurrentViewport];

		for (uint32_t i = 0; i < deferred.Pass->GetSpecification().Colors.size(); ++i)
		{
			deferred.Pass->SetColorClearValue(AttachmentColorClearValue(glm::vec4(0.0f)), i);
		}

		deferred.Pass->SetDepthClearValue(AttachmentDepthStencilClearValue(1.0f, 0));

		RenderCommand::BeginRenderPass(deferred.Pass, deferred.GBuffers, {});

		auto& viewport = s_Data->ViewportSize[s_Data->CurrentViewport];
		RenderCommand::SetViewport(viewport.RenderWidth, viewport.RenderHeight, 0, 0);
		RenderCommand::SetScissor(viewport.RenderWidth, viewport.RenderHeight, 0, 0);

		s_Data->RendererState = Renderer3DState::Color;

		s_Data->Environment = environment;

		s_Data->InstanceOffset = 0;
		s_Data->CurrentBatchSize = 0;

		s_Data->m_LastParams = {};
	}

	void Renderer3D::UploadInstanceData(const std::vector<InstanceData>& instances, Ref<StorageBuffer> buffer, Ref<DescriptorSet> set)
	{
		if (instances.empty())
			return;

		InstanceData* startInstance = 
			(InstanceData*)buffer->Reserve(instances.size() * sizeof(InstanceData));

		std::memcpy(startInstance, instances.data(), instances.size() * sizeof(InstanceData));

		buffer->UploadReserve(startInstance, instances.size() * sizeof(InstanceData));
		set->Update(buffer, 0);
	}

	void Renderer3D::Draw(const Renderer3DRenderParams& params, Ref<Material> material)
	{

		//SCOPED_ACCU_TIMER("Draw");
		

		switch (s_Data->RendererState)
		{
		case Renderer3DState::Shadow:
		{
			if (s_Data->m_LastParams != params)
			{
				DrawBatch(true);
				s_Data->m_LastParams = params;
			}
		} break;
		case Renderer3DState::Color:
		{
			if (s_Data->m_LastMat != material || s_Data->m_LastParams != params)
			{
				DrawBatch(true);
				s_Data->m_LastMat = material;
				s_Data->m_LastParams = params;
			}
		} break;
		default:
			return;
		}
		++s_Data->CurrentBatchSize;
	}


	void Renderer3D::BeginSubmesh()
	{
		DrawBatch(true);
	}

	void Renderer3D::DrawSubmesh(const Renderer3DRenderParams& params, Ref<Material> material)
	{
		//SCOPED_ACCU_TIMER("DrawSubmesh");
		s_Data->m_LastMat = material;
		s_Data->m_LastParams = params;

		s_Data->CurrentBatchSize = 1;

		DrawBatch(false);
	}


	void Renderer3D::EndSubmesh()
	{
		Flush();
	}

	void Renderer3D::Flush()
	{
		s_Data->InstanceOffset += s_Data->CurrentBatchSize;
		s_Data->CurrentBatchSize = 0;
	}

	void Renderer3D::EndColor()
	{
		DrawBatch(true);

		s_Data->RendererState = Renderer3DState::None;

		RenderCommand::EndRenderPass();
	}

	void Renderer3D::EndShadow()
	{
		DrawBatch(true);

		s_Data->RendererState = Renderer3DState::None;

		RenderCommand::EndRenderPass();
	}


	void Renderer3D::End()
	{

		ImageMemoryBarrier shadowBarriers[4];
		for (uint32_t i = 0; i < 4; ++i)
		{
			shadowBarriers[i] = 
				ImageMemoryBarrier(
					s_Data->Shadow.Cascades[i]->Get()->GetDepthAttachment(),
					ImageLayout::ShaderReadOnlyOptimal, AccessFlags_DepthStencilWrite, AccessFlags_ShaderRead);
		}
		
		ImageMemoryBarrier gbufferBarriers[6];
		for (uint32_t i = 0; i < 5; ++i)
		{
			gbufferBarriers[i] =
				ImageMemoryBarrier(
					s_Data->Deferred[s_Data->CurrentViewport].GBuffers->Get()->GetColorAttachment(i),
					ImageLayout::ShaderReadOnlyOptimal, AccessFlags_ColorAttachmentWrite, AccessFlags_ShaderRead);
		}
		gbufferBarriers[5] =
			ImageMemoryBarrier(
				s_Data->Deferred[s_Data->CurrentViewport].GBuffers->Get()->GetDepthAttachment(),
				ImageLayout::DepthAttachmentOptimal, AccessFlags_DepthStencilWrite, AccessFlags_DepthStencilRead
			);

		RenderCommand::PipelineBarrier(
			PipelineStages_LateFragmentTests, PipelineStages_FragmentShader,
			{}, {}, std::initializer_list<ImageMemoryBarrier>(shadowBarriers, shadowBarriers + 4)
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput | PipelineStages_LateFragmentTests, PipelineStages_EarlyFragmentTests | PipelineStages_FragmentShader,
			{}, {}, std::initializer_list<ImageMemoryBarrier>(gbufferBarriers, gbufferBarriers + 6));

		auto& screen = s_Data->Screen[s_Data->CurrentViewport];
		auto& viewport = s_Data->ViewportSize[s_Data->CurrentViewport];

		RenderCommand::SetViewport(viewport.RenderWidth, viewport.RenderHeight);

		RenderCommand::BeginRenderPass(screen.Pass, screen.HDR, {});
	
		RenderCommand::BindGraphicsPipeline(screen.Pipeline);

		RenderCommand::BindVertexBuffers({ s_Data->Skybox.Vertices }, { 0 });

		RenderCommand::BindDescriptorSet(screen.Shader, RendererGlobals::GetSamplerSet(), 0);
		RenderCommand::BindDescriptorSet(screen.Shader, screen.Descriptors->GetSet(1), 1);
		RenderCommand::BindDescriptorSet(screen.Shader, s_Data->Environment->GetSet(), 2);
		RenderCommand::BindDescriptorSet(screen.Shader, screen.Descriptors->GetSet(3), 3);
		RenderCommand::BindDescriptorSet(screen.Shader, screen.Descriptors->GetSet(4), 4);
		RenderCommand::Draw(3, 1, 0, 0);

		{
			RenderCommand::BindGraphicsPipeline(s_Data->Skybox.Pipeline);
			RenderCommand::BindVertexBuffers({ s_Data->Skybox.Vertices }, { 0 });
			RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("SkyboxPass"), RendererGlobals::GetEnvironmentMap()->GetSet(), 0);
			RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("SkyboxPass"), 0, s_Data->SceneData.Proj, s_Data->SceneData.View);
			RenderCommand::Draw(36, 1, 0, 0);
		}
		
		RenderCommand::EndRenderPass();

		ImageMemoryBarrier hdrBarrier(
			screen.HDR->Get()->GetColorAttachment(0), 
			ImageLayout::ShaderReadOnlyOptimal, 
			AccessFlags_ColorAttachmentWrite, AccessFlags_ShaderRead
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput, PipelineStages_FragmentShader,
			{}, {}, { hdrBarrier }
		);

		auto& tonemap = s_Data->Tonemap[s_Data->CurrentViewport];

		RenderCommand::BindGraphicsPipeline(tonemap.Pipeline);

		RenderCommand::BindDescriptorSet(screen.Shader, RendererGlobals::GetSamplerSet(), 0);
		RenderCommand::BindDescriptorSet(tonemap.Shader, tonemap.Descriptors->GetSet(1), 1);

		RenderCommand::BeginRenderPass(tonemap.Pass, tonemap.SDR, {});

		RenderCommand::Draw(3, 1, 0, 0);

		RenderCommand::EndRenderPass();


		ImageMemoryBarrier ldrBarrier(
			tonemap.SDR->Get()->GetColorAttachment(0),
			ImageLayout::TransferSrcOptimal,
			AccessFlags_ColorAttachmentWrite, AccessFlags_TransferRead
		);

		ImageMemoryBarrier outputBeginCopyBarrier(
			s_Data->Output,
			ImageLayout::TransferDstOptimal,
			AccessFlags_None, AccessFlags_TransferWrite
		);
		outputBeginCopyBarrier.OldLayout = ImageLayout::None;


		RenderCommand::PipelineBarrier(
			PipelineStages_ColorAttachmentOutput, PipelineStages_Transfer,
			{}, {}, { ldrBarrier, outputBeginCopyBarrier }
		);

		RenderCommand::CopyTexture(tonemap.SDR->Get()->GetColorAttachment(0), 0, 0, s_Data->Output, 0, 0);

		ImageMemoryBarrier outputEndCopyBarrier(
			s_Data->Output,
			ImageLayout::ShaderReadOnlyOptimal,
			AccessFlags_TransferWrite, AccessFlags_ShaderRead
		);

		RenderCommand::PipelineBarrier(
			PipelineStages_Transfer, PipelineStages_FragmentShader,
			{}, {}, { outputEndCopyBarrier }
		);
	}


	Kaidel::Ref<Kaidel::RenderPass> Renderer3D::GetDeferredPassRenderPass()
	{
		return s_Data->Deferred["Main"].Pass;
	}


	Kaidel::Renderer3DStats Renderer3D::GetStats()
	{
		return s_Data->Stats;
	}

	void Renderer3D::CreatePerViewportData(const std::string& viewportName, uint32_t viewportWidth, uint32_t viewportHeight)
	{

		auto& data = s_Data;
		KD_INFO("Renderer3D Creating Viewport {}", viewportName);
		KD_INFO("{}", data->ViewportSize["Main"].RenderWidth);

		s_Data->ViewportSize[viewportName].RenderWidth = viewportWidth;
		s_Data->ViewportSize[viewportName].RenderHeight = viewportHeight;

		//Deferred pass
		RenderPassSpecification deferredRenderPass;
		deferredRenderPass.Colors =
		{
			//Position
			RenderPassAttachment(Format::RGBA32F, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Normals
			RenderPassAttachment(Format::RGBA32F, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Albedo
			RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Metallic/roughness
			RenderPassAttachment(Format::RG32F, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store),
			//Emissive
			RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store)
		};
		deferredRenderPass.DepthStencil =
			RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, TextureSamples::x1,
				AttachmentLoadOp::Clear, AttachmentStoreOp::Store);
		s_Data->Deferred[viewportName].Pass = s_Data->Deferred["Main"].Pass;

		s_Data->Deferred[viewportName].Shader = s_Data->Deferred["Main"].Shader;

		//No pipeline, comes from material

		FramebufferSpecification deferredFramebuffer;
		deferredFramebuffer.Width = s_Data->ViewportSize[viewportName].RenderWidth;
		deferredFramebuffer.Height = s_Data->ViewportSize[viewportName].RenderHeight;
		deferredFramebuffer.RenderPass = s_Data->Deferred[viewportName].Pass;
		s_Data->Deferred[viewportName].GBuffers.Construct([&deferredFramebuffer](uint32_t) {
			return Framebuffer::Create(deferredFramebuffer);
		});

		//Screen pass
		s_Data->Screen[viewportName].Pass = s_Data->Screen["Main"].Pass;

		s_Data->Screen[viewportName].Shader = s_Data->Screen["Main"].Shader;

		s_Data->Screen[viewportName].Pipeline = s_Data->Screen["Main"].Pipeline;

		FramebufferSpecification screenFramebuffer;
		screenFramebuffer.Width = s_Data->ViewportSize[viewportName].RenderWidth;
		screenFramebuffer.Height = s_Data->ViewportSize[viewportName].RenderHeight;
		screenFramebuffer.RenderPass = s_Data->Screen[viewportName].Pass;
		s_Data->Screen[viewportName].HDR.Construct([&screenFramebuffer, &viewportName](uint32_t i) {
			screenFramebuffer.OverrideDepth = s_Data->Deferred[viewportName].GBuffers[i]->GetDepthAttachment();
			return Framebuffer::Create(screenFramebuffer);
		});

		s_Data->Screen[viewportName].Descriptors.Construct([&viewportName, &deferredRenderPass](uint32_t i) {
			DescriptorSetPack pack = DescriptorSetPack(s_Data->Screen[viewportName].Shader, { {0, RendererGlobals::GetSamplerSet() }, {2, {}} });

			for (uint32_t col = 0; col < deferredRenderPass.Colors.size(); ++col)
			{
				Ref<Texture> color = s_Data->Deferred[viewportName].GBuffers[i]->GetColorAttachment(col);
				pack[1]->Update(color, {}, ImageLayout::ShaderReadOnlyOptimal, col);
			}

			pack[3]->Update(s_Data->DirectionalLightBuffer[i], 0);

			for (uint32_t cascade = 0; cascade < 4; ++cascade)
			{
				Ref<Texture> depth = s_Data->Shadow.Cascades[cascade][i]->GetDepthAttachment();
				pack[3]->Update(depth, {}, ImageLayout::ShaderReadOnlyOptimal, 1, cascade);
			}

			pack[4]->Update(s_Data->SceneDataBuffer[i], 0);

			return pack;
		});

		//Tonemap pass
		s_Data->Tonemap[viewportName].Pass = s_Data->Tonemap["Main"].Pass;

		s_Data->Tonemap[viewportName].Shader = s_Data->Tonemap["Main"].Shader;

		s_Data->Tonemap[viewportName].Pipeline = s_Data->Tonemap["Main"].Pipeline;

		FramebufferSpecification tonemapFramebuffer;
		tonemapFramebuffer.Width = s_Data->ViewportSize[viewportName].RenderWidth;
		tonemapFramebuffer.Height = s_Data->ViewportSize[viewportName].RenderHeight;
		tonemapFramebuffer.RenderPass = s_Data->Tonemap[viewportName].Pass;
		s_Data->Tonemap[viewportName].SDR.Construct([&tonemapFramebuffer](uint32_t) {
			return Framebuffer::Create(tonemapFramebuffer);
		});

		s_Data->Tonemap[viewportName].Descriptors.Construct([&viewportName](uint32_t i) {
			DescriptorSetPack pack = DescriptorSetPack(s_Data->Tonemap[viewportName].Shader, {{0, RendererGlobals::GetSamplerSet()}});
			pack[1]->Update(s_Data->Screen[viewportName].HDR[i]->GetColorAttachment(0), {}, ImageLayout::ShaderReadOnlyOptimal, 0);

			return pack;
		});

		s_Data->StorageBuffers[viewportName].InstanceDataBuffer.Construct([](uint32_t) {
			return StorageBuffer::Create(nullptr, sizeof(glm::mat4));
			});

		s_Data->StorageBuffers[viewportName].InstanceDataSet.Construct([&viewportName](uint32_t i) {
			DescriptorSetLayoutSpecification specs({ {DescriptorType::StorageBuffer, ShaderStage_All} });
			Ref<DescriptorSet> set = DescriptorSet::Create(specs);

			set->Update(s_Data->StorageBuffers[viewportName].InstanceDataBuffer[i], 0);
			return set;
			});

		for (uint32_t c = 0; c < 4; ++c)
		{
			s_Data->StorageBuffers[viewportName].ShadowInstanceDataBuffer[c].Construct([](uint32_t) {
				return StorageBuffer::Create(nullptr, sizeof(glm::mat4));
				});

			s_Data->StorageBuffers[viewportName].ShadowInstanceDataSet[c].Construct([&viewportName, c](uint32_t i) {
				DescriptorSetLayoutSpecification specs({ {DescriptorType::StorageBuffer, ShaderStage_All} });
				Ref<DescriptorSet> set = DescriptorSet::Create(specs);

				set->Update(s_Data->StorageBuffers[viewportName].ShadowInstanceDataBuffer[c][i], 0);
				return set;
			});
		}


	}


	void Renderer3D::DrawBatch(bool increment)
	{

		//SCOPED_ACCU_TIMER("Renderer3D::DrawBatch");
		if (!s_Data->CurrentBatchSize)
			return;
		
		auto& params = s_Data->m_LastParams;

		++s_Data->Stats.DrawCalls;

		RenderCommand::BindVertexBuffers({ params.VB }, { 0 });
		RenderCommand::BindIndexBuffer(params.IB, 0);
		
		switch (s_Data->RendererState)
		{
		case Renderer3DState::Shadow:
		{
			RenderCommand::BindPushConstants(s_Data->Shadow.Shader, sizeof(glm::mat4), (uint32_t)s_Data->InstanceOffset);
			RenderCommand::DrawIndexed(params.IndexCount, params.VertexCount, s_Data->CurrentBatchSize,
				params.IndexOffset, params.VertexOffset, 0);
		} break;
		case Renderer3DState::Color:
		{
			s_Data->Stats.InstanceCount += s_Data->CurrentBatchSize;
			s_Data->Stats.VertexCount += params.VertexCount;
			s_Data->Stats.IndexCount += params.IndexCount;

			
			s_Data->m_LastMat->BindPipeline();
			s_Data->m_LastMat->BindValues();
			s_Data->m_LastMat->BindSceneData(s_Data->SceneDataSet);
			s_Data->m_LastMat->BindInstanceData(s_Data->StorageBuffers[s_Data->CurrentViewport].InstanceDataSet);

			RenderCommand::BindPushConstants(s_Data->m_LastMat->GetShader(), 0, (uint32_t)s_Data->InstanceOffset);
			RenderCommand::DrawIndexed(params.IndexCount, params.VertexCount, s_Data->CurrentBatchSize,
				params.IndexOffset, params.VertexOffset, 0);

		} break;
		}

		

		if (!increment)
			return;

		s_Data->InstanceOffset += s_Data->CurrentBatchSize;
		s_Data->CurrentBatchSize = 0;
	}

	bool Renderer3D::NeedsRecreation(Ref<Texture2D> output, const std::string& viewportName)
	{
		const auto& specs = output->GetTextureSpecification();
		return specs.Width && specs.Height && (specs.Width != s_Data->ViewportSize[viewportName].RenderWidth ||
			specs.Height != s_Data->ViewportSize[viewportName].RenderHeight);
	}
}
