#include "KDpch.h"
#include "LightingRenderer.h"
#include "Kaidel/Core/BoundedVector.h"
#include "Light.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "RendererAPI.h"
#include "RenderCommand.h"
namespace Kaidel {

	struct CubeLightingVertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		int MaterialIndex;
	};

	struct LightingRendererData {
		static constexpr uint32_t MaxCubes = 10000;
		static constexpr uint32_t MaxVertices = MaxCubes * 24;
		static constexpr uint32_t MaxIndices = MaxCubes * 36;
		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		BoundedVector<CubeLightingVertex> CubeLightingVertexArray = { 10,MaxCubes,[](auto start,uint64_t size) {
			LightingRenderer::FlushCubes(start,size);
			LightingRenderer::StartCubeBatch();
			} };
		struct {
			glm::vec4 Position;
			glm::vec4 Normal;
			glm::vec2 TexCoord;
		} CubeVertexData[24];
		
		Ref<Shader> CubeLightingShader;
		uint32_t CubeIndexCount;
		std::mutex CubeMutex;



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

		LightingRendererData() {

			// Front face
			CubeVertexData[ 0] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,0}}; // Bottom-left
			CubeVertexData[ 1] = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,0}}; // Bottom-right
			CubeVertexData[ 2] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,1}}; // Top-right
			CubeVertexData[ 3] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,1}}; // Top-left

			// Back face										   		    						   
			CubeVertexData[ 4] = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,0}}; // Bottom-left
			CubeVertexData[ 5] = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,0}}; // Bottom-right
			CubeVertexData[ 6] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,1}}; // Top-right
			CubeVertexData[7] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,1} }; // Top-left

			// Right face										   		    						   
			CubeVertexData[ 8] = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,0}}; // Bottom-front
			CubeVertexData[ 9] = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,0}}; // Bottom-back
			CubeVertexData[10] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,1}}; // Top-back
			CubeVertexData[11] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,1}}; // Top-front

			// Left face										   									   
			CubeVertexData[12] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,0}}; // Bottom-front
			CubeVertexData[13] = { {-0.5f , -0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,0}}; // Bottom-back
			CubeVertexData[14] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,1}}; // Top-back
			CubeVertexData[15] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,1}}; // Top-front

			// Bottom face										   									   
			CubeVertexData[16] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,0}}; // Front-left
			CubeVertexData[17] = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,0}}; // Front-right
			CubeVertexData[18] = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,1}}; // Back-right
			CubeVertexData[19] = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,1}}; // Back-left

			// Top face											   		    					   	   
			CubeVertexData[20] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,0}}; // Front-left
			CubeVertexData[21] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,0}}; // Front-right
			CubeVertexData[22] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,1}}; // Back-right
			CubeVertexData[23] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,1}}; // Back-left


			switch (RendererAPI::GetAPI())
			{
			case RendererAPI::API::OpenGL:
			{
				CubeLightingShader = Shader::Create(std::filesystem::path{ "assets/shaders/LightingPass/VS.glsl" }, std::filesystem::path{ "assets/shaders/LightingPass/FS.glsl" });
				break;
			}
			case RendererAPI::API::DirectX:
			{
				break;
			}
			}
			switch (RendererAPI::GetAPI()) {
			case RendererAPI::API::OpenGL:
			{
				CubeVertexArray = VertexArray::Create();
				break;
			}
			case RendererAPI::API::DirectX:
			{

				CubeVertexArray = VertexArray::Create(CubeLightingShader);
				break;
			}
			}

			CubeVertexBuffer = VertexBuffer::Create(0);
			CubeVertexBuffer->SetLayout({
				{ShaderDataType::Float3,"a_Position"},
				{ShaderDataType::Float3,"a_Normal"},
				{ShaderDataType::Float2,"a_TexCoord"},
				{ShaderDataType::Int,"a_MaterialIndex"}
				});
			CubeVertexArray->AddVertexBuffer(CubeVertexBuffer);
			uint32_t* cubeIndices = SetupCubeIndices();
			Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, MaxIndices);
			CubeVertexArray->SetIndexBuffer(cubeIB);
			delete[] cubeIndices;
		}
	};

	static LightingRendererData* s_RendererData;


	LightingRenderer::LightingRenderer() {
		if (!s_RendererData)
			s_RendererData = new LightingRendererData;

	}
	void LightingRenderer::BeginRendering() {
		StartBatch();
	}
	void LightingRenderer::DrawCube(const glm::mat4& transform,uint32_t materialIndex) {
		CubeLightingVertex vertices[24];
		glm::mat4 normalTransform = glm::transpose(glm::inverse(glm::mat3(transform)));
		for (size_t i = 0; i < 24; ++i) {
			vertices[i].Position = transform * s_RendererData->CubeVertexData[i].Position;
			vertices[i].Normal = normalTransform * s_RendererData->CubeVertexData[i].Normal;
			vertices[i].TexCoord = s_RendererData->CubeVertexData[i].TexCoord;
			vertices[i].MaterialIndex = materialIndex;
		}

		std::unique_lock<std::mutex> lock(s_RendererData->CubeMutex);
		auto bvi = s_RendererData->CubeLightingVertexArray.Reserve(24);
		std::memcpy(&bvi[0], vertices, sizeof(CubeLightingVertex) * 24);
		s_RendererData->CubeIndexCount += 36;
	}
	void LightingRenderer::EndRendering() {
		Flush();
	}
	void LightingRenderer::Flush() {
		FlushCubes(s_RendererData->CubeLightingVertexArray.Get(),s_RendererData->CubeLightingVertexArray.Size());
	}
	
	void LightingRenderer::StartBatch(){
		StartCubeBatch();
	}
	void LightingRenderer::StartCubeBatch(){
		s_RendererData->CubeIndexCount = 0;
		s_RendererData->CubeLightingVertexArray.Reset();
	}
	void LightingRenderer::FlushCubes(CubeLightingVertex* start, uint64_t size) {
		if (s_RendererData->CubeIndexCount) {
			s_RendererData->CubeVertexBuffer->SetData(start, (size) * sizeof(CubeLightingVertex));
			s_RendererData->CubeVertexArray->Bind();
			s_RendererData->CubeLightingShader->Bind();
			s_RendererData->CubeLightingShader->SetInt("u_SpotLightDepthMaps",_SpotLightBindingSlot);

			RenderCommand::DrawIndexed(s_RendererData->CubeVertexArray, s_RendererData->CubeIndexCount);
		}
	}
}
