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
		/*s_Data.MeshShader = Shader::Create({ {{"assets/shaders/GeometryPass/Mesh_VS.glsl",ShaderType::VertexShader} , {"assets/shaders/GeometryPass/Mesh_FS.glsl",ShaderType::FragmentShader}}, "Mesh" });
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

		}*/

	}
	void Renderer3D::Shutdown() {

	}

	void Renderer3D::Begin(const Renderer3DBeginData& beginData) {

	}

	static void RenderingPipeline(Mesh* mesh) {


	}

	Ref<Shader> Renderer3D::GetMeshShader() {
		return s_Data.MeshShader;
	}

	void Renderer3D::FlushMesh(Asset<Mesh> mesh) {
	
		

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

	}

	void Renderer3D::End() {

	}
	void Renderer3D::RenderingPipeLine() {
			
	}
}
