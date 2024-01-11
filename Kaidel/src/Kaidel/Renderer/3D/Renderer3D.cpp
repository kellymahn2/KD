#include "KDpch.h"
#include "Renderer3D.h"

#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Buffer.h"
#include "Kaidel/Renderer/UniformBuffer.h"
#include "Kaidel/Renderer/Light.h"
#include "Kaidel/Core/Timer.h"
#include<unordered_set>
namespace Kaidel {


	struct Renderer3DData {

		static inline constexpr uint32_t MaxCubes = 10000;
		static inline constexpr uint32_t MaxCubeVertices = 10000 * 24;
		static inline constexpr uint32_t MaxCubeIndices = MaxCubes * 36;


		Ref<Shader> MeshShader;
		Ref<ComputeShader> SceneCompositeShader;
		struct CameraBufferData {
			glm::mat4 ViewProj;
			glm::vec3 Position;
		};
		Ref<Material> DefaultMaterial;
		CameraBufferData CameraBuffer;
		
		struct LightCount {
			int PointLightCount;
			int SpotLightCount;
		};
		
		Ref<UniformBuffer> CameraUniformBuffer;
		Ref<UniformBuffer> LightCountUniformBuffer;
		Ref<Framebuffer> OutputBuffer;
		Ref<Framebuffer> G_Buffers;

		/*BoundedVector<Ref<Model>> DrawnModels = { 0,32,[&](Ref<Model>* models,auto count) {
				for (auto i = 0; i < count; ++i) {
					(models[i])->Flush();
				} 
			} 
		};*/
		std::unordered_set<Ref<Model>> DrawnModels;
	};

	Renderer3DData s_Data;


	void Renderer3D::Init() {
		using fs = std::filesystem::path;
		s_Data.MeshShader = Shader::Create(fs::path("assets/shaders/GeometryPass/Mesh_VS.glsl"), fs::path("assets/shaders/GeometryPass/Mesh_FS.glsl"), "Mesh");
		s_Data.CameraUniformBuffer = UniformBuffer::Create(80, 0);
		s_Data.LightCountUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::LightCount), 1);
		FramebufferSpecification fbSpec{};
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		fbSpec.Samples = 1;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA32F,FramebufferTextureFormat::RGBA32F ,FramebufferTextureFormat::RED_INTEGER,FramebufferTextureFormat::RGBA8 ,FramebufferTextureFormat::Depth};
		s_Data.G_Buffers = Framebuffer::Create(fbSpec);
		s_Data.SceneCompositeShader = ComputeShader::Create("assets/shaders/LightingPass/Mesh_CS.glsl");
		s_Data.DefaultMaterial = CreateRef<Material>();
		MaterialTextureHandler::Init();
	}
	void Renderer3D::Shutdown() {

	}

	void Renderer3D::Begin(const Renderer3DBeginData& beginData) {
		s_Data.CameraBuffer.ViewProj = beginData.CameraVP;
		s_Data.CameraBuffer.Position = beginData.CameraPosition;
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraBufferData));

		Renderer3DData::LightCount lightCount{};
		lightCount.SpotLightCount = SpotLight::GetLightCount();
		lightCount.PointLightCount = PointLight::GetLightCount();
		s_Data.LightCountUniformBuffer->SetData(&lightCount, sizeof(Renderer3DData::LightCount));
		s_Data.OutputBuffer = beginData.OutputBuffer;
		const auto& outputSpec = beginData.OutputBuffer->GetSpecification();
		if (const auto& gBufferSpec = s_Data.G_Buffers->GetSpecification(); (gBufferSpec.Width != outputSpec.Width || gBufferSpec.Height != outputSpec.Height) && outputSpec.Width > 0 && outputSpec.Height > 0) {
			s_Data.G_Buffers->Resize(outputSpec.Width, outputSpec.Height);
		}

		s_Data.G_Buffers->Bind();
		s_Data.G_Buffers->ClearDepthAttachment(1.0f);
		float defaults[4] = { 0.0f };
		s_Data.G_Buffers->ClearAttachment(0, defaults);
		s_Data.G_Buffers->ClearAttachment(1, defaults);
		s_Data.G_Buffers->ClearAttachment(2, defaults);
		s_Data.G_Buffers->ClearAttachment(3, defaults);


		Material::SetMaterials();
		MaterialTextureHandler::GetTexturesMap()->Bind(0);
		SpotLight::SetLights();
		PointLight::SetLights();
		DirectionalLight::SetLights();
		s_Data.CameraUniformBuffer->Bind();

		s_Data.DrawnModels.clear();
	}
	void Renderer3D::DrawCube(const glm::mat4& transform, Ref<Material> material) {
		
	}
	void Renderer3D::DrawModel(const glm::mat4& transform, Ref<Model> model,Ref<Material> material) {

		if (!material)
		{
			DrawModel(transform, model, s_Data.DefaultMaterial);
			return;
		}
		{
			s_Data.MeshShader->Bind();
			s_Data.G_Buffers->Bind();
			{
				model->Draw(transform, material);
			}
			s_Data.G_Buffers->Unbind();
			s_Data.DrawnModels.insert(model);
		}
	}
	void Renderer3D::End() {
		{
			s_Data.MeshShader->Bind();
			s_Data.G_Buffers->Bind();
			for (auto& model : s_Data.DrawnModels) {
				model->Flush();
			}
			s_Data.G_Buffers->Unbind();
		}

		{

			s_Data.SceneCompositeShader->Bind();

			s_Data.G_Buffers->BindColorAttachmentToImageSlot(0, 0, ImageBindingMode_Read);
			s_Data.G_Buffers->BindColorAttachmentToImageSlot(1, 1, ImageBindingMode_Read);
			s_Data.G_Buffers->BindColorAttachmentToImageSlot(2, 2, ImageBindingMode_Read);
			s_Data.G_Buffers->BindColorAttachmentToImageSlot(3, 3, ImageBindingMode_Read);

			s_Data.OutputBuffer->BindColorAttachmentToImageSlot(0, 4, ImageBindingMode_Write);

			const auto& outputSpec = s_Data.OutputBuffer->GetSpecification();
			s_Data.SceneCompositeShader->Execute(outputSpec.Width / 32 + 1, outputSpec.Height / 32 + 1, 1);
			s_Data.SceneCompositeShader->Wait();
		}
	}
	void Renderer3D::RenderingPipeLine() {
			
	}
}
