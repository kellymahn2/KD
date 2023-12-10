#include "KDpch.h"
#include "Kaidel/Renderer/Renderer.h"
#include "Kaidel/Renderer/Renderer2D.h"
#include "Kaidel/Renderer/Renderer3D.h"

namespace Kaidel {

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{
		KD_PROFILE_FUNCTION();

		RenderCommand::Init();
		Renderer3D::Init();
		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer3D::Shutdown();
		Renderer2D::Shutdown();
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	void Renderer::BeginScene(OrthographicCamera& camera)
	{
		s_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
	}

	void Renderer::EndScene()
	{
	}

	void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		shader->SetMat4("u_ViewProjection", s_SceneData->ViewProjectionMatrix);
		shader->SetMat4("u_Transform", transform);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);
	}

}
