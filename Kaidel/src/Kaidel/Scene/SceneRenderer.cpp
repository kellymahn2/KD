#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"


#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/GraphicsAPI/CubeMap.h"

#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
namespace Kaidel {

	static std::mutex s_MeshRenderingMutex;
	static std::mutex s_MeshPushingMutex;


	static void FlushShadowPass(Ref<VertexArray> vertexArray) {

	}

	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
	}

	void SceneRenderer::Reset()
	{
	}

	void SceneRenderer::Render(Ref<Framebuffer> _3DOutputFramebuffer,Ref<Framebuffer> _2DOutputFramebuffer,const glm::mat4& cameraViewProj,const glm::vec3& cameraPos)
	{

		Scene* activeScene = static_cast<Scene*>(m_Context);

		Renderer2D::Begin(cameraViewProj,_2DOutputFramebuffer);
		//Sprites
		{
			auto view = activeScene->m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto e : view) {
				auto [tc, src] = view.get<TransformComponent, SpriteRendererComponent>(e);
				Renderer2D::DrawSprite(tc.GetTransform(), glm::vec4{1.0f,1.0f,0.0f,1.0f});
			}
		}

		Renderer2D::End();
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
