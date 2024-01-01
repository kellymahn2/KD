#include "KDpch.h"
#include "Kaidel/Renderer/Renderer2D.h"

#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Shader.h"
#include "Kaidel/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include "UniformBuffer.h"
#include "Kaidel/Core/Timer.h"

#include "Kaidel\Core\ThreadExecutor.h"
#include "Kaidel/Core/BoundedVector.h"
namespace Kaidel {
	
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;

		// Editor-only
		int EntityID;
	};
	struct CircleVertex {
		glm::vec3 Position;
		glm::vec3 localPosition;
		glm::vec4 Color;
		float Thickness;
		float Fade;

		// Editor-only
		int EntityID;
	};
	struct LineVertex {
		glm::vec3 Position;
		glm::vec4 Color;
		int EntityID;
	};
	struct Renderer2DData
	{
		static constexpr uint32_t MaxQuads = 10000;
		static constexpr uint32_t MaxVertices = MaxQuads * 4;
		static constexpr uint32_t MaxIndices = MaxQuads * 6;
		static constexpr uint32_t MaxTextureSlots = 32; // TODO: RenderCaps

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;

		uint32_t QuadIndexCount = 0;
		BoundedVector < QuadVertex> QuadVertexBufferArray = { 10,MaxVertices ,[](auto start,uint64_t size) {
			Renderer2D::FlushQuads(start,size);
			Renderer2D::StartQuadBatch();
			} };
		std::mutex QuadMutex;
		size_t QuadVertexBufferIndex = 0;
		//QuadVertex* QuadVertexBufferBase = nullptr;
		//QuadVertex* QuadVertexBufferPtr = nullptr;

		Ref<VertexArray> CircleVertexArray;
		Ref<VertexBuffer> CircleVertexBuffer;
		Ref<Shader> CircleShader;

		uint32_t CircleIndexCount = 0;
		BoundedVector<CircleVertex> CircleVertexBufferArray = { 10,MaxVertices,[](auto start,uint64_t size) {
			Renderer2D::FlushCircles(start,size);
			Renderer2D::StartCircleBatch();
			} };
		std::mutex CircleMutex;




		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		Ref<Shader> LineShader;
		std::mutex LineMutex;
		uint32_t LineVertexCount = 0;
		BoundedVector<LineVertex> LineVertexBufferArray = { 10,MaxVertices,[](auto start,uint64_t size) {
			Renderer2D::FlushLines(start,size);
			Renderer2D::StartLineBatch();
			} };

		Ref<Texture2D> WhiteTexture;
		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		glm::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;
		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
		float LineWidth = 2.0f;
	};

	static Renderer2DData s_Data;
	static uint32_t* SetupQuadIndices() {
		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
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
	void Renderer2D::Init()
	{
		KD_PROFILE_FUNCTION();
		//TODO : Make the other shaders.
		/*switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
		{
			s_Data.LineShader = Shader::Create("assets/shaders/Line.glsl");
			s_Data.QuadShader = Shader::Create("assets/shaders/Quad.glsl");
			s_Data.CircleShader = Shader::Create("assets/shaders/Circle.glsl");
			break;
		}
		case RendererAPI::API::DirectX:
		{

			s_Data.LineShader = Shader::Create("assets/shaders/Line.kdShader");
			s_Data.QuadShader = Shader::Create("assets/shaders/Quad.kdShader");
			s_Data.CircleShader = Shader::Create("assets/shaders/Circle.kdShader");
			break;
		}
		}*/

		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
		{
			s_Data.QuadVertexArray = VertexArray::Create();
			break;
		}
		case RendererAPI::API::DirectX:
		{

			s_Data.QuadVertexArray = VertexArray::Create(s_Data.QuadShader);
			break;
		}
		}
		s_Data.QuadVertexBuffer = VertexBuffer::Create(0);
		s_Data.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_Texoord"     },
			{ ShaderDataType::Float,  "a_TexIndex"     },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,    "a_EntityID"     }
			});
		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		//s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

		uint32_t* quadIndices = SetupQuadIndices();
		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);
		s_Data.QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;



		//Circles

		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
		{
			s_Data.CircleVertexArray = VertexArray::Create();
			break;
		}
		case RendererAPI::API::DirectX:
		{

			s_Data.CircleVertexArray = VertexArray::Create(s_Data.CircleShader);
			break;
		}
		}

		s_Data.CircleVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex));
		s_Data.CircleVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float3, "a_LocalPosition"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float , "a_Thickness"    },
			{ ShaderDataType::Float , "a_Fade"         },
			{ ShaderDataType::Int   , "a_EntityID"     }
			});
		s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);
		s_Data.CircleVertexArray->SetIndexBuffer(quadIB);



		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
		{
			s_Data.LineVertexArray = VertexArray::Create();
			break;
		}
		case RendererAPI::API::DirectX:
		{

			s_Data.LineVertexArray = VertexArray::Create(s_Data.LineShader);
			break;
		}
		}

		s_Data.LineVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex));
		s_Data.LineVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Int   , "a_EntityID"     }
			});
		s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
		//s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];




		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));








		// Set first texture slot to 0
		s_Data.TextureSlots[0] = s_Data.WhiteTexture;

		s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
	}

	void Renderer2D::Shutdown()
	{
		KD_PROFILE_FUNCTION();

		//delete[] s_Data.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		KD_PROFILE_FUNCTION();

		s_Data.QuadShader->Bind();
		s_Data.QuadShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		StartBatch();
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		KD_PROFILE_FUNCTION();

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraBuffer.ViewProjection = viewProj;
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		KD_PROFILE_FUNCTION();

		
		s_Data.CameraBuffer.ViewProjection = camera.GetProjection()*camera.GetViewMatrix();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		KD_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::SetVertexBufferValues(uint32_t vertexCount, const glm::mat4& transform, const glm::vec4& tintColor, const glm::vec2* textureCoords, float textureIndex, float tilingFactor, int entityID)
	{
		for (size_t i = 0; i < vertexCount; i++)
		{
			/*s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].Color = tintColor;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TexIndex = textureIndex;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].EntityID = entityID;
			s_Data.QuadVertexBufferIndex++;*/
		}
	}

	void Renderer2D::StartQuadBatch() {
		s_Data.QuadIndexCount = 0;
		s_Data.QuadVertexBufferArray.Reset();

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::StartCircleBatch()
	{
		s_Data.CircleIndexCount = 0;
		s_Data.CircleVertexBufferArray.Reset();
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::StartLineBatch()
	{
		s_Data.LineVertexCount = 0;
		s_Data.LineVertexBufferArray.Reset();
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::StartBatch()
	{
		StartQuadBatch();
		StartCircleBatch();
		StartLineBatch();
	}

	void Renderer2D::Flush()
	{
		std::scoped_lock<std::mutex> quadLock(s_Data.QuadMutex);
		std::scoped_lock<std::mutex> circleLock(s_Data.CircleMutex);
		std::scoped_lock<std::mutex> lineLock(s_Data.LineMutex);
		FlushQuads(s_Data.QuadVertexBufferArray.Get(),s_Data.QuadVertexBufferArray.Size());
		FlushCircles(s_Data.CircleVertexBufferArray.Get(), s_Data.CircleVertexBufferArray.Size());
		FlushLines(s_Data.LineVertexBufferArray.Get(), s_Data.LineVertexBufferArray.Size());
	}

	void Renderer2D::FlushQuads(QuadVertex* start, uint64_t size)
	{
		if (s_Data.QuadIndexCount) {
			s_Data.QuadVertexBuffer->SetData(start,
				(size) * sizeof(QuadVertex));
			s_Data.QuadVertexBuffer->Bind();
			//s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
			s_Data.QuadVertexArray->GetIndexBuffer()->Bind();
			s_Data.CameraUniformBuffer->Bind();
			s_Data.QuadVertexArray->Bind();
			s_Data.QuadShader->Bind();
			// Bind textures
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++) {
				s_Data.TextureSlots[i]->Bind(i);
			}
			RenderCommand::DrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
			s_Data.QuadShader->Unbind();
			s_Data.QuadVertexArray->Unbind();
			s_Data.CameraUniformBuffer->UnBind();
			s_Data.QuadVertexArray->GetIndexBuffer()->Unbind();
			s_Data.QuadVertexBuffer->Unbind();
			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer2D::FlushCircles(CircleVertex* start, uint64_t size)
	{

		if (s_Data.CircleIndexCount) {

			s_Data.CircleVertexBuffer->SetData(start, (size) * sizeof(CircleVertex));
			s_Data.CircleVertexBuffer->Bind();
			//s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
			s_Data.CircleVertexArray->GetIndexBuffer()->Bind();
			s_Data.CameraUniformBuffer->Bind();
			s_Data.CircleVertexArray->Bind();
			s_Data.CircleShader->Bind();
			RenderCommand::DrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
			s_Data.CircleShader->Unbind();
			s_Data.CircleVertexArray->Unbind();
			s_Data.CameraUniformBuffer->UnBind();
			s_Data.CircleVertexArray->GetIndexBuffer()->Unbind();
			s_Data.CircleVertexBuffer->Unbind();
			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer2D::FlushLines(LineVertex* start,uint64_t size)
	{
		if (s_Data.LineVertexCount) {
			s_Data.LineVertexBuffer->SetData(start,(size)*sizeof(LineVertex));
			s_Data.LineVertexBuffer->Bind();
			s_Data.CameraUniformBuffer->Bind();
			s_Data.LineVertexArray->Bind();
			s_Data.LineShader->Bind();
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
			s_Data.LineShader->Unbind();
			s_Data.LineVertexArray->Unbind();
			s_Data.CameraUniformBuffer->UnBind();
			s_Data.LineVertexBuffer->Unbind();
			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer2D::NextBatch()
	{
		std::scoped_lock<std::mutex> lock(s_Data.QuadMutex);
		Flush();
		StartBatch();
	}


	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{

		constexpr size_t quadVertexCount = 4;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		const float tilingFactor = 1.0f;

		SetVertexBufferValues(quadVertexCount, transform, color, textureCoords, textureIndex, tilingFactor, entityID);
		/*for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].Color = color;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TexIndex = textureIndex;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].EntityID = entityID;
			s_Data.QuadVertexBufferIndex++;
		}*/
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		KD_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices || s_Data.QuadVertexBufferIndex == s_Data.MaxVertices)
			NextBatch();

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i] == *texture)
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		SetVertexBufferValues(quadVertexCount, transform, tintColor, textureCoords, textureIndex, tilingFactor, entityID);

		/*for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].Color = tintColor;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TexCoord = textureCoords[i];
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TexIndex = textureIndex;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferArray[s_Data.QuadVertexBufferIndex].EntityID = entityID;
			s_Data.QuadVertexBufferIndex++;
		}*/
		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}


	void SetQuadVertexValues(QuadVertex& qv, const glm::mat4& transform, const glm::vec4& color, const glm::vec4& position, const glm::vec2& textureCoord, int entityID) {
		qv.Position = transform * position;
		qv.Color = color;
		qv.TexCoord = textureCoord;
		qv.TexIndex = 0.0f;
		qv.TilingFactor = 1.0f;
		qv.EntityID = entityID;
	}
	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, uint64_t QuadIndex, int entityID/*=-1*/)
	{
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		QuadVertex first{};
		SetQuadVertexValues(first, transform, color, s_Data.QuadVertexPositions[0], textureCoords[0], entityID);
		QuadVertex second{};
		SetQuadVertexValues(second, transform, color, s_Data.QuadVertexPositions[1], textureCoords[1], entityID);
		QuadVertex third{};
		SetQuadVertexValues(third, transform, color, s_Data.QuadVertexPositions[2], textureCoords[2], entityID);
		QuadVertex fourth{};
		SetQuadVertexValues(fourth, transform, color, s_Data.QuadVertexPositions[3], textureCoords[3], entityID);
		std::scoped_lock<std::mutex> lock(s_Data.QuadMutex);
		auto bvi =  s_Data.QuadVertexBufferArray.Reserve(4);
		bvi[0] = first;
		bvi[1] = second;
		bvi[2] = third;
		bvi[3] = fourth;
		s_Data.QuadIndexCount += 6;
		s_Data.Stats.QuadCount += 1;
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness /*= 1.0f*/, float fade /*= .005f*/, int entityID /*= -1*/)
	{
		KD_PROFILE_FUNCTION();

		/*constexpr size_t quadVertexCount = 4;


		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.CircleVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.CircleVertexBufferPtr->localPosition = s_Data.QuadVertexPositions[i] * 2.0f;
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr->EntityID = entityID;
			s_Data.CircleVertexBufferPtr++;
		}

		s_Data.CircleIndexCount += 6;

		s_Data.Stats.CircleCount++;*/

	}
	void SetCircleVertexValues(CircleVertex& cv, const glm::mat4& transform, const glm::vec4& color,const glm::vec4& position, float Thickness, float fade, int entityID) {
		cv.Position = transform * position;
		cv.localPosition = position * 2.0f;
		cv.Color = color;
		cv.Thickness = Thickness;
		cv.Fade = fade;
		cv.EntityID = entityID;
	}
	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, uint64_t insertIndex, float Thickness /*= 1.0f*/, float fade /*= .005f*/, int entityID /*= -1*/)
	{
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		CircleVertex first{};
		SetCircleVertexValues(first, transform, color, s_Data.QuadVertexPositions[0], Thickness,fade,entityID);
		CircleVertex second{};
		SetCircleVertexValues(second, transform, color, s_Data.QuadVertexPositions[1],Thickness,fade, entityID);
		CircleVertex third{};
		SetCircleVertexValues(third, transform, color, s_Data.QuadVertexPositions[2], Thickness,fade, entityID);
		CircleVertex fourth{};
		SetCircleVertexValues(fourth, transform, color, s_Data.QuadVertexPositions[3],Thickness,fade, entityID);
		std::lock_guard<std::mutex> lock(s_Data.CircleMutex);
		auto bvi = s_Data.CircleVertexBufferArray.Reserve(4);
		bvi[0] = first;
		bvi[1] = second;
		bvi[2] = third;
		bvi[3] = fourth;
		s_Data.CircleIndexCount += 6;
		s_Data.Stats.CircleCount+= 1;
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		KD_PROFILE_FUNCTION();
		if (src.Texture)
			DrawQuad(transform, src.Texture,src.TilingFactor,src.Color, entityID);
		else
			DrawQuad(transform, src.Color, entityID);
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID/*=-1*/)
	{
		/*s_Data.LineVertexBufferPtr->Position = p0;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;
		s_Data.LineVertexBufferPtr->Position = p1;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr->EntityID = entityID;
		s_Data.LineVertexBufferPtr++;
		s_Data.LineVertexCount += 2;*/
	}
	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, uint64_t insertIndex, int entityID /*= -1*/)
	{
		LineVertex first{};
		first.Position = p0;
		first.Color = color;
		first.EntityID = entityID;
		LineVertex second{};
		second.Position = p1;
		second.Color = color;
		second.EntityID = entityID;
		std::scoped_lock<std::mutex> lock(s_Data.LineMutex);
		auto bvi = s_Data.LineVertexBufferArray.Reserve(4);
		bvi[0] = first;
		bvi[1] = second;
		s_Data.LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID /*= -1*/)
	{
		glm::vec3 p0 = { position.x - size.x * .5f,position.y - size.y * .5f ,position.z};
		glm::vec3 p1 = { position.x + size.x * .5f,position.y - size.y * .5f ,position.z };
		glm::vec3 p2 = { position.x + size.x * .5f,position.y + size.y * .5f ,position.z };
		glm::vec3 p3 = { position.x - size.x * .5f,position.y + size.y * .5f ,position.z };

		DrawLine(p0, p1, color, entityID);
		DrawLine(p1, p2, color, entityID);
		DrawLine(p2, p3, color, entityID);
		DrawLine(p3, p0, color, entityID);

	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID /*= -1*/)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; ++i) {
			lineVertices[i] = transform * s_Data.QuadVertexPositions[i];
		}
		DrawLine(lineVertices[0], lineVertices[1], color, entityID);
		DrawLine(lineVertices[1], lineVertices[2], color, entityID);
		DrawLine(lineVertices[2], lineVertices[3], color, entityID);
		DrawLine(lineVertices[3], lineVertices[0], color, entityID);
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}
	float Renderer2D::GetLineWidth() {
		return s_Data.LineWidth;
	}
	void Renderer2D::SetLineWidth(float width){
		s_Data.LineWidth = width;
		RenderCommand::SetLineWidth(width);
	}
	Ref<Texture2D> Renderer2D::GetWhite() {
		return s_Data.WhiteTexture;
	}
}
