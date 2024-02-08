#include "KDpch.h"

#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Renderer2D.h"
#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Buffer.h"
#include "Kaidel/Renderer/Shader.h"
#include "Kaidel/Renderer/UniformBuffer.h"

namespace Kaidel {


	struct SpriteRendererData {
		static inline constexpr uint32_t MaxSprites = 2048;
		static inline constexpr uint32_t MaxSpriteVertices = MaxSprites * 4;
		static inline constexpr uint32_t MaxSpriteIndices = MaxSprites * 6;

		SpriteVertex DefaultSpriteVertices[4];

		Ref<VertexArray> SpriteVAO;
		Ref<VertexBuffer> SpriteVBO;
		Ref<Shader> SpriteShader;

		uint32_t RenderedSpriteCount = 0;
		uint32_t SpritesWaitingForRender = 0;

		std::mutex SpriteRenderingMutex;
		BoundedVector<SpriteVertex> Vertices = { 0,MaxSpriteVertices,[](auto data,uint64_t size) {
			Renderer2D::FlushSprites();
		} };
		uint32_t* SetupSpriteIndices() {
			uint32_t* quadIndices = new uint32_t[MaxSpriteIndices];

			uint32_t offset = 0;
			for (uint32_t i = 0; i < MaxSpriteIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}
			return quadIndices;
		}
		void Init() {
			using path = FileSystem::path;
			SpriteShader = Shader::CreateFromPath(path("assets/shaders/GeometryPass/Geometry_Sprite_VS_2D.glsl"), path("assets/shaders/GeometryPass/Geometry_Sprite_FS_2D.glsl"));
			//Vertex Array Object
			{
				SpriteVAO = VertexArray::Create();
			}


			//Index Buffer Object
			{
				uint32_t* indices = SetupSpriteIndices();
				Ref<IndexBuffer> ibo = IndexBuffer::Create(indices, MaxSpriteIndices);
				delete[] indices;
				SpriteVAO->SetIndexBuffer(ibo);
			}

			//Vertex Buffer Object
			{
				SpriteVBO = VertexBuffer::Create(0);
				SpriteVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"},
					{ShaderDataType::Float2,"a_TexCoords"},
					{ShaderDataType::Int,"a_MaterialID"}
					});
				SpriteVAO->AddVertexBuffer(SpriteVBO);
			}


			//Default Sprite Vertices
			{
				DefaultSpriteVertices[0] = { {-.5f,-.5f,.0f} ,glm::vec2{0,0},0 };
				DefaultSpriteVertices[1] = { { .5f,-.5f,.0f} ,glm::vec2{1,0},0 };
				DefaultSpriteVertices[2] = { { .5f, .5f,.0f} ,glm::vec2{1,1},0 };
				DefaultSpriteVertices[3] = { {-.5f, .5f,.0f} ,glm::vec2{0,1},0 };
			}
		}


	};


	struct LineRendererData {
		static inline constexpr uint32_t MaxLines = 2048;
		static inline constexpr uint32_t MaxLineVertices = MaxLines * 2;


		Ref<VertexArray> LineVAO;
		Ref<VertexBuffer> LineVBO;
		Ref<Shader> LineShader;
		
		
		uint32_t RenderedLineCount = 0;
		uint32_t LinesWaitingForRender = 0;
		
		BoundedVector<LineVertex> Vertices = { 0,MaxLineVertices,[](auto data,uint64_t size) {
			Renderer2D::FlushLines();
		}};
		void Init() {
			LineShader = Shader::CreateFromPath("assets/shaders/GeometryPass/Geometry_Line_VS_2D.glsl", "assets/shaders/GeometryPass/Geometry_Line_VS_2D.glsl");
			//Vertex Array Object
			{
				LineVAO = VertexArray::Create();
			}

			//Vertex Buffer Object
			{
				LineVBO = VertexBuffer::Create(0);
				LineVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"},
					{ShaderDataType::Float4,"a_Color"}
				});
				LineVAO->AddVertexBuffer(LineVBO);
			}

		}
	};


	struct Renderer2DData {

		
		Ref<Framebuffer> OutputBuffer;

		struct Camera {
			glm::mat4 CameraViewProjection;
		};

		Camera CameraBuffer;

		Ref<UniformBuffer> CameraUniformBuffer;
		Ref<Material2D> DefaultMaterial;
		SpriteRendererData SpriteRendererData;
		LineRendererData LineRendererData;
	};


	static Renderer2DData s_Renderer2DData;
	
	void Renderer2D::Init(){
		
		//Sprite Renderer Init
		{
			s_Renderer2DData.SpriteRendererData.Init();
		}

		//Camera Uniform Buffer
		{
			s_Renderer2DData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::Camera), 0);
		}

		//Default Material
		{
			s_Renderer2DData.DefaultMaterial = CreateRef<Material2D>();
		}
		//Material2D Textures
		{
			Material2DTextureHandler::Init();
		}

	}
	void Renderer2D::Shutdown(){

	}
	void Renderer2D::Begin(const Renderer2DBeginData& beginData){
		s_Renderer2DData.OutputBuffer = beginData.OutputBuffer;
		s_Renderer2DData.CameraBuffer.CameraViewProjection = beginData.CameraVP;
		s_Renderer2DData.CameraUniformBuffer->SetData(&s_Renderer2DData.CameraBuffer, sizeof(Renderer2DData::Camera));
		s_Renderer2DData.CameraUniformBuffer->Bind();
		s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender = 0;
		s_Renderer2DData.SpriteRendererData.RenderedSpriteCount = 0;
		Material2DTextureHandler::GetTexturesMap()->Bind(0);
		Material2D::SetMaterials();
	}
	void Renderer2D::DrawSprite(const glm::mat4& transform, Ref<Material2D> material){

		if (!material) {
			DrawSprite(transform, s_Renderer2DData.DefaultMaterial);
			return;
		}

		SpriteVertex vertex[4];
		for (uint32_t i = 0; i < 4; ++i) {
			vertex[i].Position = transform * glm::vec4(s_Renderer2DData.SpriteRendererData.DefaultSpriteVertices[i].Position,1.0f);
			vertex[i].TexCoords = s_Renderer2DData.SpriteRendererData.DefaultSpriteVertices[i].TexCoords;
			vertex[i].MaterialID = material->GetIndex();
		}

		std::unique_lock<std::mutex> lock(s_Renderer2DData.SpriteRendererData.SpriteRenderingMutex);
		auto bvi = s_Renderer2DData.SpriteRendererData.Vertices.Reserve(4);
		for (uint32_t i = 0; i < 4; ++i) {
			bvi[i] = std::move(vertex[i]);
		}
		s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender++;
	}

	void Renderer2D::FlushSprites() {
		if (s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender) {
			s_Renderer2DData.OutputBuffer->Bind();
			s_Renderer2DData.SpriteRendererData.SpriteVBO->SetData(s_Renderer2DData.SpriteRendererData.Vertices.Get(), s_Renderer2DData.SpriteRendererData.Vertices.Size() * sizeof(SpriteVertex));
			s_Renderer2DData.SpriteRendererData.SpriteShader->Bind();
			RenderCommand::SetCullMode(CullMode::None);
			RenderCommand::DrawIndexed(s_Renderer2DData.SpriteRendererData.SpriteVAO, s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender * 6);
			s_Renderer2DData.SpriteRendererData.RenderedSpriteCount += s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender;
			s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender = 0;
			s_Renderer2DData.OutputBuffer->Unbind();
		}
		s_Renderer2DData.SpriteRendererData.Vertices.Reset();
	}

	void Renderer2D::End() {
		FlushSprites();
	}

}
