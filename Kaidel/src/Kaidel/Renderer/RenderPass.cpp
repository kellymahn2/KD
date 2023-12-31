#include "KDpch.h"
#include "RenderPass.h"
#include "Kaidel/Scene/SceneRenderer.h"
#include "Kaidel/Renderer/GeometryFlusher.h"
#include "Kaidel/Renderer/ShadowFlusher.h"
#include "Platform/OpenGL/OpenGLTexture.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "Renderer3D.h"
namespace Kaidel {


	static SceneRenderer s_SceneRenderer;

	struct CountInfo {
		int PointLightCount;
		int SpotLightCount;
	};

	static void BindLights(void* scene) {
		static Ref<UniformBuffer> s_LightingDataUB = UniformBuffer::Create(sizeof(CountInfo), 1);

		//Point Lights
		{
			auto view = static_cast<Scene*>(scene)->m_Registry.view<TransformComponent, PointLightComponent>();
			for (auto e : view) {
				auto [tc, plc] = view.get<TransformComponent, PointLightComponent>(e);
				auto& light = plc.Light->GetLight();
				light.Position = tc.Translation;
			}
		}
		//Spot Lights
		{

			auto view = static_cast<Scene*>(scene)->m_Registry.view<TransformComponent, SpotLightComponent>();

			for (auto e : view) {

				auto [tc, slc] = view.get<TransformComponent, SpotLightComponent>(e);
				auto& light = slc.Light->GetLight();
				light.Position = tc.Translation;
			}
		}


		DirectionalLight::SetLights();
		PointLight::SetLights();
		SpotLight::SetLights();
		Material::SetMaterials();
		CountInfo countInfo;
		countInfo.PointLightCount = PointLight::GetLightCount();
		countInfo.SpotLightCount = SpotLight::GetLightCount();
		s_LightingDataUB->SetData(&countInfo, sizeof(countInfo));
		s_LightingDataUB->Bind();
	}

	void ShadowPass::RenderSpotLights() {
		static Ref<Framebuffer> s_BidingBuffer;
		if (!s_BidingBuffer) {
			FramebufferSpecification spec;
			spec.Attachments = {};
			spec.Width = _ShadowMapWidth;
			spec.Height = _ShadowMapHeight;
			spec.Samples = 1;
			s_BidingBuffer = Framebuffer::Create(spec);
		}
		auto view = PassConfig.Scene->m_Registry.view<TransformComponent, SpotLightComponent>();
		auto depthMaps = SpotLight::GetDepthMaps();
		if (!depthMaps)
			return;
		uint32_t oldViewportX, oldViewportY, oldViewportWidth, oldViewportHeight;
		RenderCommand::GetViewport(oldViewportX, oldViewportY, oldViewportWidth, oldViewportHeight);
		for (auto e : view) {
			auto [tc, dlc] = view.get<TransformComponent, SpotLightComponent>(e);
			auto& light = dlc.Light->GetLight();
			glm::mat4 viewMat = glm::lookAt(light.Position, glm::normalize(light.Direction) + light.Position, glm::vec3(0, 1, 0));
			glm::mat4 projMat = glm::perspective(glm::acos(light.CutOffAngle)*2.0f, (float)_ShadowMapWidth / (float)_ShadowMapHeight, 1.0f, 25.0f);

			depthMaps->ClearLayer(dlc.Light->GetIndex(), 1.0f);
			s_BidingBuffer->SetDepthAttachmentFromArray(depthMaps->GetRendererID(), dlc.Light->GetIndex());
			s_BidingBuffer->Bind();
			RenderCommand::SetViewport(0, 0, _ShadowMapWidth, _ShadowMapHeight);

			ShadowFlusher flusher = { projMat * viewMat,light.Position };


			Renderer3D::BeginRendering(&flusher);
			DrawCubes(PassConfig.Scene);
			JobSystem::GetMainJobSystem().Wait();
			Renderer3D::EndRendering();

			s_BidingBuffer->Unbind();
		}
		RenderCommand::SetViewport(oldViewportX, oldViewportY, oldViewportWidth, oldViewportHeight);
	}
	void ShadowPass::Render() {

		BindLights(PassConfig.Scene.get());
		static Ref<Framebuffer> s_BidingBuffer;
		if (!s_BidingBuffer) {
			FramebufferSpecification spec;
			spec.Attachments = {};
			spec.Width = _ShadowMapWidth;
			spec.Height = _ShadowMapHeight;
			spec.Samples = 1;
			s_BidingBuffer = Framebuffer::Create(spec);
		}
		RenderSpotLights();
		{
			auto view = PassConfig.Scene->m_Registry.view<TransformComponent, DirectionalLightComponent>();
			auto depthmaps = DirectionalLight::GetDepthMaps();
			if (!depthmaps)
				return;
			uint32_t oldViewportX, oldViewportY, oldViewportWidth, oldViewportHeight;
			RenderCommand::GetViewport(oldViewportX, oldViewportY, oldViewportWidth, oldViewportHeight);
			for (auto e : view) {
				auto [tc, dlc] = view.get<TransformComponent, DirectionalLightComponent>(e);
				auto& light = dlc.Light->GetLight();
				glm::mat4 viewMat = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0), glm::vec3(0, 1, 0));
				glm::mat4 projMat = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f,7.5f);



