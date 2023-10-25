#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"
#include "Kaidel\Renderer\Renderer2D.h"
#include "Kaidel\Core\JobSystem.h"
#include "Kaidel/Core/Timer.h"

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
	


	void SceneRenderer::Render()
	{
		auto context = static_cast<Scene*>(m_Context);
		auto quadView = context->m_Registry.view<TransformComponent, SpriteRendererComponent>();
		{
			for (auto e : quadView) {
				auto start = m_QuadInsertIndex;
				m_JobSystem.Execute([e, this, start, context, &view = quadView]() {
					Renderer2D::DrawQuad(view.get<TransformComponent>(e).GetTransform(), view.get<SpriteRendererComponent>(e).Color, start, (int)(e));
					});
				m_QuadInsertIndex += 4;
			}
		}

		auto circleView = context->m_Registry.view<TransformComponent, CircleRendererComponent>();
		for (auto e : circleView) {
			auto start = m_CircleInsertIndex;
			m_JobSystem.Execute([e, this, start,context,&view = circleView]() {
				Entity entity{ e,context};
				auto& crc = view.get<CircleRendererComponent>(e);
				Renderer2D::DrawCircle(view.get<TransformComponent>(e).GetTransform(), crc.Color,
					m_CircleInsertIndex,crc.Thickness, crc.Fade, (int)e);
				});
			m_CircleInsertIndex += 4;
		}


		auto lineView= context->m_Registry.view<TransformComponent, LineRendererComponent>();
		for (auto e : lineView) {
			auto start = m_LineInsertIndex;
			m_JobSystem.Execute([e, this, start, context, &view = lineView]() {
				auto& lrc = view.get<LineRendererComponent>(e);
				auto transform= view.get<TransformComponent>(e).GetTransform();
				uint32_t s = start;
				for (uint32_t i = 0; i < lrc.FinalPoints.size() - 1; ++i) {
					Renderer2D::DrawLine(transform*glm::vec4{lrc.FinalPoints.at(i).Position,1.0f}, 
						transform* glm::vec4{ lrc.FinalPoints.at(i + 1).Position,1.0f }, lrc.Color, s, (int)e);
					s += 2;
				}
			});
			m_LineInsertIndex += lineView.get<LineRendererComponent>(e).FinalPoints.size()*2;
		}
		m_JobSystem.Wait();
	}

}
