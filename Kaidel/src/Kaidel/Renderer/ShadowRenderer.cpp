#include "KDpch.h"
#include "ShadowRenderer.h"
#include "Kaidel/Core/BoundedVector.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "RendererAPI.h"
#include "RenderCommand.h"
namespace Kaidel {
	struct CubeShadowVertex {
		glm::vec3 Position;
	};
	static struct ShadowRendererData {
		static constexpr uint32_t MaxCubes = 10000;
		static constexpr uint32_t MaxVertices = MaxCubes * 8;
		static constexpr uint32_t MaxIndices = MaxCubes * 36;
		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		BoundedVector<CubeShadowVertex> CubeShadowVertexArray = { 10,MaxCubes,[](auto start,uint64_t size) {
			ShadowRenderer::FlushCubes(start,size);
			ShadowRenderer::StartCubeBatch();
			} };
		struct {
			glm::vec4 Position;
		} CubeVertexData[8];

		Ref<Shader> CubeSpotLightShadowShader;
		uint32_t CubeIndexCount;
		std::mutex CubeMutex;

		int32_t LightIndex;

		uint32_t* SetupCubeIndices() {
			uint32_t* cubeIndices = new uint32_t[MaxIndices];

			uint32_t offset = 0;

			//// Front face
			//CubeVertexData[0] = { {-0.5f , -0.5f ,  0.5f , 1.0f} }; // Bottom-left
			//CubeVertexData[1] = { { 0.5f , -0.5f ,  0.5f , 1.0f} }; // Bottom-right
			//CubeVertexData[2] = { { 0.5f ,  0.5f ,  0.5f , 1.0f} }; // Top-right
			//CubeVertexData[3] = { {-0.5f ,  0.5f ,  0.5f , 1.0f} }; // Top-left

			//// Back face										   		    						   
			//CubeVertexData[4] = { {-0.5f , -0.5f , -0.5f , 1.0f} }; // Bottom-left
			//CubeVertexData[5] = { { 0.5f , -0.5f , -0.5f , 1.0f} }; // Bottom-right
			//CubeVertexData[6] = { { 0.5f ,  0.5f , -0.5f , 1.0f} }; // Top-right
			//CubeVertexData[7] = { {-0.5f ,  0.5f , -0.5f , 1.0f} }; // Top-left

			for (uint32_t i = 0; i < MaxIndices; i += 36)
			{
				//Front
				cubeIndices[i + 0] = offset + 0;
				cubeIndices[i + 1] = offset + 1;	
				cubeIndices[i + 2] = offset + 2;
				cubeIndices[i + 3] = offset + 2;
				cubeIndices[i + 4] = offset + 3;
				cubeIndices[i + 5] = offset + 0;

				//Back
				cubeIndices[i + 6] = offset +  4;
				cubeIndices[i + 7] = offset +  7;
				cubeIndices[i + 8] = offset +  6;
				cubeIndices[i + 9] = offset +  6;
				cubeIndices[i + 10] = offset + 5;
				cubeIndices[i + 11] = offset + 4;


				//Left
				cubeIndices[i + 12] = offset + 7;
				cubeIndices[i + 13] = offset + 4;
				cubeIndices[i + 14] = offset + 0;
				cubeIndices[i + 15] = offset + 0;
				cubeIndices[i + 16] = offset + 3;
				cubeIndices[i + 17] = offset + 7;


				//Right
				cubeIndices[i + 18] = offset + 5;
				cubeIndices[i + 19] = offset + 6;
				cubeIndices[i + 20] = offset + 2;
				cubeIndices[i + 21] = offset + 2;
				cubeIndices[i + 22] = offset + 1;
				cubeIndices[i + 23] = offset + 5;

				//Top
				cubeIndices[i + 24] = offset +  6;
				cubeIndices[i + 25] = offset +  7;
				cubeIndices[i + 26] = offset +  3;
				cubeIndices[i + 27] = offset +  3;
				cubeIndices[i + 28] = offset +  2;
				cubeIndices[i + 29] = offset +  6;

				//Bottom
				cubeIndices[i + 30] = offset + 4;
				cubeIndices[i + 31] = offset + 5;
				cubeIndices[i + 32] = offset + 1;
				cubeIndices[i + 33] = offset + 1;
				cubeIndices[i + 34] = offset + 0;
				cubeIndices[i + 35] = offset + 4;

				offset += 8;
			}
			return cubeIndices;
		}

