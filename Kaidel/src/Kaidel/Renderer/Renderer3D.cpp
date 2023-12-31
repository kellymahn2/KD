#include "KDpch.h"
#include "Renderer3D.h"

#include "Kaidel/Renderer/VertexArray.h"
#include "Kaidel/Renderer/Shader.h"
#include "Kaidel/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include "UniformBuffer.h"
#include "Kaidel/Core/Timer.h"

#include "Kaidel\Core\ThreadExecutor.h"
#include "Kaidel/Core/BoundedVector.h"


namespace Kaidel {
	

	struct Renderer3DData {
		

		Renderer3DFlusher* Flusher;
		Ref<Material> DefaultMaterial;
		struct {
			glm::vec4 CubeVertexPositions;
			glm::vec4 CubeVertexNormals;
			glm::vec2 CubeTexCoords;
		} CubeVertexData[24];
		std::mutex CubeMutex;
	};
	Renderer3DData s_Data;

	/*static uint32_t* SetupCubeIndices() {
		uint32_t* cubeIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i += 36)
		{
			cubeIndices[i + 0] = offset  + 0;
			cubeIndices[i + 1] = offset  + 1;
			cubeIndices[i + 2] = offset  + 2;
										 
			cubeIndices[i + 3] = offset  + 2;
			cubeIndices[i + 4] = offset  + 3;
			cubeIndices[i + 5] = offset  + 0;
										 
			cubeIndices[i + 6] = offset  + 4;
			cubeIndices[i + 7] = offset  + 5;
			cubeIndices[i + 8] = offset  + 6;
										 
			cubeIndices[i + 9] = offset  + 6;
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
	}*/



