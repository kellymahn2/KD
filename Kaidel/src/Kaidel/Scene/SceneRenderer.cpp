#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"


#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/2D\Renderer2D.h"
#include "Kaidel/Renderer/3D\Renderer3D.h"
#include "Kaidel/Renderer/3D/ShadowPass.h"
#include "Kaidel/Renderer/3D/GeometryPass.h"
#include "Kaidel/Renderer/3D/BeginPass.h"
#include "Kaidel/Core/JobSystem.h"
#include "Kaidel/Core/Timer.h"
#include "Kaidel/Renderer/3D/Light.h"
#include "Kaidel/Renderer/3D/Material.h"
#include "Kaidel/Renderer/Primitives.h"
#include "Kaidel/Renderer/RenderCommand.h"

#include "Kaidel/Renderer/GraphicsAPI/CubeMap.h"

#include <glm/gtx/compatibility.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
namespace Kaidel {

	static std::mutex s_MeshRenderingMutex;
	static std::mutex s_MeshPushingMutex;

	static Ref<CubeMap> cb;

	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
		if (!cb) {
			cb = CubeMap::Create(1024,1024, TextureFormat::RGBA8);
			cb->SetData(CubeMapSide::Left, "assets/skybox/left.jpg",true);
			cb->SetData(CubeMapSide::Right, "assets/skybox/right.jpg", true);
			cb->SetData(CubeMapSide::Top, "assets/skybox/top.jpg", true);
			cb->SetData(CubeMapSide::Bottom, "assets/skybox/bottom.jpg", true);
			cb->SetData(CubeMapSide::Front, "assets/skybox/front.jpg", true);
			cb->SetData(CubeMapSide::Back, "assets/skybox/back.jpg", true);
		}
	}

	void SceneRenderer::Reset()
	{
	}

	void SceneRenderer::RenderMesh(Ref<Mesh> mesh) {

		if (mesh->GetDrawData().Size() == 0)
			return;
		mesh->GetPerInstanceBuffer()->SetData(mesh->GetDrawData().Get(),sizeof(MeshDrawData) * mesh->GetDrawData().Size());

		ShadowPass shadowPass;
		shadowPass.Render(mesh);

		GeometryPass geoPass;
		geoPass.Render(mesh);

		mesh->GetDrawData().Reset();
	}

	void SceneRenderer::Render(Ref<Framebuffer> _3DOutputFramebuffer,Ref<Framebuffer> _2DOutputFramebuffer,const glm::mat4& cameraViewProj,const glm::vec3& cameraPos)
	{

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


			BeginPass bp;
			StartData data{};
			data.CameraPosition = cameraPos;
			data.CameraVP = cameraViewProj;
			data.Outputbuffer = _3DOutputFramebuffer;
			bp.Render(data);


#if 0
			SCOPED_TIMER(MT_RENDERING);
			{
				auto meshView = reg.view<TransformComponent, MeshComponent>();
				for (auto e : meshView) {

					JobSystem::GetMainJobSystem().Execute([this, meshView, &reg, e]() {
						auto [tc, mc] = meshView.get(e);
						auto& mesh = mc.Mesh;
						auto& dd = mesh->GetDrawData();
			
						{
							std::scoped_lock<std::mutex> lock(s_MeshPushingMutex);
							if (!dd.CanReserveWithoutOverflow(1)) {
								std::scoped_lock<std::mutex> lock1(s_MeshRenderingMutex);
								RenderMesh(mesh.Data);
							}
						}
						
						MeshDrawData mdd{};
						mdd.Transform = tc.GetTransform();
						mdd.NormalTransform = glm::transpose(glm::inverse(mdd.Transform));
						uint32_t matID = 0;
						if (auto mc = reg.try_get<MaterialComponent>(e); mc && mc->Material) {
							matID = mc->Material->GetIndex();
						}
						mdd.MaterialID = matID;
			
						std::scoped_lock<std::mutex> lock(s_MeshPushingMutex);
						auto bvi = dd.Reserve(1);
						bvi[0] = std::move(mdd);
					});

				}
				JobSystem::GetMainJobSystem().Wait();
			}
			{
				auto meshView = reg.view<MeshComponent>();

				{
					auto cube = Primitives::CubePrimitive;
					if (cube->GetDrawData().Size() != 0) {
						RenderMesh(cube.Data);
					}
				}

				/*for (auto e : meshView) {
					JobSystem::GetMainJobSystem().Execute([this, meshView, e]() {
						auto& mc = meshView.get<MeshComponent>(e);
						auto& mesh = mc.Mesh;
						auto& dd = mesh->GetDrawData();
			
						if (dd.Size() != 0) {
							std::scoped_lock<std::mutex> lock(s_MeshRenderingMutex);
							RenderMesh(mesh.Data);
						}
					});
				}
				JobSystem::GetMainJobSystem().Wait();*/
			}
#else
				{
					SCOPED_TIMER(ST_RENDERING);
					{
						auto meshView = reg.view<TransformComponent, MeshComponent>();
						for (auto e : meshView) {

							auto [tc, mc] = meshView.get(e);
							auto& mesh = mc.Mesh;
							auto& dd = mesh->GetDrawData();

							if (!dd.CanReserveWithoutOverflow(1)) {
								RenderMesh(mesh.Data);
							}
							MeshDrawData mdd{};
							mdd.Transform = tc.GetTransform();
							mdd.NormalTransform = glm::transpose(glm::inverse(mdd.Transform));
							uint32_t matID = 0;
							if (auto mc = reg.try_get<MaterialComponent>(e); mc && mc->Material) {
								matID = mc->Material->GetIndex();
							}
							mdd.MaterialID = matID;

							std::scoped_lock<std::mutex> lock(s_MeshPushingMutex);
							auto bvi = dd.Reserve(1);
							bvi[0] = std::move(mdd);

						}
						JobSystem::GetMainJobSystem().Wait();
					}
					{
						auto meshView = reg.view<MeshComponent>();
						for (auto e : meshView) {
							auto& mc = meshView.get<MeshComponent>(e);
							auto& mesh = mc.Mesh;
							auto& dd = mesh->GetDrawData();

							if (dd.Size() != 0) {
								RenderMesh(mesh.Data);
							}
						}
						JobSystem::GetMainJobSystem().Wait();
					}
				}

				_3DOutputFramebuffer->Bind();
				GlobalRendererData->GBuffers->BindColorAttachmentToSlot(0, 0);
				GlobalRendererData->GBuffers->BindColorAttachmentToSlot(1, 1);
				GlobalRendererData->GBuffers->BindColorAttachmentToSlot(2, 2);
				GlobalRendererData->GBuffers->BindColorAttachmentToSlot(3, 3);

				RenderCommand::RenderFullScreenQuad(GlobalRendererData->LightingPassShader,_3DOutputFramebuffer->GetSpecification().Width,_3DOutputFramebuffer->GetSpecification().Height);




				_3DOutputFramebuffer->Unbind();

#endif

				/*	Renderer3DBeginData data;

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
					}*/

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
