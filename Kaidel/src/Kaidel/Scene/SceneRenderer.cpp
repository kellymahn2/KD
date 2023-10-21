#include "KDpch.h"
#include "SceneRenderer.h"
#include "Components.h"
#include "Scene.h"
#include "Entity.h"
#include "Kaidel\Renderer\Renderer2D.h"
#include "Kaidel\Core\JobSystem.h"
namespace Kaidel {

	static JobSystem s_JobSystem = JobSystem(4);

	SceneRenderer::SceneRenderer(void* scene)
		:m_Context(scene)
	{
	}

	void SceneRenderer::Reset()
	{
		m_InsertIndex = 0;
	}


	void SceneRenderer::Render()
	{
		auto context = static_cast<Scene*>(m_Context);
		int i = 0;
		{
			auto view = context->m_Registry.view<TransformComponent, SpriteRendererComponent>();
			for (auto e : view) {
				auto start = m_InsertIndex;
				s_JobSystem.Execute([e,this,start]() {
					Entity entity{ e,static_cast<Scene*>(m_Context) };
					Renderer2D::DrawQuad(entity.GetComponent<TransformComponent>().GetTransform(), entity.GetComponent<SpriteRendererComponent>().Color, start, (int)(entity.operator entt::entity()));
					});
				m_InsertIndex += 4;
			}
		}
		s_JobSystem.Wait();
	}



}
