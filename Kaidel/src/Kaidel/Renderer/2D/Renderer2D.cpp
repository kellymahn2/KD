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
			SpriteShader = Shader::Create({ {"assets/shaders/GeometryPass/Geometry_Sprite_VS_2D.glsl",ShaderType::VertexShader}, {"assets/shaders/GeometryPass/Geometry_Sprite_FS_2D.glsl",ShaderType::FragmentShader} });
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
		} };
		void Init() {
			LineShader = Shader::Create({ { "assets/shaders/GeometryPass/Geometry_Line_VS_2D.glsl" ,ShaderType::VertexShader}, {"assets/shaders/GeometryPass/Geometry_Line_FS_2D.glsl",ShaderType::FragmentShader }});
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

	struct BezierRendererData {
		
		Ref<Shader> BezierShader;

		Ref<VertexArray> BezierVAO;
		Ref<VertexBuffer> BezierVBO;

		void Init() {

			ShaderSpecification bezierShaderSpecification;

			bezierShaderSpecification.Definitions = {
				{"assets/shaders/GeometryPass/Geometry_Bezier_VS_2D.glsl",ShaderType::VertexShader},
				{"assets/shaders/GeometryPass/Geometry_Bezier_FS_2D.glsl",ShaderType::FragmentShader},
				{"assets/shaders/GeometryPass/Geometry_Bezier_TES_2D.glsl",ShaderType::TessellationEvaluationShader}
			};

			BezierShader = Shader::Create(bezierShaderSpecification);
			
			//Vertex Array Object
			{
				BezierVAO = VertexArray::Create();
			}

			//Vertex Buffer Object
			{
				BezierVBO = VertexBuffer::Create(0);
				BezierVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"}
					});
				BezierVAO->AddVertexBuffer(BezierVBO);
			}
			
		}
	};

	struct PointRendererData {

		static inline constexpr uint32_t MaxPoints = 2048;

		Ref<Shader> PointShader;
		Ref<VertexArray> PointVAO;
		Ref<VertexBuffer> PointVBO;


		uint32_t RenderedPointCount = 0;
		uint32_t PointsWaitingForRender = 0;

		BoundedVector<PointVertex> Vertices = { 0,MaxPoints,[](auto ptr,auto size) {
			Renderer2D::FlushPoints();
		}};

		void Init() {
			PointShader = Shader::Create({ {"assets/shaders/GeometryPass/Geometry_Point_VS_2D.glsl",ShaderType::VertexShader},{"assets/shaders/GeometryPass/Geometry_Point_FS_2D.glsl",ShaderType::FragmentShader} });
			
			//Vertex Array Object
			{
				PointVAO = VertexArray::Create();
			}

			//Vertex Buffer Object
			{
				PointVBO = VertexBuffer::Create(0);
				PointVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"},
					{ShaderDataType::Float4,"a_Color"}
				});
				PointVAO->AddVertexBuffer(PointVBO);
				
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
		BezierRendererData BezierRendererData;
		PointRendererData PointRendererData;
	};


	static Renderer2DData s_Renderer2DData;

	void Renderer2D::Init() {

		//Sprite Renderer Init
		{
			s_Renderer2DData.SpriteRendererData.Init();
		}

		//Line Renderer Init
		{
			s_Renderer2DData.LineRendererData.Init();
		}

		//Bezier Renderer Init
		{
			s_Renderer2DData.BezierRendererData.Init();
		}

		//Point Renderer Init
		{
			s_Renderer2DData.PointRendererData.Init();
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
	void Renderer2D::Shutdown() {

	}
	void Renderer2D::Begin(const Renderer2DBeginData& beginData) {
		s_Renderer2DData.OutputBuffer = beginData.OutputBuffer;
		s_Renderer2DData.CameraBuffer.CameraViewProjection = beginData.CameraVP;
		s_Renderer2DData.CameraUniformBuffer->SetData(&s_Renderer2DData.CameraBuffer, sizeof(Renderer2DData::Camera));
		s_Renderer2DData.CameraUniformBuffer->Bind();
		s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender = 0;
		s_Renderer2DData.SpriteRendererData.RenderedSpriteCount = 0;
		s_Renderer2DData.LineRendererData.LinesWaitingForRender = 0;
		s_Renderer2DData.LineRendererData.RenderedLineCount = 0;
		s_Renderer2DData.PointRendererData.PointsWaitingForRender = 0;
		s_Renderer2DData.PointRendererData.RenderedPointCount = 0;
		Material2DTextureHandler::GetTexturesMap()->Bind(0);
		Material2D::SetMaterials();
	}

#pragma region Sprite
	void Renderer2D::DrawSprite(const glm::mat4& transform, Ref<Material2D> material) {

		if (!material) {
			DrawSprite(transform, s_Renderer2DData.DefaultMaterial);
			return;
		}

		SpriteVertex vertex[4];
		for (uint32_t i = 0; i < 4; ++i) {
			vertex[i].Position = transform * glm::vec4(s_Renderer2DData.SpriteRendererData.DefaultSpriteVertices[i].Position, 1.0f);
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

#pragma endregion
#pragma region Bezier

	static void GetSegmentCount(float totalSegmentCount, float* lineCount, float* segmentPerLineCount) {
		float maxTessLevel = RenderCommand::QueryMaxTessellationLevel();
		for (float i = 1.0f; i < maxTessLevel; i += 1.0f) {
			for (float j = 1.0f; j < maxTessLevel; j += 1.0f) {
				if (i * j >= totalSegmentCount) {
					*lineCount = i;
					*segmentPerLineCount = j;
					return;
				}
			}
		}
		*lineCount = maxTessLevel;
		*segmentPerLineCount = maxTessLevel;
	}

	void Renderer2D::DrawBezier(const glm::mat4& transform, const std::vector<glm::vec3>& points, const glm::vec4& color, float increment) {

		glm::mat4 mvp = s_Renderer2DData.CameraBuffer.CameraViewProjection * transform;
		s_Renderer2DData.BezierRendererData.BezierVBO->SetData(points.data(), points.size() * sizeof(glm::vec3));
		s_Renderer2DData.BezierRendererData.BezierShader->Bind();
		s_Renderer2DData.BezierRendererData.BezierShader->SetMat4("u_MVP", s_Renderer2DData.CameraBuffer.CameraViewProjection * transform);

		s_Renderer2DData.BezierRendererData.BezierShader->SetInt("u_NumControlPoints", points.size());
		s_Renderer2DData.BezierRendererData.BezierShader->SetFloat4("u_Color",color);

		float totalNumSegments = ceilf(1.0f / (float)increment);
		float numLines;
		float numSegmentsPerLine;
		
		GetSegmentCount(totalNumSegments, &numLines, &numSegmentsPerLine);

		s_Renderer2DData.OutputBuffer->Bind();
		RenderCommand::SetCullMode(CullMode::None);
		RenderCommand::SetPatchVertexCount(points.size());
		RenderCommand::SetDefaultTessellationLevels({ numLines,numSegmentsPerLine,1.0,1.0 });
		RenderCommand::DrawPatches(s_Renderer2DData.BezierRendererData.BezierVAO, points.size());
		RenderCommand::SetPatchVertexCount(3);
		RenderCommand::SetDefaultTessellationLevels();
		s_Renderer2DData.OutputBuffer->Unbind();

	}

#pragma endregion
#pragma region Line

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color) {
		LineVertex vertices[2];
		vertices[0].Position = p0;
		vertices[0].Color = color;
		vertices[1].Position = p1;
		vertices[1].Color = color;

		auto bvi = s_Renderer2DData.LineRendererData.Vertices.Reserve(2);
		bvi[0] = vertices[0];
		bvi[1] = vertices[1];
		++s_Renderer2DData.LineRendererData.LinesWaitingForRender;
	}

	void Renderer2D::FlushLines() {
		if (s_Renderer2DData.LineRendererData.LinesWaitingForRender) {
			s_Renderer2DData.OutputBuffer->Bind();
			s_Renderer2DData.LineRendererData.LineVBO->SetData(s_Renderer2DData.LineRendererData.Vertices.Get(), s_Renderer2DData.LineRendererData.Vertices.Size() * sizeof(LineVertex));
			s_Renderer2DData.LineRendererData.LineShader->Bind();
			RenderCommand::SetCullMode(CullMode::None);
			RenderCommand::DrawLines(s_Renderer2DData.LineRendererData.LineVAO, s_Renderer2DData.LineRendererData.Vertices.Size());
			s_Renderer2DData.LineRendererData.RenderedLineCount+= s_Renderer2DData.LineRendererData.LinesWaitingForRender;
			s_Renderer2DData.LineRendererData.LinesWaitingForRender = 0;
			s_Renderer2DData.OutputBuffer->Unbind();
		}
		s_Renderer2DData.LineRendererData.Vertices.Reset();
	}

#pragma endregion
#pragma region Point

	void Renderer2D::DrawPoint(const glm::vec3& position, const glm::vec4& color) {
		auto bvi = s_Renderer2DData.PointRendererData.Vertices.Reserve(1);
		bvi[0] = { position,color };
		++s_Renderer2DData.PointRendererData.PointsWaitingForRender;
	}

	void Renderer2D::FlushPoints() {
		if (s_Renderer2DData.PointRendererData.PointsWaitingForRender) {
			s_Renderer2DData.OutputBuffer->Bind();
			s_Renderer2DData.PointRendererData.PointVBO->SetData(s_Renderer2DData.PointRendererData.Vertices.Get(), s_Renderer2DData.PointRendererData.Vertices.Size()*sizeof(PointVertex));
			s_Renderer2DData.PointRendererData.PointShader->Bind();
			RenderCommand::SetCullMode(CullMode::None);
			RenderCommand::DrawPoints(s_Renderer2DData.PointRendererData.PointVAO, s_Renderer2DData.PointRendererData.PointsWaitingForRender);
			s_Renderer2DData.PointRendererData.RenderedPointCount += s_Renderer2DData.PointRendererData.PointsWaitingForRender;
			s_Renderer2DData.PointRendererData.PointsWaitingForRender = 0;
			s_Renderer2DData.OutputBuffer->Unbind();
		}
		s_Renderer2DData.PointRendererData.Vertices.Reset();
	}

#pragma endregion

	void Renderer2D::End() {
		FlushSprites();
		FlushLines();
		FlushPoints();
	}

}
