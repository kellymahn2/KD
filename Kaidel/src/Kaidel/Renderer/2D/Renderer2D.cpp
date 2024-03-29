#include "KDpch.h"

#include "Kaidel/Core/BoundedVector.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Renderer2D.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/MaterialTexture.h"

namespace Kaidel {


	CustomRenderer Renderer2D::CreatePointRenderer(const std::initializer_list<BufferElement>& additionalElements, Ref<Shader> shader) {

		const BufferLayout& defaults = s_Renderer2DData.PointRendererData.CustomRenderers.top().VAO->GetVertexBuffers().front()->GetLayout();

		BufferLayout layout = defaults;

		layout.Push(additionalElements);

		CustomRenderer renderer;

		Ref<VertexBuffer> vbo = VertexBuffer::Create(0);
		
		vbo->SetLayout(layout);

		{
			VertexArraySpecification spec;
			spec.UsedShader = shader;
			spec.VertexBuffers = { vbo };
			renderer.VAO = VertexArray::Create(spec);
		}
		renderer.Shader = shader;
		return renderer;
	}
	
	void Renderer2D::PushPointRenderer(const CustomRenderer& renderer) {
		s_Renderer2DData.PointRendererData.CustomRenderers.push(renderer);
	}
	void Renderer2D::PopPointRenderer() {
		KD_CORE_ASSERT(s_Renderer2DData.PointRendererData.CustomRenderers.size() > 1);
		s_Renderer2DData.PointRendererData.CustomRenderers.pop();
	}

	#pragma region Initialization
	void Renderer2D::SpriteRendererData::Init() {
		{
			using path = FileSystem::path;
			SpriteShader = Shader::Create({ {"assets/shaders/GeometryPass/Geometry_Sprite_VS_2D.glsl",ShaderType::VertexShader}, {"assets/shaders/GeometryPass/Geometry_Sprite_FS_2D.glsl",ShaderType::FragmentShader} });
			Ref<IndexBuffer> ibo;
			//Index Buffer Object
			{
				uint32_t* indices = SetupSpriteIndices();
				ibo = IndexBuffer::Create(indices, MaxSpriteIndices);
				delete[] indices;
			}

			//Vertex Buffer Object
			{
				SpriteVBO = VertexBuffer::Create(0);
				SpriteVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"},
					{ShaderDataType::Float2,"a_TexCoords"},
					{ShaderDataType::Int,"a_MaterialID"}
					});
			}


			//Vertex Array Object
			{
				VertexArraySpecification spec;
				spec.VertexBuffers = { SpriteVBO };
				spec.IndexBuffer = ibo;
				spec.UsedShader = SpriteShader;
				SpriteVAO = VertexArray::Create(spec);
			}


