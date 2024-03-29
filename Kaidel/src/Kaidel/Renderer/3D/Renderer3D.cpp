#include "KDpch.h"
#include "Renderer3D.h"

#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/3D/Light.h"
#include "Kaidel/Renderer/MaterialTexture.h"
#include "Kaidel/Core/Timer.h"
#include<unordered_set>
namespace Kaidel {


	struct Renderer3DData {

		static inline constexpr uint32_t MaxCubes = 10000;
		static inline constexpr uint32_t MaxCubeVertices = 10000 * 24;
		static inline constexpr uint32_t MaxCubeIndices = MaxCubes * 36;


		Ref<Shader> MeshShader;

		Ref<VertexArray> MeshVAO;
		Ref<VertexBuffer> MeshVBO;
		Ref<VertexBuffer> PerInstanceVBO;
		Ref<IndexBuffer> MeshIBO;

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
		Math::Frustum Frustum;

		std::mutex PushMutex;
		/*BoundedVector<Ref<Model>> DrawnModels = { 0,32,[&](Ref<Model>* models,auto count) {
				for (auto i = 0; i < count; ++i) {
					(models[i])->Flush();
				}
			}
		};*/

		std::vector<Asset<Mesh>> DrawnModels;


		Ref<Shader> ShadowMapShader;

		Ref<Framebuffer> ShadowMapFrameBuffer;
		Renderer3DStats Stats;

		RendererSettings* CurrentRendererSettings;

	};



	Renderer3DData s_Data;

	void Renderer3D::ResetStats() {
		s_Data.Stats = Renderer3DStats{};
	}
	Renderer3DStats& Renderer3D::GetStats() {
		return s_Data.Stats;
	}


	void Renderer3D::SetupPrimitives() {



	}

