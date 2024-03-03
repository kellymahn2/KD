#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"


#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Renderer/3D\Renderer3D.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/3D/Light.h"
#include "Kaidel/Renderer/3D/Material.h"

#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
namespace Kaidel {

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

		//3D
		{
			{
				auto view = activeScene->m_Registry.view<TransformComponent, SpotLightComponent>();
				for (auto e : view) {
					auto [tc, slc] = view.get<TransformComponent, SpotLightComponent>(e);
					auto& light = slc.Light->GetLight();

					light.LightViewProjection = glm::infinitePerspective(2.0f * glm::acos(light.CutOffAngle), 1.0f, .5f) * glm::lookAt(glm::vec3(light.Position), glm::vec3(light.Position + glm::normalize(light.Direction)), glm::vec3(0.0f, 1.0f, 0.0f));
					light.Position = glm::vec4(tc.Translation, 1.0f);
				}
			}
			Renderer3DBeginData data;

			data.OutputBuffer = _3DOutputFramebuffer;
			data.CameraPosition = cameraPos;
			data.CameraVP = cameraViewProj;

			{
				Renderer3D::Begin(data);
				{
					auto view = activeScene->m_Registry.view<TransformComponent, MeshComponent>();
					for (auto e : view) {
						auto [tc, mc] = view.get<TransformComponent, MeshComponent>(e);
						if (!mc.Mesh)
							continue;
						auto meshMat = mc.Mesh.Data->GetMaterial();
						if (auto p = activeScene->m_Registry.try_get<MaterialComponent>(e); p != nullptr)
							meshMat = p->Material;
						Renderer3D::DrawMesh(tc.GetTransform(), mc.Mesh, meshMat);
					}
				}

				Renderer3D::End();
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

	}

}