	void Renderer3D::Init() {

		s_Data.DefaultMaterial = CreateRef<Material>();
		// Front face
		s_Data.CubeVertexData[0] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,0} }; // Bottom-left
		s_Data.CubeVertexData[1] = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,0} }; // Bottom-right
		s_Data.CubeVertexData[2] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,1} }; // Top-right
		s_Data.CubeVertexData[3] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,1} }; // Top-left

		// Back face										   		    						   
		s_Data.CubeVertexData[4] = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,0} }; // Bottom-left
		s_Data.CubeVertexData[5] = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,0} }; // Bottom-right
		s_Data.CubeVertexData[7] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,1} }; // Top-left
		s_Data.CubeVertexData[6] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,1} }; // Top-right

		// Right face										   		    						   
		s_Data.CubeVertexData[8]  = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,0} }; // Bottom-front
		s_Data.CubeVertexData[9]  = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,0} }; // Bottom-back
		s_Data.CubeVertexData[10] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,1} }; // Top-back
		s_Data.CubeVertexData[11] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,1} }; // Top-front

		// Left face										   									   
		s_Data.CubeVertexData[12] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,0} }; // Bottom-front
		s_Data.CubeVertexData[13] = { {-0.5f , -0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,0} }; // Bottom-back
		s_Data.CubeVertexData[14] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,1} }; // Top-back
		s_Data.CubeVertexData[15] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,1} }; // Top-front

		// Bottom face										   									   
		s_Data.CubeVertexData[16] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,0} }; // Front-left
		s_Data.CubeVertexData[17] = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,0} }; // Front-right
		s_Data.CubeVertexData[18] = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,1} }; // Back-right
		s_Data.CubeVertexData[19] = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,1} }; // Back-left

		// Top face											   		    					   	   
		s_Data.CubeVertexData[20] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,0} }; // Front-left
		s_Data.CubeVertexData[21] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,0} }; // Front-right
		s_Data.CubeVertexData[22] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,1} }; // Back-right
		s_Data.CubeVertexData[23] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,1} }; // Back-left
		//s_Data.TextureSlots = { 0,(uint64_t)RenderCommand::QueryMaxTextureSlots(),[&](auto start,uint64_t size) {
		//	s_Data.TextureSlotIndex = (uint32_t)size;
		//	Flush();
		//	}};

		//s_Data.DefaultMaterial = CreateRef<Material>();
		//s_Data.CameraUniformBuffer = UniformBuffer::Create(80, 0);

		//switch (RendererAPI::GetAPI())
		//{
		//case RendererAPI::API::OpenGL:
		//{
		//	s_Data.CubeShader = Shader::Create("assets/shaders/Cube.glsl");
		//	break;
		//}
		//case RendererAPI::API::DirectX:
		//{
		//	s_Data.CubeShader = Shader::Create("assets/shaders/Cube.kdShader");
		//	break;
		//}
		//}

		////Cubes
		//switch (RendererAPI::GetAPI()) {
		//case RendererAPI::API::OpenGL:
		//{
		//	s_Data.CubeVertexArray = VertexArray::Create();
		//	break;
		//}
		//case RendererAPI::API::DirectX:
		//{

		//	s_Data.CubeVertexArray = VertexArray::Create(s_Data.CubeShader);
		//	break;
		//}
		//}
		//s_Data.CubeVertexBuffer = VertexBuffer::Create(0);
		//s_Data.CubeVertexBuffer->SetLayout({
		//	{ShaderDataType::Float3,"a_Position"},
		//	{ShaderDataType::Float3,"a_Normal"},
		//	{ShaderDataType::Float2,"a_TexCoords"},
		//	{ShaderDataType::Int,"a_MaterialIndex"},
		//	{ShaderDataType::Int,"a_EntityID"}
		//	});
		//s_Data.CubeVertexArray->AddVertexBuffer(s_Data.CubeVertexBuffer);

		//uint32_t* cubeIndices = SetupCubeIndices();
		//Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, s_Data.MaxIndices);
		//s_Data.CubeVertexArray->SetIndexBuffer(cubeIB);
		//delete[] cubeIndices;

		////misc.
		//s_Data.WhiteTexture = Texture2D::Create(1, 1);
		//uint32_t whiteTextureData = 0xffffffff;
		//s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		//auto slot = s_Data.TextureSlots.Reserve(1);
		//auto& ptr = slot[0];
		//ptr = s_Data.WhiteTexture;
		//// Front face
		//s_Data.CubeVertexData[0]  = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,0}}; // Bottom-left
		//s_Data.CubeVertexData[1]  = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,0}}; // Bottom-right
		//s_Data.CubeVertexData[2]  = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,1}}; // Top-right
		//s_Data.CubeVertexData[3]  = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,1}}; // Top-left
		//													   		    						   
		//// Back face										   		    						   
		//s_Data.CubeVertexData[4]  = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,0}}; // Bottom-left
		//s_Data.CubeVertexData[5]  = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,0}}; // Bottom-right
		//s_Data.CubeVertexData[7]  = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,1}}; // Top-left
		//s_Data.CubeVertexData[6]  = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,1}}; // Top-right
		//													   		    						   
		//// Right face										   		    						   
		//s_Data.CubeVertexData[8]  = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,0}}; // Bottom-front
		//s_Data.CubeVertexData[9]  = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,0}}; // Bottom-back
		//s_Data.CubeVertexData[10] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,1}}; // Top-back
		//s_Data.CubeVertexData[11] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,1}}; // Top-front
		//													   									   
		//// Left face										   									   
		//s_Data.CubeVertexData[12] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,0}}; // Bottom-front
		//s_Data.CubeVertexData[13] = { {-0.5f , -0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,0}}; // Bottom-back
		//s_Data.CubeVertexData[14] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,1}}; // Top-back
		//s_Data.CubeVertexData[15] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,1}}; // Top-front
		//													   									   
		//// Bottom face										   									   
		//s_Data.CubeVertexData[16] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,0}}; // Front-left
		//s_Data.CubeVertexData[17] = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,0}}; // Front-right
		//s_Data.CubeVertexData[18] = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,1}}; // Back-right
		//s_Data.CubeVertexData[19] = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,1}}; // Back-left
		//													   		    					   	   
		//// Top face											   		    					   	   
		//s_Data.CubeVertexData[20] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,0}}; // Front-left
		//s_Data.CubeVertexData[21] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,0}}; // Front-right
		//s_Data.CubeVertexData[22] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,1}}; // Back-right
		//s_Data.CubeVertexData[23] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,1}}; // Back-left
		//
		//
		////Materials

		//s_Data.MaterialTextures = Texture2DArray::Create(256, 256);
		//uint32_t init = 0xffffffff;
		////Defualt  Diffuse
		//s_Data.MaterialTextures->PushTexture(&init, 1, 1);
		////Default Specular
		//s_Data.MaterialTextures->PushTexture(&init, 1, 1);
		//
		////Temp
		//{
		//	s_Data.MaterialTextures->PushTexture("assets/textures/container2.png");
		//	s_Data.MaterialTextures->PushTexture("assets/textures/container2_specular.png");

		//}


		//

	}

	void Renderer3D::Shutdown() {

	}

	void Renderer3D::BeginRendering(Renderer3DFlusher* flusher)
	{
		KD_PROFILE_FUNCTION();


		KD_CORE_ASSERT(flusher, "Cannot begin rendering without flusher");
		
		s_Data.Flusher = flusher;
		s_Data.Flusher->Begin();
		StartBatch();
	}

	void Renderer3D::EndRendering() {
		s_Data.Flusher->End();
	}

	
	void Renderer3D::StartCubeBatch() {
	}

	void Renderer3D::StartBatch()
	{
	}

	void Renderer3D::Flush() {
		s_Data.Flusher->Flush();
	}

	void Renderer3D::FlushCubes(CubeVertex* start,uint64_t size) {

		
	}

	void Renderer3D::NextBatch()
	{
		
	}


	static inline uint32_t _GetTexCoordIndex(const glm::vec4& pos) {

	}

	static void SetCubeVertexValues(CubeVertex& cv, const glm::mat4& transform,const glm::mat4& normalTransform,const glm::vec4& position,const glm::vec4& normals, const glm::vec2& texCoords,int materialIndex,int entityID) {
		cv.Position = transform * position;
		cv.Normal = normalTransform * normals;
		cv.TexCoords = texCoords;
		cv.MaterialIndex = materialIndex;
		cv.EntityID = entityID;
	}
	void Renderer3D::DrawCube(const glm::mat4& transform, Ref<Material> material,int entityID) {

		if (!material) {

			DrawCube(transform, s_Data.DefaultMaterial, entityID);
			return;
		}
		CubeVertex  data[24];
		glm::mat4 normalTransform = glm::transpose(glm::inverse(glm::mat3(transform)));
		for (std::size_t i = 0; i < 24;++i) {
			SetCubeVertexValues(data[i], transform, normalTransform,s_Data.CubeVertexData[i].CubeVertexPositions, s_Data.CubeVertexData[i].CubeVertexNormals,s_Data.CubeVertexData[i].CubeTexCoords, material->GetIndex(), entityID);
		}
		

		std::scoped_lock<std::mutex> lock(s_Data.CubeMutex);
		s_Data.Flusher->PushCubeVertex(data, data + 24);
	}

}