	void Renderer3D::Init() {
		s_Data.MeshShader = Shader::Create({ {{"assets/shaders/GeometryPass/Mesh_VS.glsl",ShaderType::VertexShader} , {"assets/shaders/GeometryPass/Mesh_FS.glsl",ShaderType::FragmentShader}}, "Mesh" });
		s_Data.CameraUniformBuffer = UniformBuffer::Create(80, 0);
		s_Data.LightCountUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::LightCount), 1);
		{
			FramebufferSpecification fbSpec{};
			fbSpec.Width = 1280;
			fbSpec.Height = 720;
			fbSpec.Samples = 1;
			fbSpec.Attachments = { TextureFormat::RGBA32F,TextureFormat::RGBA32F ,TextureFormat::R32I,TextureFormat::RGBA8 ,TextureFormat::Depth32F };
			s_Data.G_Buffers = Framebuffer::Create(fbSpec);
		}
		s_Data.SceneCompositeShader = ComputeShader::Create("assets/shaders/LightingPass/Mesh_CS.glsl");
		s_Data.DefaultMaterial = CreateRef<Material>();
		s_Data.ShadowMapShader = Shader::Create({ {"assets/shaders/ShadowPass/Mesh_VS.glsl",ShaderType::VertexShader} ,{"assets/shaders/ShadowPass/Mesh_FS.glsl",ShaderType::FragmentShader} });
		{
			FramebufferSpecification fbSpec{};
			fbSpec.Width = _ShadowMapWidth;
			fbSpec.Height = _ShadowMapHeight;
			fbSpec.Attachments = { TextureFormat::Depth32F };
			s_Data.ShadowMapFrameBuffer = Framebuffer::Create(fbSpec);
		}


		{

			s_Data.MeshVBO = VertexBuffer::Create(0);
			s_Data.MeshVBO->SetLayout({
			{ShaderDataType::Float3,"a_Position"},
			{ShaderDataType::Float3,"a_Normal"},
			{ShaderDataType::Float2,"a_TexCoords"},
				});
			s_Data.PerInstanceVBO = VertexBuffer::Create(1024 * sizeof(MeshDrawData));
			s_Data.PerInstanceVBO->SetLayout({
				{ShaderDataType::Mat4,"a_Transform",1},
				{ShaderDataType::Mat3,"a_NormalTransform",1},
				{ShaderDataType::Int,"a_MaterialID",1}
				});

			s_Data.MeshIBO = IndexBuffer::Create(nullptr, 0);

			VertexArraySpecification spec;
			spec.VertexBuffers.push_back(s_Data.MeshVBO);
			spec.VertexBuffers.push_back(s_Data.PerInstanceVBO);
			spec.UsedShader = s_Data.MeshShader;
			spec.IndexBuffer = s_Data.MeshIBO;
			s_Data.MeshVAO = VertexArray::Create(spec);

		}

	}
	void Renderer3D::Shutdown() {

	}

	void Renderer3D::Begin(const Renderer3DBeginData& beginData) {
		s_Data.CameraBuffer.ViewProj = beginData.CameraVP;
		s_Data.CameraBuffer.Position = beginData.CameraPosition;
		s_Data.Frustum.ExtractPlanesFromProjectionView(s_Data.CameraBuffer.ViewProj);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraBufferData));

		if (s_Data.G_Buffers->GetSpecification().Samples != RendererAPI::GetSettings().MSAASampleCount) {
			s_Data.G_Buffers->Resample(RendererAPI::GetSettings().MSAASampleCount);
		}


		Renderer3DData::LightCount lightCount{};
		lightCount.SpotLightCount = SpotLight::GetLightCount();
		lightCount.PointLightCount = PointLight::GetLightCount();
		s_Data.LightCountUniformBuffer->SetData(&lightCount, sizeof(Renderer3DData::LightCount));
		s_Data.OutputBuffer = beginData.OutputBuffer;
		const auto& outputSpec = beginData.OutputBuffer->GetSpecification();
		if (const auto& gBufferSpec = s_Data.G_Buffers->GetSpecification(); (gBufferSpec.Width != outputSpec.Width || gBufferSpec.Height != outputSpec.Height) && outputSpec.Width > 0 && outputSpec.Height > 0) {
			s_Data.G_Buffers->Resize(outputSpec.Width, outputSpec.Height);
		}

		s_Data.G_Buffers->ClearDepthAttachment(1.0f);
		glm::vec4 defaults = glm::vec4{ 0.0f };
		s_Data.G_Buffers->ClearAttachment(0, &defaults.x);
		s_Data.G_Buffers->ClearAttachment(1, &defaults.x);
		{
			int default = 0;
			s_Data.G_Buffers->ClearAttachment(2, &default);
		}
		s_Data.G_Buffers->ClearAttachment(3, &defaults.x);

		/*defaults = glm::vec4{-1.0f};
		s_Data.G_Buffers->ClearAttachment(4, &defaults.x);*/


		Material::SetMaterials();
		MaterialTexture::GetTextureArray()->Bind(0);
		SpotLight::SetLights();
		PointLight::SetLights();
		DirectionalLight::SetLights();
		RendererAPI::GetSettings().SetInShaders(5);

		s_Data.CameraUniformBuffer->Bind();

		s_Data.DrawnModels.clear();

	}

	static void RenderingPipeline(Mesh* mesh) {


	}

	Ref<Shader> Renderer3D::GetMeshShader() {
		return s_Data.MeshShader;
	}


	void Renderer3D::FlushMesh(Asset<Mesh> mesh) {
	
		
		if (mesh->m_InstanceCount == 0)
			return;
		//Geometry Pass
		{
			SCOPED_TIMER(Geometry Pass)
			//Timer timer("Geometry Pass");
			s_Data.G_Buffers->Bind();
			s_Data.MeshShader->Bind();
			s_Data.PerInstanceVBO->SetData(mesh->m_DrawData.Get(), mesh->m_DrawData.Size() * sizeof(MeshDrawData));
			s_Data.MeshVBO->SetData(mesh->m_Vertices.data(), mesh->m_Vertices.size() * sizeof(MeshVertex));
			s_Data.MeshVAO->SetIndexBuffer(mesh->m_IBO);
			RenderCommand::SetCullMode(CullMode::Back);
			{
				RenderCommand::DrawIndexedInstanced(s_Data.MeshVAO, mesh->m_IndexCount, mesh->m_InstanceCount);
			}
			RenderCommand::SetCullMode(CullMode::None);
			s_Data.Stats.GeometryPassDrawCount++;
			s_Data.G_Buffers->Unbind();

		}


		//Shadow Pass
		{
			SCOPED_TIMER(Shadow Pass)
			s_Data.ShadowMapShader->Bind();
			auto spotlightDepthMaps = SpotLight::GetDepthMaps();
			s_Data.ShadowMapFrameBuffer->Bind();
			RenderCommand::SetCullMode(CullMode::Front);
			for (auto& light : SpotLight::GetLights()) {
				s_Data.ShadowMapShader->SetInt("u_LightIndex", light->GetIndex());
				s_Data.ShadowMapFrameBuffer->SetDepthAttachment(spotlightDepthMaps->GetHandle(light->GetIndex()));
				RenderCommand::DrawIndexedInstanced(s_Data.MeshVAO, mesh->m_IndexCount, mesh->m_InstanceCount);
			}
			RenderCommand::SetCullMode(CullMode::None);
			s_Data.ShadowMapFrameBuffer->Unbind();
		}

		mesh->m_InstanceCount = 0;
		mesh->m_DrawData.Reset();

	}

	template<typename T>
	bool Find(const std::vector<T>& v, const T& value) {
		for (auto& val : v) {
			if (val == value)
				return true;
		}
		return false;
	}



	void Renderer3D::DrawMesh(const glm::mat4& transform, Asset<Mesh> mesh, Ref<Material> material) {

		if (!material)
		{
			DrawMesh(transform, mesh, s_Data.DefaultMaterial);
			return;
		}
		{
			//if (s_Data.Frustum.IsAABBInFrustum(transform, mesh->GetBoundingBox())) {


				s_Data.Stats.PushCount++;
				auto& c = s_Data.DrawnModels;
				if (!Find(s_Data.DrawnModels,mesh)) {
					s_Data.DrawnModels.push_back(mesh);
				}

				//Model needs flush
				if (!mesh->Draw(transform, material)) {
					FlushMesh(mesh);
					mesh->Draw(transform, material);
				}



			//}
		}
	}

	void Renderer3D::End() {

		{
			auto& c = s_Data.DrawnModels;
			for (auto& mesh : s_Data.DrawnModels) {
				FlushMesh(mesh);
			}

			s_Data.DrawnModels.clear();
		}

		{
			s_Data.SceneCompositeShader->Bind();

			s_Data.G_Buffers->BindColorAttachmentToImageSlot(0, 0, ImageBindingMode_Read);
			s_Data.G_Buffers->BindColorAttachmentToImageSlot(1, 1, ImageBindingMode_Read);
			s_Data.G_Buffers->BindColorAttachmentToImageSlot(2, 2, ImageBindingMode_Read);
			s_Data.G_Buffers->BindColorAttachmentToImageSlot(3, 3, ImageBindingMode_Read);

			s_Data.OutputBuffer->BindColorAttachmentToImageSlot(0, 4, ImageBindingMode_Write);
			
			SpotLight::GetDepthMaps()->Bind(_SpotLightBindingSlot);

			const auto& outputSpec = s_Data.OutputBuffer->GetSpecification();
			s_Data.SceneCompositeShader->Execute(glm::ceil((float)outputSpec.Width / 32.0f), glm::ceil((float)outputSpec.Height / 32.0f), 1);
			s_Data.SceneCompositeShader->Wait();

			s_Data.OutputBuffer->CopyDepthAttachment(s_Data.G_Buffers);
		}
	}
	void Renderer3D::RenderingPipeLine() {
			
	}
}
