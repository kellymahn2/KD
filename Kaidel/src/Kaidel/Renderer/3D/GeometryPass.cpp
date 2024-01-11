#include "KDpch.h"
#include "GeometryPass.h"
#include "Kaidel/Renderer/Shader.h"
#include "Kaidel/Renderer/RenderCommand.h"
namespace Kaidel {

	static Ref<Framebuffer> G_Buffers;
	static Ref<Shader> GeometryPassShader;
	static Ref<Shader> MeshShader;
	static Ref<Shader> SphereShader;

	GeometryPass::GeometryPass(const GeometryPassInput& input) 
		:m_Input(input)
	{
		if (!G_Buffers)
		{
			FramebufferSpecification fbSpec{};
			fbSpec.Width = 1280;
			fbSpec.Height = 720;
			fbSpec.Samples = 1;
			fbSpec.Attachments = { FramebufferTextureFormat::RGBA32F,FramebufferTextureFormat::RGBA32F ,FramebufferTextureFormat::RGBA8 };
		}

		if (const FramebufferSpecification& spec = G_Buffers->GetSpecification(); (spec.Width != input.Width || spec.Height != input.Height) && input.Width > 0 && input.Height > 0) {
			G_Buffers->Resize(input.Width, input.Height);
		}
		if (!MeshShader) {
			MeshShader = Shader::Create(std::filesystem::path("assets/shaders/GeometryPass/Mesh_VS.glsl"),std::filesystem::path("assets/shaders/GeometryPass/Mesh_FS.glsl"), "MeshShader");
		}
	}

	GeometryPassOutput GeometryPass::Apply() {
		G_Buffers->Bind();
		RenderCommand::DrawIndexed(m_Input.VAO, m_Input.IndexCount);
		G_Buffers->Unbind();
		GeometryPassOutput output;
		output.G_Buffers = G_Buffers;
		return output;
	}

}
