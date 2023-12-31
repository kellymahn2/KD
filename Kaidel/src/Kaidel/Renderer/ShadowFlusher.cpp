#include "KDpch.h"
#include "ShadowFlusher.h"
#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Shader.h"
#include "Kaidel/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include "UniformBuffer.h"
#include "Texture.h"
#include "Material.h"
namespace Kaidel {



	static struct ShadowFlusherData {
		static constexpr uint32_t MaxCubes = 1000;
		static constexpr uint32_t MaxVertices = MaxCubes * 24;
		static constexpr uint32_t MaxIndices = MaxCubes * 36;
		static constexpr uint32_t MaxTextureSlots = 32;


		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<Shader> CubeShader;
		uint32_t CubeIndexCount = 0;
		BoundedVector <CubeVertex> CubeVertexBufferArray = { 10,MaxVertices ,[](auto start,uint64_t size) {
			ShadowFlusher::FlushCubes(start,size);
			ShadowFlusher::StartCubeBatch();
			} };

		struct LightData
		{
			glm::mat4 ViewProjection{};
			glm::vec3 CameraPosition{};
		};


		LightData LightBuffer{};
		Ref<UniformBuffer> LightUniformBuffer;

		ShadowFlusher* Flusher = nullptr;
		
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

		ShadowFlusherData() {

			/*TextureSlots = { 0,(uint64_t)RenderCommand::QueryMaxTextureSlots(),[&](auto start,uint64_t size) {
				TextureSlotIndex = (uint32_t)size;
				Flusher->Flush();
				} };*/

			LightUniformBuffer = UniformBuffer::Create(80, 1);

			switch (RendererAPI::GetAPI())
			{
			case RendererAPI::API::OpenGL:
			{
				CubeShader = Shader::Create("assets/shaders/ShadowPass.glsl");
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
			/*WhiteTexture = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
			auto slot = TextureSlots.Reserve(1);
			auto& ptr = slot[0];
			ptr = WhiteTexture;*/
			//Materials

			
		}


	}* s_ShadowFlusherData;









	void ShadowFlusher::PushCubeVertex(CubeVertex* begin, CubeVertex* end) {
		auto bvi = s_ShadowFlusherData->CubeVertexBufferArray.Reserve(end - begin);
		std::memcpy(&bvi[0], begin, (end - begin) * sizeof(CubeVertex));
		s_ShadowFlusherData->CubeIndexCount += 36;
	}


	ShadowFlusher::ShadowFlusher(const glm::mat4& viewProjectionMatrix, const glm::vec3& viewPosition) {
		if (s_ShadowFlusherData == nullptr) {
			s_ShadowFlusherData = new ShadowFlusherData;
		}
		s_ShadowFlusherData->LightBuffer.ViewProjection = viewProjectionMatrix;
		s_ShadowFlusherData->LightBuffer.CameraPosition = viewPosition;
	}
	ShadowFlusher::~ShadowFlusher() {

	}
	void ShadowFlusher::Begin() {
		s_ShadowFlusherData->LightUniformBuffer->SetData(&s_ShadowFlusherData->LightBuffer, sizeof(ShadowFlusherData::LightData));
		StartBatch();
	}
	void ShadowFlusher::End() {
		Flush();
	}
	void ShadowFlusher::Flush() {
		FlushCubes(s_ShadowFlusherData->CubeVertexBufferArray.Get(), s_ShadowFlusherData->CubeVertexBufferArray.Size());
	}

	void ShadowFlusher::FlushCubes(CubeVertex* start, uint64_t size) {
		if (s_ShadowFlusherData->CubeIndexCount) {
			s_ShadowFlusherData->CubeVertexBuffer->SetData(start, (size) * sizeof(CubeVertex));
			s_ShadowFlusherData->CubeVertexBuffer->Bind();
			s_ShadowFlusherData->CubeVertexArray->GetIndexBuffer()->Bind();
			s_ShadowFlusherData->LightUniformBuffer->Bind();
			s_ShadowFlusherData->CubeVertexArray->Bind();
			s_ShadowFlusherData->CubeShader->Bind();
			
			RenderCommand::DrawIndexed(s_ShadowFlusherData->CubeVertexArray, s_ShadowFlusherData->CubeIndexCount);

			s_ShadowFlusherData->CubeShader->Unbind();
			s_ShadowFlusherData->CubeVertexArray->Unbind();
			s_ShadowFlusherData->LightUniformBuffer->UnBind();
			s_ShadowFlusherData->CubeVertexArray->GetIndexBuffer()->Unbind();
			s_ShadowFlusherData->CubeVertexBuffer->Unbind();
		}
	}
	void ShadowFlusher::StartBatch() {
		StartCubeBatch();
	}
	void ShadowFlusher::StartCubeBatch() {
		s_ShadowFlusherData->CubeIndexCount = 0;
		s_ShadowFlusherData->CubeVertexBufferArray.Reset();
	}
}
