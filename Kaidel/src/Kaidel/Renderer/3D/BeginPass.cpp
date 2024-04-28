#include "KDpch.h"
#include "Kaidel/Renderer/MaterialTexture.h"
#include "Kaidel/Renderer/3D/BeginPass.h"
#include "Kaidel/Renderer/RendererAPI.h"

namespace Kaidel {

	GlobalRenderer3DData* GlobalRendererData = nullptr;
	GlobalRenderer3DData::GlobalRenderer3DData() {
		CameraUniformBuffer = UniformBuffer::Create(sizeof(CameraBufferData), RendererBindings::CameraBinding);
		LightCountUniformBuffer = UniformBuffer::Create(sizeof(LightCount), RendererBindings::LightCountBinding);


		{
			{
				ShaderSpecification spec;
				spec.Definitions = { ShaderDefinition("assets/shaders/ShadowPass_VS.glsl",ShaderType::VertexShader),ShaderDefinition("assets/shaders/ShadowPass_FS.glsl",ShaderType::FragmentShader) };
				//ShadowPassShader = Shader::Create({ {"assets/shaders/ShadowPass/Mesh_VS.glsl",ShaderType::VertexShader} ,{"assets/shaders/ShadowPass/Mesh_FS.glsl",ShaderType::FragmentShader} });
				ShadowPassShader = Shader::Create(spec);
			}
			
			{
				FramebufferSpecification fbSpec{};
				fbSpec.Width = _ShadowMapWidth;
				fbSpec.Height = _ShadowMapHeight;
				fbSpec.Attachments = { TextureFormat::Depth32F };
				ShadowDepthBuffer = Framebuffer::Create(fbSpec);
			}

			{
				FramebufferSpecification fbSpec{};
				fbSpec.Width = 1280;
				fbSpec.Height = 720;
				fbSpec.Samples = 1;
				fbSpec.Attachments = { TextureFormat::RGBA32F,TextureFormat::RGBA32F ,TextureFormat::R32I,TextureFormat::RGBA8 ,TextureFormat::Depth32F };
				GBuffers = Framebuffer::Create(fbSpec);
			}

			{
				ShaderSpecification spec;
				spec.Definitions = { ShaderDefinition("assets/shaders/GeometryPass_VS.glsl",ShaderType::VertexShader),ShaderDefinition("assets/shaders/GeometryPass_FS.glsl",ShaderType::FragmentShader) };
				GeometryPassShader = Shader::Create(spec);
			}

			{
				ShaderSpecification spec = { {ShaderDefinition("assets/shaders/FullScreenQuad_VS.glsl",ShaderType::VertexShader),ShaderDefinition("assets/shaders/LightingPass_FS.glsl",ShaderType::FragmentShader)} };
				LightingPassShader = Shader::Create(spec);
			}
		}

	}

	void BeginPass::Render(const StartData& startData) {
		
		//Light counts
		{
			GlobalRenderer3DData::LightCount lightCount{};
			lightCount.SpotLightCount = (int)SpotLight::GetLightCount();
			lightCount.PointLightCount = (int)PointLight::GetLightCount();
			GlobalRendererData->LightCountUniformBuffer->SetData(&lightCount, sizeof(GlobalRenderer3DData::LightCount));
			GlobalRendererData->LightCountUniformBuffer->Bind();
		}
		
		//Camera data
		{
			GlobalRendererData->CameraBuffer.Position = startData.CameraPosition;
			GlobalRendererData->CameraBuffer.ViewProj = startData.CameraVP;
			GlobalRendererData->CameraUniformBuffer->SetData(&GlobalRendererData->CameraBuffer, sizeof(GlobalRenderer3DData::CameraBufferData));
			GlobalRendererData->CameraUniformBuffer->Bind();
		}

		auto& outputSpec = startData.Outputbuffer->GetSpecification();
		if (const auto& gBufferSpec = GlobalRendererData->GBuffers->GetSpecification(); (gBufferSpec.Width != outputSpec.Width || gBufferSpec.Height != outputSpec.Height) && outputSpec.Width > 0 && outputSpec.Height > 0) {
			GlobalRendererData->GBuffers->Resize(outputSpec.Width, outputSpec.Height);
		}

		if (GlobalRendererData->GBuffers->GetSpecification().Samples != RendererAPI::GetSettings().MSAASampleCount) {
			GlobalRendererData->GBuffers->Resample(RendererAPI::GetSettings().MSAASampleCount);
		}

		GlobalRendererData->GBuffers->ClearDepthAttachment(1.0f);
		glm::vec4 defaults = glm::vec4{ 0.0f };
		GlobalRendererData->GBuffers->ClearAttachment(0, &defaults.x);
		GlobalRendererData->GBuffers->ClearAttachment(1, &defaults.x);
		{
			int default = 0;
			GlobalRendererData->GBuffers->ClearAttachment(2, &default);
		}
		GlobalRendererData->GBuffers->ClearAttachment(3, &defaults.x);

		SpotLight::SetLights();

		Material::SetMaterials();
		MaterialTexture::GetTextureArray()->Bind(RendererBindings::TextureBinding);
		RendererAPI::GetSettings().SetInShaders(5);

	}
}
