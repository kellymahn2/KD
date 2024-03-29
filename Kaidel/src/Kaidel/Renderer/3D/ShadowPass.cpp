#include "KDpch.h"
#include "Kaidel/Renderer/3D/Light.h"
#include "ShadowPass.h"
#include "Kaidel/Scene/Components.h"
#include "BeginPass.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Core/JobSystem.h"
namespace Kaidel {

	static std::mutex s_RenderingMutex;


	void ShadowPass::FlushMesh(Ref<Mesh> mesh) {
		auto vbo = mesh->GetVertexBuffer();
		auto ibo = mesh->GetIndexBuffer();
		auto vao = mesh->GetShadowVAO();


		
		uint32_t spotLightCount = SpotLight::GetLightCount();


		for (uint32_t i = 0; i < spotLightCount; ++i) {

			GlobalRendererData->ShadowPassShader->Bind();
			GlobalRendererData->ShadowDepthBuffer->Bind();
		
			GlobalRendererData->ShadowPassShader->SetInt("u_LightIndex",i);
			
			GlobalRendererData->ShadowDepthBuffer->SetDepthAttachment(SpotLight::GetDepthMaps()->GetHandle(i));

			GlobalRendererData->ShadowDepthBuffer->Bind();

			RenderCommand::SetCullMode(CullMode::Front);
			RenderCommand::DrawIndexedInstanced(vao, ibo ? ibo->GetCount() : 0, mesh->GetDrawData().Size());
			RenderCommand::SetCullMode(CullMode::Back);

			GlobalRendererData->ShadowDepthBuffer->Unbind();
			GlobalRendererData->ShadowPassShader->Unbind();

		}
	}

	void ShadowPass::Render(Ref<Mesh> mesh) {
		FlushMesh(mesh);
	}
}