				depthmaps->ClearLayer(dlc.Light->GetIndex(),1.0f);
				s_BidingBuffer->SetDepthAttachmentFromArray(depthmaps->GetRendererID(), dlc.Light->GetIndex());
				s_BidingBuffer->Bind();

				RenderCommand::SetViewport(0, 0, _ShadowMapWidth, _ShadowMapHeight);

				ShadowFlusher flusher = { projMat * viewMat,{0,0,0} };


				Renderer3D::BeginRendering(&flusher);
				DrawCubes(PassConfig.Scene);
				JobSystem::GetMainJobSystem().Wait();
				Renderer3D::EndRendering();

				s_BidingBuffer->Unbind();
			}
			RenderCommand::SetViewport(oldViewportX, oldViewportY, oldViewportWidth, oldViewportHeight);
		}

		}



	void ShadowPass::DrawCubes(Ref<Scene> scene) {
		static auto cubeView = scene->m_Registry.view<TransformComponent, CubeRendererComponent>();
		cubeView = scene->m_Registry.view<TransformComponent, CubeRendererComponent>();
		for (auto e : cubeView) {
			JobSystem::GetMainJobSystem().Execute([e, this, &view = cubeView]() {
				CubeRendererComponent& crc = view.get<CubeRendererComponent>(e);
				Renderer3D::DrawCube(view.get<TransformComponent>(e).GetTransform(), crc.Material, (int)e);
				});
		}
	}

	void GeometryPass::DrawQuads(Ref<Scene> scene) {
		static auto quadView = scene->m_Registry.view<TransformComponent, SpriteRendererComponent>();
		quadView = scene->m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (auto e : quadView) {
			JobSystem::GetMainJobSystem().Execute([e, this, &view = quadView]() {
				Renderer2D::DrawQuad(view.get<TransformComponent>(e).GetTransform(), view.get<SpriteRendererComponent>(e).Color, 0, (int)(e));
				});
		}
	}
	void GeometryPass::DrawCircles(Ref<Scene> scene) {
		static auto circleView = scene->m_Registry.view<TransformComponent, CircleRendererComponent>();
		circleView = scene->m_Registry.view<TransformComponent, CircleRendererComponent>();
		for (auto e : circleView) {
			JobSystem::GetMainJobSystem().Execute([e, this, &view = circleView]() {
				auto& crc = view.get<CircleRendererComponent>(e);
				Renderer2D::DrawCircle(view.get<TransformComponent>(e).GetTransform(), crc.Color,
					0, crc.Thickness, crc.Fade, (int)e);
				});
		}
	}
	void GeometryPass::DrawLines(Ref<Scene> scene) {
		static auto lineView = scene->m_Registry.view<TransformComponent, LineRendererComponent>();
		lineView = scene->m_Registry.view<TransformComponent, LineRendererComponent>();
		for (auto e : lineView) {
			JobSystem::GetMainJobSystem().Execute([e, this, &view = lineView]() {
				auto& lrc = view.get<LineRendererComponent>(e);
				auto transform = view.get<TransformComponent>(e).GetTransform();
				for (uint32_t i = 0; i < lrc.FinalPoints.size() - 1; ++i) {
					Renderer2D::DrawLine(transform * glm::vec4{ lrc.FinalPoints.at(i).Position,1.0f },
						transform * glm::vec4{ lrc.FinalPoints.at(i + 1).Position,1.0f }, lrc.Color, 0, (int)e);
				}
				});
		}
	}
	void GeometryPass::DrawCubes(Ref<Scene> scene) {
		static auto cubeView = scene->m_Registry.view<TransformComponent, CubeRendererComponent>();
		cubeView = scene->m_Registry.view<TransformComponent, CubeRendererComponent>();
		for (auto e : cubeView) {
			JobSystem::GetMainJobSystem().Execute([e, this, &view = cubeView]() {
				CubeRendererComponent& crc = view.get<CubeRendererComponent>(e);
				Renderer3D::DrawCube(view.get<TransformComponent>(e).GetTransform(), crc.Material, (int)e);
				});
		}
	}

	void GeometryPass::Render()
	{
		BindLights(PassConfig.Scene.get());

		if (GeometryType & GeometryType_3D) {

			GeometryFlusher flusher = { PassConfig.ViewProjectionMatrix,PassConfig.CameraPosition };
			Renderer3D::BeginRendering(&flusher);
			DrawCubes(PassConfig.Scene);
			JobSystem::GetMainJobSystem().Wait();

			auto view = PassConfig.Scene->m_Registry.view<TransformComponent, SpotLightComponent>();
			for (auto e : view) {
				auto [tc, dlc] = view.get<TransformComponent, SpotLightComponent>(e);
				auto& light = dlc.Light->GetLight();
				Renderer3D::DrawCube(glm::translate(glm::mat4(1.0f), light.Position), nullptr);
			}

			Renderer3D::EndRendering();

			

		}


		/*if(GeometryType & GeometryType_3D)
			Renderer3D::BeginRendering(PassConfig.CameraPosition, PassConfig.ViewProjectionMatrix);

		DrawQuads(PassConfig.Scene);
		DrawCircles(PassConfig.Scene);
		DrawLines(PassConfig.Scene);




		JobSystem::GetMainJobSystem().Wait();
		DrawCubes(PassConfig.Scene);

		JobSystem::GetMainJobSystem().Wait();

		if (GeometryType & GeometryType_3D)
			Renderer3D::EndRendering();*/
	}
}