			//Default Sprite Vertices
			{
				DefaultSpriteVertices[0] = { glm::vec3{-.5f,-.5f,.0f} ,glm::vec2{0,0},0 };
				DefaultSpriteVertices[1] = { glm::vec3{ .5f,-.5f,.0f} ,glm::vec2{1,0},0 };
				DefaultSpriteVertices[2] = { glm::vec3{ .5f, .5f,.0f} ,glm::vec2{1,1},0 };
				DefaultSpriteVertices[3] = { glm::vec3{-.5f, .5f,.0f} ,glm::vec2{0,1},0 };
			}
		}
	}

	void Renderer2D::LineRendererData::Init() {
		{
			LineShader = Shader::Create({ { "assets/shaders/GeometryPass/Geometry_Line_VS_2D.glsl" ,ShaderType::VertexShader}, {"assets/shaders/GeometryPass/Geometry_Line_FS_2D.glsl",ShaderType::FragmentShader } });

			//Vertex Buffer Object
			{
				LineVBO = VertexBuffer::Create(0);
				LineVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"},
					{ShaderDataType::Float4,"a_Color"}
					});
			}

			//Vertex Array Object
			{
				VertexArraySpecification spec;
				spec.VertexBuffers = { LineVBO };
				spec.UsedShader = LineShader;
				LineVAO = VertexArray::Create(spec);
			}

		}
	}

	void Renderer2D::BezierRendererData::Init() {
		{

			ShaderSpecification bezierShaderSpecification;

			bezierShaderSpecification.Definitions = {
				{"assets/shaders/GeometryPass/Geometry_Bezier_VS_2D.glsl",ShaderType::VertexShader},
				{"assets/shaders/GeometryPass/Geometry_Bezier_FS_2D.glsl",ShaderType::FragmentShader},
				{"assets/shaders/GeometryPass/Geometry_Bezier_TES_2D.glsl",ShaderType::TessellationEvaluationShader}
			};

			BezierShader = Shader::Create(bezierShaderSpecification);

			//Vertex Buffer Object
			{
				BezierVBO = VertexBuffer::Create(0);
				BezierVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"}
					});
			}

			//Vertex Array Object
			{
				VertexArraySpecification spec;
				spec.VertexBuffers = { BezierVBO };
				spec.UsedShader = BezierShader;

				BezierVAO = VertexArray::Create(spec);
			}



		}
	}

	void Renderer2D::PointRendererData::Init() {
		{
			Ref<Shader> pointShader = Shader::Create({ {"assets/shaders/GeometryPass/Geometry_Point_VS_2D.glsl",ShaderType::VertexShader},{"assets/shaders/GeometryPass/Geometry_Point_FS_2D.glsl",ShaderType::FragmentShader} });




			//Vertex Buffer Object
			Ref<VertexBuffer> pointVBO = VertexBuffer::Create(0);
			{
				pointVBO->SetLayout({
					{ShaderDataType::Float3,"a_Position"},
					{ShaderDataType::Float4,"a_Color"}
					});
			}



			CustomRenderer renderer;

			//Vertex Array Object
			{
				VertexArraySpecification spec;
				spec.VertexBuffers = { pointVBO};
				spec.UsedShader = pointShader;

				renderer.VAO = VertexArray::Create(spec);
				renderer.Shader = pointShader;
			}

			CustomRenderers.push(renderer);



		}
	}


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

	}

	#pragma endregion



	void Renderer2D::Shutdown() {

	}
	
	Ref<Material2D> Renderer2D::GetDefaultMaterial() {
		return s_Renderer2DData.DefaultMaterial;
	}


	Ref<Framebuffer> Renderer2D::GetOutputFramebuffer() {
		return s_Renderer2DData.OutputBuffer;
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



		MaterialTexture::GetTextureArray()->Bind(0);
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
		auto bvi = VerticesBuffer<SpriteVertex,MaxSpriteVertices>::Vertices.Reserve(4);
		for (uint32_t i = 0; i < 4; ++i) {
			bvi[i] = std::move(vertex[i]);
		}
		s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender++;
	}

	void Renderer2D::FlushSprites() {

		auto& vertices = VerticesBuffer<SpriteVertex, MaxSpriteVertices>::Vertices;

		if (s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender) {
			s_Renderer2DData.OutputBuffer->Bind();
			s_Renderer2DData.SpriteRendererData.SpriteVBO->SetData(vertices.Get(), vertices.Size() * sizeof(SpriteVertex));
			s_Renderer2DData.SpriteRendererData.SpriteShader->Bind();
			RenderCommand::SetCullMode(CullMode::None);
			RenderCommand::DrawIndexed(s_Renderer2DData.SpriteRendererData.SpriteVAO, s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender * 6);
			s_Renderer2DData.SpriteRendererData.RenderedSpriteCount += s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender;
			s_Renderer2DData.SpriteRendererData.SpritesWaitingForRender = 0;
			s_Renderer2DData.OutputBuffer->Unbind();
		}
		vertices.Reset();
	}

#pragma endregion
#pragma region Bezier

	void Renderer2D::GetSegmentCount(float totalSegmentCount, float* lineCount, float* segmentPerLineCount) {
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

		auto bvi = VerticesBuffer<LineVertex,MaxLineVertices>::Vertices.Reserve(2);
		bvi[0] = vertices[0];
		bvi[1] = vertices[1];
		++s_Renderer2DData.LineRendererData.LinesWaitingForRender;
	}

	void Renderer2D::FlushLines() {

		auto& vertices = VerticesBuffer<LineVertex, MaxLineVertices>::Vertices;

		if (s_Renderer2DData.LineRendererData.LinesWaitingForRender) {
			s_Renderer2DData.OutputBuffer->Bind();
			s_Renderer2DData.LineRendererData.LineVBO->SetData(vertices.Get(), vertices.Size() * sizeof(LineVertex));
			s_Renderer2DData.LineRendererData.LineShader->Bind();
			RenderCommand::SetCullMode(CullMode::None);
			RenderCommand::DrawLines(s_Renderer2DData.LineRendererData.LineVAO, vertices.Size());
			s_Renderer2DData.LineRendererData.RenderedLineCount+= s_Renderer2DData.LineRendererData.LinesWaitingForRender;
			s_Renderer2DData.LineRendererData.LinesWaitingForRender = 0;
			s_Renderer2DData.OutputBuffer->Unbind();
		}
		vertices.Reset();
	}

#pragma endregion
#pragma region Point


#pragma endregion

	void Renderer2D::End() {
		FlushSprites();
		FlushLines();
		FlushPoints<0>();
	}

}
