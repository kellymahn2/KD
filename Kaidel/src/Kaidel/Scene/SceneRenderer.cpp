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

#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
namespace Kaidel {

	struct Material {
		glm::vec4 Albedo;
		float Roughness;
		float Specular;
		float Metallic;
	};


	static std::mutex s_MeshRenderingMutex;
	static std::mutex s_MeshPushingMutex;
	TextureSamples samples = TextureSamples::x4;

	static struct Data {
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<IndexBuffer> CubeIndexBuffer;
		PerFrameResource<Ref<UniformBuffer>> CameraBuffers;
		PerFrameResource<Ref<DescriptorSet>> CameraBufferSets;
		Ref<UniformBuffer> MaterialBuffer;
		Ref<GraphicsPipeline> CubePipeline;
		Ref<RenderPass> RenderPass;
		PerFrameResource<Ref<Framebuffer>> Framebuffer;
		TextureSamples Samples;
		uint32_t Width = 1280, Height = 720;
	}* s_Data;

	static glm::mat4 s_Model = glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-10)) 
		* glm::rotate(glm::mat4(1.0f),45.0f,glm::vec3(0,1,0))
		* glm::rotate(glm::mat4(1.0f),45.0f,glm::vec3(0,0,1));

	const std::vector<glm::vec3> vertices = {
		// Front face
		{glm::vec3(-0.5f, -0.5f,  0.5f)},
		{glm::vec3(0.5f, -0.5f,  0.5f)},
		{glm::vec3(0.5f,  0.5f,  0.5f)},
		{glm::vec3(-0.5f,  0.5f,  0.5f)},
		// Back face
		{glm::vec3(-0.5f, -0.5f, -0.5f)},
		{glm::vec3(0.5f, -0.5f, -0.5f)},
		{glm::vec3(0.5f,  0.5f, -0.5f)},
		{glm::vec3(-0.5f,  0.5f, -0.5f)},
		// Left face
		{glm::vec3(-0.5f, -0.5f,  0.5f)},
		{glm::vec3(-0.5f, -0.5f, -0.5f)},
		{glm::vec3(-0.5f,  0.5f, -0.5f)},
		{glm::vec3(-0.5f,  0.5f,  0.5f)},
		// Right face
		{glm::vec3(0.5f, -0.5f,  0.5f)},
		{glm::vec3(0.5f, -0.5f, -0.5f)},
		{glm::vec3(0.5f,  0.5f, -0.5f)},
		{glm::vec3(0.5f,  0.5f,  0.5f)},
		// Top face
		{glm::vec3(-0.5f,  0.5f,  0.5f)},
		{glm::vec3(0.5f,  0.5f,  0.5f)},
		{glm::vec3(0.5f,  0.5f, -0.5f)},
		{glm::vec3(-0.5f,  0.5f, -0.5f)},
		// Bottom face
		{glm::vec3(-0.5f, -0.5f,  0.5f)},
		{glm::vec3(0.5f, -0.5f,  0.5f)},
		{glm::vec3(0.5f, -0.5f, -0.5f)},
		{glm::vec3(-0.5f, -0.5f, -0.5f)},
	};

	const std::vector<unsigned short> indices = {
		// Front face
		0, 1, 2,
		2, 3, 0,
		// Back face
		5, 4, 7,
		7, 6, 5,
		// Left face
		9, 8, 11,
		11, 10, 9,
		// Right face
		12, 13, 14,
		14, 15, 12,
		// Top face
		16, 17, 18,
		18, 19, 16,
		// Bottom face
		23, 22, 21,
		21, 20, 23,
	};

	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
		if (!s_Data)
			s_Data = new Data;
		

		s_Data->CubeVertexBuffer = VertexBuffer::Create(vertices.size() * sizeof(glm::vec3));
		s_Data->CubeIndexBuffer = IndexBuffer::Create(indices.data(), indices.size() * sizeof(uint16_t), IndexType::Uint16);

		for (auto& camera : s_Data->CameraBuffers) {
			camera = UniformBuffer::Create(sizeof(glm::mat4));
		}

		uint32_t i = 0;
		for (auto& set : s_Data->CameraBufferSets) {
			DescriptorSetSpecification specs{};
			specs.Set = 0;
			specs.Shader = ShaderLibrary::LoadShader("Cube", "assets/_shaders/Cube.shader");
			DescriptorValues val(s_Data->CameraBuffers.GetResources()[i]);
			specs.Values.push_back(val);
			set = DescriptorSet::Create(specs);
			++i;
		}

		{
			RenderPassSpecification specs{};
			{

				RenderPassAttachment attach =
					RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, samples);
				attach.LoadOp = AttachmentLoadOp::Clear;
				attach.StoreOp = AttachmentStoreOp::Store;
				specs.Colors.push_back(attach);
			}
			{
				RenderPassAttachment attach =
					RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, samples);
				attach.LoadOp = AttachmentLoadOp::Clear;
				attach.StoreOp = AttachmentStoreOp::Store;
				specs.DepthStencil = attach;
			}
			s_Data->RenderPass = RenderPass::Create(specs);
		}

		{
			FramebufferSpecification specs;
			specs.Width = 1280;
			specs.Height = 720;
			specs.RenderPass = s_Data->RenderPass;
			for (auto& fb : s_Data->Framebuffer) {
				fb = Framebuffer::Create(specs);
			}
		}

		{
			GraphicsPipelineSpecification specs;
			
			specs.Input.Bindings.push_back(VertexInputBinding({
				{"a_Position",Format::RGB32F}
			}));
			specs.Multisample.Samples = samples;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.Rasterization.FrontCCW = true;
			specs.Rasterization.CullMode = PipelineCullMode::None;
			specs.Shader = ShaderLibrary::GetNamedShader("Cube");
			specs.RenderPass = s_Data->RenderPass;
			specs.Subpass = 0;
			specs.DepthStencil.DepthTest = true;
			specs.DepthStencil.DepthWrite = true;
			specs.DepthStencil.DepthCompareOperator = CompareOp::LessOrEqual;
			s_Data->CubePipeline = GraphicsPipeline::Create(specs);
		}
		s_Data->Samples = samples;
	}
	void SceneRenderer::Reset()
	{ 
	}

	void SceneRenderer::Render(Ref<Texture2D> outputBuffer, const glm::mat4& cameraViewProj, const glm::vec3& cameraPos)
	{
		if (samples != s_Data->Samples || 
			outputBuffer->GetTextureSpecification().Width != s_Data->Framebuffer->Get()->GetSpecification().Width || 
			outputBuffer->GetTextureSpecification().Height != s_Data->Framebuffer->Get()->GetSpecification().Height) {
			{
				RenderPassSpecification specs{};
				{

					RenderPassAttachment attach =
						RenderPassAttachment(Format::RGBA8UN, ImageLayout::None, ImageLayout::ColorAttachmentOptimal, samples);
					attach.LoadOp = AttachmentLoadOp::Clear;
					attach.StoreOp = AttachmentStoreOp::Store;
					specs.Colors.push_back(attach);
				}
				{
					RenderPassAttachment attach =
						RenderPassAttachment(Format::Depth32F, ImageLayout::None, ImageLayout::DepthAttachmentOptimal, samples);
					attach.LoadOp = AttachmentLoadOp::Clear;
					attach.StoreOp = AttachmentStoreOp::Store;
					specs.DepthStencil = attach;
				}
				s_Data->RenderPass = RenderPass::Create(specs);
			}

			{
				FramebufferSpecification specs;
				specs.Width = outputBuffer->GetTextureSpecification().Width;
				specs.Height = outputBuffer->GetTextureSpecification().Height;
				specs.RenderPass = s_Data->RenderPass;
				*s_Data->Framebuffer = Framebuffer::Create(specs);
			}

			{
				GraphicsPipelineSpecification specs;

				specs.Input.Bindings.push_back(VertexInputBinding({
					{"a_Position",Format::RGB32F}
					}));
				specs.Multisample.Samples = samples;
				specs.Primitive = PrimitiveTopology::TriangleList;
				specs.Rasterization.FrontCCW = true;
				specs.Rasterization.CullMode = PipelineCullMode::None;
				specs.Shader = ShaderLibrary::GetNamedShader("Cube");
				specs.RenderPass = s_Data->RenderPass;
				specs.Subpass = 0;
				specs.DepthStencil.DepthTest = true;
				specs.DepthStencil.DepthWrite = true;
				specs.DepthStencil.DepthCompareOperator = CompareOp::LessOrEqual;
				s_Data->CubePipeline = GraphicsPipeline::Create(specs);
			}
			s_Data->Samples = samples;
			s_Data->Width = outputBuffer->GetTextureSpecification().Width;
			s_Data->Height = outputBuffer->GetTextureSpecification().Height;
		}

		//glm::mat4 proj = glm::perspective(glm::radians(60.0f), 1280.0f / 720.0f, 0.1f, 1000.0f);
		Scene& activeScene = *static_cast<Scene*>(m_Context);

		s_Data->CameraBuffers->Get()->SetData(&cameraViewProj, sizeof(glm::mat4));
		s_Data->CubeVertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(glm::vec3));
		RenderCommand::BindVertexBuffers({ s_Data->CubeVertexBuffer }, { 0 });
		RenderCommand::BindIndexBuffer(s_Data->CubeIndexBuffer, 0);
		RenderCommand::BindGraphicsPipeline(s_Data->CubePipeline);
		RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("Cube"),0,(uint8_t*)&s_Model, sizeof(glm::mat4));
		RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("Cube"), *s_Data->CameraBufferSets, 0);
		RenderCommand::SetViewport(s_Data->Framebuffer->Get()->GetSpecification().Width, s_Data->Framebuffer->Get()->GetSpecification().Height);
		RenderCommand::SetScissor(s_Data->Framebuffer->Get()->GetSpecification().Width, s_Data->Framebuffer->Get()->GetSpecification().Height);
		RenderCommand::BeginRenderPass(s_Data->RenderPass, *s_Data->Framebuffer, 
			{ AttachmentColorClearValue(glm::vec4{ 0.0f,0.0f,0.0f,0.0f }) ,
			  AttachmentDepthStencilClearValue(1.0f,0)});
		RenderCommand::DrawIndexed(indices.size(), vertices.size(), 1, 0, 0, 0);
		RenderCommand::EndRenderPass();

		{
			ImageMemoryBarrier barrier = 
				ImageMemoryBarrier(
					s_Data->Framebuffer->Get()->GetColorAttachment(0),
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

		RenderCommand::ResolveTexture(s_Data->Framebuffer->Get()->GetColorAttachment(0), 0, 0, outputBuffer, 0, 0);

		//Renderer2D::Begin(cameraViewProj, outputBuffer);
		//Sprites
		//{
		//	auto view = activeScene->m_Registry.view<TransformComponent, SpriteRendererComponent>();
		//	for (auto e : view) {
		//		auto [tc, src] = view.get<TransformComponent, SpriteRendererComponent>(e);
		//		
		//		ImageSubresource resource = TextureLibrary::GetTextureByLayer(0);
		//
		//		SamplingRegion region{};
		//		region.Layer = resource.Layer;
		//		region.UV0 = resource.TopLeft;
		//		region.UV1 = resource.BottomRight;
		//
		//		Renderer2D::DrawSprite(tc.GetTransform(), glm::vec4{1.0f,1.0f,1.0f,1.0f}, region);
		//	}
		//}
		//Renderer2D::End();

#if 0


		Scene* activeScene = static_cast<Scene*>(m_Context);

		auto& reg = activeScene->m_Registry;
		//3D
		{
			{
				auto view = activeScene->m_Registry.view<TransformComponent, SpotLightComponent>();
				for (auto e : view) {
					auto [tc, slc] = view.get<TransformComponent, SpotLightComponent>(e);
					auto& light = slc.Light->GetLight();

					light.Position = glm::vec4(tc.Translation, 1.0f);
					light.LightViewProjection = glm::infinitePerspective(2.0f * glm::acos(light.CutOffAngle), 1.0f, .5f) * 
						glm::lookAt(glm::vec3(light.Position), glm::vec3(light.Position + glm::normalize(light.Direction)), glm::vec3(0.0f, 1.0f, 0.0f));
				}
			}

			GlobalRendererData->CameraBuffer.Position = cameraPos;
			GlobalRendererData->CameraBuffer.ViewProj = cameraViewProj;

			GlobalRendererData->CameraUniformBuffer->SetData(&GlobalRendererData->CameraBuffer, sizeof(GlobalRenderer3DData::CameraBufferData));
			SpotLight::SetLights();

			//Shadows
			{

				reg.view<TransformComponent, MeshComponent>().each([](auto e, TransformComponent& tc, MeshComponent& mc) {
					if (!mc.Mesh)
						return;
					auto& dd = mc.Mesh->GetShadowDrawData();


					auto bvi = dd.Reserve(1);
					bvi[0] = tc.GetTransform();
				});
			}

		}

		//2D
		{

			Renderer2DBeginData data;
			data.OutputBuffer = _2DOutputFramebuffer;
			data.CameraVP = cameraViewProj;
			{
				Renderer2D::Begin(data);
				//Sprites
				{
					auto view = activeScene->m_Registry.view<TransformComponent, SpriteRendererComponent>();
					for (auto e : view) {
						auto [tc, src] = view.get<TransformComponent, SpriteRendererComponent>(e);
						Renderer2D::DrawSprite(tc.GetTransform(), {});
					}
				}

				Renderer2D::FlushSprites();
				//Particles
				{
					auto view = activeScene->m_Registry.view<TransformComponent, ParticleSystemComponent>();
					for (auto e : view) {
						auto [tc, psc] = view.get<TransformComponent, ParticleSystemComponent>(e);
						if (psc.PS) {
							auto& transform = tc.GetTransform();
							for (auto& particle : *psc.PS) {

								glm::vec3 pos = 0.5f * psc.PS->GetSpecification().ParticleAcceleration * particle.AliveTime * particle.AliveTime + particle.InitialVelocity * particle.AliveTime + particle.InitialPostition;
								Renderer2D::DrawSprite(transform*glm::translate(glm::mat4(1.0f), pos), {});
							}
						}
					}
				}

				Renderer2D::End();
			}
		}
#endif
	}

}
