#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"


#include "Kaidel/Renderer/UniformBuffer.h"
#include "Kaidel\Renderer\Renderer2D.h"
#include "Kaidel\Renderer\Renderer3D.h"
#include "Kaidel\Core\JobSystem.h"
#include "Kaidel/Core/Timer.h"

#include "Kaidel/Renderer/Material.h"
#include "Kaidel/Renderer/Light.h"
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
		m_QuadInsertIndex = 0;
		m_CircleInsertIndex = 0;
		m_LineInsertIndex = 0;
	}

	void Tesselate() {


	}
	void SceneRenderer::Bind(){
		static Ref<UniformBuffer> s_CountInfoUB = UniformBuffer::Create(2*sizeof(uint64_t),1);

		{
			auto view = static_cast<Scene*>(m_Context)->m_Registry.view<TransformComponent, LightComponent>();
			for (auto e : view) {
				auto [tc, lc] = view.get<TransformComponent, LightComponent>(e);
				lc.Light->SetPosition(tc.Translation);
			}
		}


		Light::SetLights();
		Material::SetMaterials();
		uint32_t countInfo[2] = { Light::GetLightCount(),Material::GetMaterialCount() };
		s_CountInfoUB->SetData(countInfo, sizeof(countInfo));
		s_CountInfoUB->Bind();
	}

	void SceneRenderer::DrawQuads() {
		auto context = static_cast<Scene*>(m_Context);
		static auto quadView = context->m_Registry.view<TransformComponent, SpriteRendererComponent>();
		quadView = context->m_Registry.view<TransformComponent, SpriteRendererComponent>();
		for (auto e : quadView) {
			auto start = m_QuadInsertIndex;
			m_JobSystem.Execute([e, this, start, context, &view = quadView]() {
				Renderer2D::DrawQuad(view.get<TransformComponent>(e).GetTransform(), view.get<SpriteRendererComponent>(e).Color, start, (int)(e));
				});
			m_QuadInsertIndex += 4;
		}
	}
	void SceneRenderer::DrawCircles() {
		auto context = static_cast<Scene*>(m_Context);
		static auto circleView = context->m_Registry.view<TransformComponent, CircleRendererComponent>();
		circleView = context->m_Registry.view<TransformComponent, CircleRendererComponent>();
		for (auto e : circleView) {
			auto start = m_CircleInsertIndex;
			m_JobSystem.Execute([e, this, start, context, &view = circleView]() {
				Entity entity{ e,context };
				auto& crc = view.get<CircleRendererComponent>(e);
				Renderer2D::DrawCircle(view.get<TransformComponent>(e).GetTransform(), crc.Color,
					m_CircleInsertIndex, crc.Thickness, crc.Fade, (int)e);
				});
			m_CircleInsertIndex += 4;
		}
	}
	void SceneRenderer::DrawLines() {
		auto context = static_cast<Scene*>(m_Context);
		static auto lineView = context->m_Registry.view<TransformComponent, LineRendererComponent>();
		lineView = context->m_Registry.view<TransformComponent, LineRendererComponent>();
		for (auto e : lineView) {
			auto start = m_LineInsertIndex;
			m_JobSystem.Execute([e, this, start, context, &view = lineView]() {
				auto& lrc = view.get<LineRendererComponent>(e);
				auto transform = view.get<TransformComponent>(e).GetTransform();
				uint32_t s = start;
				for (uint32_t i = 0; i < lrc.FinalPoints.size() - 1; ++i) {
					Renderer2D::DrawLine(transform * glm::vec4{ lrc.FinalPoints.at(i).Position,1.0f },
						transform * glm::vec4{ lrc.FinalPoints.at(i + 1).Position,1.0f }, lrc.Color, s, (int)e);
					s += 2;
				}
				});
			m_LineInsertIndex += lineView.get<LineRendererComponent>(e).FinalPoints.size() * 2;
		}
	}
	void SceneRenderer::DrawCubes() {
		auto context = static_cast<Scene*>(m_Context);
		static auto cubeView = context->m_Registry.view<TransformComponent, CubeRendererComponent>();
		cubeView = context->m_Registry.view<TransformComponent, CubeRendererComponent>();
		for (auto e : cubeView) {
			m_JobSystem.Execute([e, this, context, &view = cubeView]() {
				CubeRendererComponent& crc = view.get<CubeRendererComponent>(e);
				Renderer3D::DrawCube(view.get<TransformComponent>(e).GetTransform(), crc.Color, (int)e);
				});
		}
	}
	void SceneRenderer::Render()
	{

		DrawQuads();
		DrawCircles();
		DrawLines();

		


		m_JobSystem.Wait();
		Bind();
		DrawCubes();

		m_JobSystem.Wait();






	}

}