		ShadowRendererData() {

			// Front face
			CubeVertexData[0] = { {-0.5f , -0.5f ,  0.5f , 1.0f}}; // Bottom-left
			CubeVertexData[1] = { { 0.5f , -0.5f ,  0.5f , 1.0f}}; // Bottom-right
			CubeVertexData[2] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}}; // Top-right
			CubeVertexData[3] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}}; // Top-left

			// Back face										   		    						   
			CubeVertexData[4] = { {-0.5f , -0.5f , -0.5f , 1.0f}}; // Bottom-left
			CubeVertexData[5] = { { 0.5f , -0.5f , -0.5f , 1.0f}}; // Bottom-right
			CubeVertexData[6] = { { 0.5f ,  0.5f , -0.5f , 1.0f}}; // Top-right
			CubeVertexData[7] = { {-0.5f ,  0.5f , -0.5f , 1.0f}}; // Top-left

			switch (RendererAPI::GetAPI())
			{
			case RendererAPI::API::OpenGL:
			{
				CubeSpotLightShadowShader = Shader::Create(std::filesystem::path{ "assets/shaders/ShadowPass/SpotLightVS.glsl" }, std::filesystem::path{ "assets/shaders/ShadowPass/FS.glsl" });
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

				CubeVertexArray = VertexArray::Create(CubeSpotLightShadowShader);
				break;
			}
			}
			CubeVertexBuffer = VertexBuffer::Create(0);
			CubeVertexBuffer->SetLayout({
				{ShaderDataType::Float3,"a_Position"},
				});
			CubeVertexArray->AddVertexBuffer(CubeVertexBuffer);
			uint32_t* cubeIndices = SetupCubeIndices();
			Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, MaxIndices);
			CubeVertexArray->SetIndexBuffer(cubeIB);
			delete[] cubeIndices;
		}

	}* s_RendererData;


	ShadowRenderer::ShadowRenderer() {
		if (!s_RendererData)
			s_RendererData = new ShadowRendererData;
	}
	void ShadowRenderer::BeginRendering(int lightIndex) {
		StartBatch();
		s_RendererData->LightIndex = lightIndex;
	}

	void ShadowRenderer::DrawCube(const glm::mat4& transform) {
		CubeShadowVertex vertices[8];
		for (int i = 0; i < 8; ++i) {
			vertices[i].Position = transform * s_RendererData->CubeVertexData[i].Position;
		}
		std::unique_lock<std::mutex> lock(s_RendererData->CubeMutex);
		auto bvi = s_RendererData->CubeShadowVertexArray.Reserve(8);
		std::memcpy(&bvi[0], vertices, sizeof(CubeShadowVertex) * 8);
		s_RendererData->CubeIndexCount += 36;

	}
	void ShadowRenderer::EndRendering() {
		Flush();
	}
	void ShadowRenderer::StartBatch() {
		StartCubeBatch();
	}
	void ShadowRenderer::StartCubeBatch() {
		s_RendererData->CubeIndexCount = 0;
		s_RendererData->CubeShadowVertexArray.Reset();
	}
	void ShadowRenderer::FlushCubes(CubeShadowVertex* start, uint64_t size) {
		if (s_RendererData->CubeIndexCount) {
			s_RendererData->CubeVertexBuffer->SetData(start, (size) * sizeof(CubeShadowVertex));
			s_RendererData->CubeVertexArray->Bind();
			s_RendererData->CubeSpotLightShadowShader->Bind();
			s_RendererData->CubeSpotLightShadowShader->SetInt("u_LightIndex",s_RendererData->LightIndex);
			RenderCommand::DrawIndexed(s_RendererData->CubeVertexArray, s_RendererData->CubeIndexCount);
		}
	}
	void ShadowRenderer::Flush() {
		FlushCubes(s_RendererData->CubeShadowVertexArray.Get(), s_RendererData->CubeShadowVertexArray.Size());
	}
}
