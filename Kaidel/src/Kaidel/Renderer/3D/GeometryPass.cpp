#include "KDpch.h"
#include "GeometryPass.h"
#include "BeginPass.h"
#include "Kaidel/Renderer/RenderCommand.h"
namespace Kaidel {
	void GeometryPass::Render(Ref<Mesh> mesh) {
		auto vbo = mesh->GetVertexBuffer();
		auto ibo = mesh->GetIndexBuffer();
		auto pib = mesh->GetPerInstanceBuffer();
		auto vao = mesh->GetVertexArray();

		GlobalRendererData->GeometryPassShader->Bind();

		GlobalRendererData->GBuffers->Bind();

		RenderCommand::SetCullMode(CullMode::Back);
		RenderCommand::DrawIndexedInstanced(vao, ibo ? ibo->GetCount() : 0, mesh->GetDrawData().Size());
		RenderCommand::SetCullMode(CullMode::None);

		GlobalRendererData->GBuffers->Unbind();
	
		GlobalRendererData->GeometryPassShader->Unbind();
	}
}

