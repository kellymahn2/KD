#include "KDpch.h"
#include "GeometryFlusher.h"
#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Shader.h"
#include "Kaidel/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include "UniformBuffer.h"
#include "Texture.h"
#include "Material.h"
namespace Kaidel {

	static struct GeometryFlusherData {
		static constexpr uint32_t MaxCubes = 1000;
		static constexpr uint32_t MaxVertices = MaxCubes * 24;
		static constexpr uint32_t MaxIndices = MaxCubes * 36;
		static constexpr uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<Shader> CubeShader;
		uint32_t CubeIndexCount = 0;
		BoundedVector <CubeVertex> CubeVertexBufferArray = { 10,MaxVertices ,[](auto start,uint64_t size) {
			GeometryFlusher::FlushCubes(start,size);
			GeometryFlusher::StartCubeBatch();
			} };
		size_t CubeVertexBufferIndex = 0;


		Ref<Texture2D> WhiteTexture;
		BoundedVector <Ref<Texture2D>> TextureSlots;

		uint32_t TextureSlotIndex = 1; // 0 = white texture


		

		struct CameraData
		{
			glm::mat4 ViewProjection{};
			glm::vec3 CameraPosition{};
		};


		CameraData CameraBuffer{};
		Ref<UniformBuffer> CameraUniformBuffer;

		Ref<Texture2DArray> MaterialTextures;

		GeometryFlusher* Flusher = nullptr;
		uint32_t* SetupCubeIndices() {
			uint32_t* cubeIndices = new uint32_t[MaxIndices];

			uint32_t offset = 0;
			for (uint32_t i = 0; i < MaxIndices; i += 36)
			{
				cubeIndices[i + 0] = offset + 0;
				cubeIndices[i + 1] = offset + 1;
				cubeIndices[i + 2] = offset + 2;

				cubeIndices[i + 3] = offset + 2;
				cubeIndices[i + 4] = offset + 3;
				cubeIndices[i + 5] = offset + 0;

				cubeIndices[i + 6] = offset + 4;
				cubeIndices[i + 7] = offset + 5;
				cubeIndices[i + 8] = offset + 6;

				cubeIndices[i + 9] = offset + 6;
				cubeIndices[i + 10] = offset + 7;
				cubeIndices[i + 11] = offset + 4;

				cubeIndices[i + 12] = offset + 8;
				cubeIndices[i + 13] = offset + 9;
				cubeIndices[i + 14] = offset + 10;

				cubeIndices[i + 15] = offset + 10;
				cubeIndices[i + 16] = offset + 11;
				cubeIndices[i + 17] = offset + 8;

				cubeIndices[i + 18] = offset + 12;
				cubeIndices[i + 19] = offset + 13;
				cubeIndices[i + 20] = offset + 14;

				cubeIndices[i + 21] = offset + 14;
				cubeIndices[i + 22] = offset + 15;
				cubeIndices[i + 23] = offset + 12;

				cubeIndices[i + 24] = offset + 16;
				cubeIndices[i + 25] = offset + 17;
				cubeIndices[i + 26] = offset + 18;

				cubeIndices[i + 27] = offset + 18;
				cubeIndices[i + 28] = offset + 19;
				cubeIndices[i + 29] = offset + 16;

				cubeIndices[i + 30] = offset + 20;
				cubeIndices[i + 31] = offset + 21;
				cubeIndices[i + 32] = offset + 22;

				cubeIndices[i + 33] = offset + 22;
				cubeIndices[i + 34] = offset + 23;
				cubeIndices[i + 35] = offset + 20;

				offset += 24;
			}
			return cubeIndices;
		}

		GeometryFlusherData() {

			TextureSlots = { 0,(uint64_t)RenderCommand::QueryMaxTextureSlots(),[&](auto start,uint64_t size) {
				TextureSlotIndex = (uint32_t)size;
				Flusher->Flush();
				} };

			CameraUniformBuffer = UniformBuffer::Create(80, 0);

			switch (RendererAPI::GetAPI())
			{
			case RendererAPI::API::OpenGL:
			{
				CubeShader = Shader::Create("assets/shaders/Cube.glsl");
				break;
			}
			case RendererAPI::API::DirectX:
			{
				CubeShader = Shader::Create("assets/shaders/Cube.kdShader");
				break;
			}
			}

			//Cubes
			switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::OpenGL:
			{
				CubeVertexArray = VertexArray::Create();
				break;
			}
			case RendererAPI::API::DirectX:
			{

				CubeVertexArray = VertexArray::Create(CubeShader);
				break;
			}
			}
			CubeVertexBuffer = VertexBuffer::Create(0);
			CubeVertexBuffer->SetLayout({
				{ShaderDataType::Float3,"a_Position"},
				{ShaderDataType::Float3,"a_Normal"},
				{ShaderDataType::Float2,"a_TexCoords"},
				{ShaderDataType::Int,"a_MaterialIndex"},
				{ShaderDataType::Int,"a_EntityID"}
				});
			CubeVertexArray->AddVertexBuffer(CubeVertexBuffer);

			uint32_t* cubeIndices = SetupCubeIndices();
			Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, MaxIndices);
			CubeVertexArray->SetIndexBuffer(cubeIB);
			delete[] cubeIndices;

			//misc.
			WhiteTexture = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
			auto slot = TextureSlots.Reserve(1);
			auto& ptr = slot[0];
			ptr = WhiteTexture;
			//Materials

			MaterialTextures = Texture2DArray::Create(256, 256);
			uint32_t init = 0xffffffff;
			//Defualt  Diffuse
			MaterialTextures->PushTexture(&init, 1, 1);
			//Default Specular
			MaterialTextures->PushTexture(&init, 1, 1);

			//Temp
			{
				MaterialTextures->PushTexture("assets/textures/container2.png");
				MaterialTextures->PushTexture("assets/textures/container2_specular.png");

			}
		}


	}* s_GeometryFlusherData = nullptr;


	void GeometryFlusher::PushCubeVertex(CubeVertex* begin, CubeVertex* end) {
		auto bvi = s_GeometryFlusherData->CubeVertexBufferArray.Reserve(end - begin);
		std::memcpy(&bvi[0], begin, (end - begin) * sizeof(CubeVertex));
		s_GeometryFlusherData->CubeIndexCount += 36;
	}	

	GeometryFlusher::GeometryFlusher(const glm::mat4& viewProjectionMatrix,const glm::vec3& viewPosition) {
		if (s_GeometryFlusherData == nullptr) {
			s_GeometryFlusherData = new GeometryFlusherData;
		}
		s_GeometryFlusherData->CameraBuffer.ViewProjection = viewProjectionMatrix;
		s_GeometryFlusherData->CameraBuffer.CameraPosition = viewPosition;
	}
	GeometryFlusher::~GeometryFlusher() {

	}


	void GeometryFlusher::Begin() {
		
		s_GeometryFlusherData->CameraUniformBuffer->SetData(&s_GeometryFlusherData->CameraBuffer, sizeof(GeometryFlusherData::CameraData));
		StartBatch();
	}
	void GeometryFlusher::End() {
		Flush();
	}
	void GeometryFlusher::Flush() {
		FlushCubes(s_GeometryFlusherData->CubeVertexBufferArray.Get(), s_GeometryFlusherData->CubeVertexBufferArray.Size());
	}

	void GeometryFlusher::FlushCubes(CubeVertex* start, uint64_t size) {
		if (s_GeometryFlusherData->CubeIndexCount) {
			s_GeometryFlusherData->CubeVertexBuffer->SetData(start, (size) * sizeof(CubeVertex));
			s_GeometryFlusherData->CubeVertexBuffer->Bind();
			s_GeometryFlusherData->CubeVertexArray->GetIndexBuffer()->Bind();
			s_GeometryFlusherData->CameraUniformBuffer->Bind();
			s_GeometryFlusherData->CubeVertexArray->Bind();
			s_GeometryFlusherData->CubeShader->Bind();
			s_GeometryFlusherData->MaterialTextures->Bind(0);
			s_GeometryFlusherData->CubeShader->SetInt("u_MaterialTextures", 0);
			//Bind textures
			for (uint32_t i = 0; i < s_GeometryFlusherData->TextureSlotIndex; ++i) {
				s_GeometryFlusherData->TextureSlots.Get()[i]->Bind(i);
			}
			RenderCommand::DrawIndexed(s_GeometryFlusherData->CubeVertexArray, s_GeometryFlusherData->CubeIndexCount);
			s_GeometryFlusherData->CubeShader->Unbind();
			s_GeometryFlusherData->CubeVertexArray->Unbind();
			s_GeometryFlusherData->CameraUniformBuffer->UnBind();
			s_GeometryFlusherData->CubeVertexArray->GetIndexBuffer()->Unbind();
			s_GeometryFlusherData->CubeVertexBuffer->Unbind();
		}
	}
	void GeometryFlusher::StartBatch() {
		StartCubeBatch();
	}
	void GeometryFlusher::StartCubeBatch() {
		s_GeometryFlusherData->CubeIndexCount = 0;
		s_GeometryFlusherData->CubeVertexBufferArray.Reset();
		s_GeometryFlusherData->TextureSlotIndex = 1;
	}

}
