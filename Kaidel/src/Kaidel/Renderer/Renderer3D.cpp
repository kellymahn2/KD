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
	struct CubeVertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		int MaterialIndex;
		int EntityID;
	};

	
	struct Renderer3DData {
		static constexpr uint32_t MaxCubes = 1000;
		static constexpr uint32_t MaxVertices= MaxCubes*24;
		static constexpr uint32_t MaxIndices = MaxCubes*36;
		static constexpr uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> CubeVertexArray;
		Ref<VertexBuffer> CubeVertexBuffer;
		Ref<Shader> CubeShader;
		uint32_t CubeIndexCount = 0;
		BoundedVector < CubeVertex> CubeVertexBufferArray = { 10,MaxVertices ,[](auto start,uint64_t size) {
			Renderer3D::FlushCubes(start,size);
			Renderer3D::StartCubeBatch();
			} };
		std::mutex CubeMutex;
		size_t CubeVertexBufferIndex = 0;


		Ref<Texture2D> WhiteTexture;
		BoundedVector <Ref<Texture2D>> TextureSlots;

		uint32_t TextureSlotIndex = 1; // 0 = white texture

		Ref<Material> DefaultMaterial;

		struct {
			glm::vec4 CubeVertexPositions;
			glm::vec4 CubeVertexNormals;
			glm::vec2 CubeTexCoords;
		} CubeVertexData[24];



		
		Renderer3D::Statistics Stats;
		struct CameraData
		{
			glm::mat4 ViewProjection;
			glm::vec3 CameraPosition;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;

		Ref<Texture2DArray> MaterialTextures;


	};
	static Renderer3DData s_Data;

	static uint32_t* SetupCubeIndices() {
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
	}



	void Renderer3D::Init() {

		s_Data.TextureSlots = { 0,(uint64_t)RenderCommand::QueryMaxTextureSlots(),[&](auto start,uint64_t size) {
			s_Data.TextureSlotIndex = (uint32_t)size;
			Flush();
			}};

		s_Data.DefaultMaterial = CreateRef<Material>();
		s_Data.CameraUniformBuffer = UniformBuffer::Create(80, 0);

		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL:
		{
			s_Data.CubeShader = Shader::Create("assets/shaders/Cube.glsl");
			break;
		}
		case RendererAPI::API::DirectX:
		{
			s_Data.CubeShader = Shader::Create("assets/shaders/Cube.kdShader");
			break;
		}
		}

		//Cubes
		switch (RendererAPI::GetAPI()) {
		case RendererAPI::API::OpenGL:
		{
			s_Data.CubeVertexArray = VertexArray::Create();
			break;
		}
		case RendererAPI::API::DirectX:
		{

			s_Data.CubeVertexArray = VertexArray::Create(s_Data.CubeShader);
			break;
		}
		}
		s_Data.CubeVertexBuffer = VertexBuffer::Create(0);
		s_Data.CubeVertexBuffer->SetLayout({
			{ShaderDataType::Float3,"a_Position"},
			{ShaderDataType::Float3,"a_Normal"},
			{ShaderDataType::Float2,"a_TexCoords"},
			{ShaderDataType::Int,"a_MaterialIndex"},
			{ShaderDataType::Int,"a_EntityID"}
			});
		s_Data.CubeVertexArray->AddVertexBuffer(s_Data.CubeVertexBuffer);

		uint32_t* cubeIndices = SetupCubeIndices();
		Ref<IndexBuffer> cubeIB = IndexBuffer::Create(cubeIndices, s_Data.MaxIndices);
		s_Data.CubeVertexArray->SetIndexBuffer(cubeIB);
		delete[] cubeIndices;

		//misc.
		s_Data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		auto slot = s_Data.TextureSlots.Reserve(1);
		auto& ptr = slot[0];
		ptr = s_Data.WhiteTexture;
		// Front face
		s_Data.CubeVertexData[0]  = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,0}}; // Bottom-left
		s_Data.CubeVertexData[1]  = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,0}}; // Bottom-right
		s_Data.CubeVertexData[2]  = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {1,1}}; // Top-right
		s_Data.CubeVertexData[3]  = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  0.0f,  1.0f, 1.0f}, {0,1}}; // Top-left
															   		    						   
		// Back face										   		    						   
		s_Data.CubeVertexData[4]  = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,0}}; // Bottom-left
		s_Data.CubeVertexData[5]  = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,0}}; // Bottom-right
		s_Data.CubeVertexData[7]  = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {1,1}}; // Top-left
		s_Data.CubeVertexData[6]  = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  0.0f, -1.0f, 1.0f}, {0,1}}; // Top-right
															   		    						   
		// Right face										   		    						   
		s_Data.CubeVertexData[8]  = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,0}}; // Bottom-front
		s_Data.CubeVertexData[9]  = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,0}}; // Bottom-back
		s_Data.CubeVertexData[10] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {1,1}}; // Top-back
		s_Data.CubeVertexData[11] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 1.0f,  0.0f,  0.0f, 1.0f}, {0,1}}; // Top-front
															   									   
		// Left face										   									   
		s_Data.CubeVertexData[12] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,0}}; // Bottom-front
		s_Data.CubeVertexData[13] = { {-0.5f , -0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,0}}; // Bottom-back
		s_Data.CubeVertexData[14] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {1,1}}; // Top-back
		s_Data.CubeVertexData[15] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, {-1.0f,  0.0f,  0.0f, 1.0f}, {0,1}}; // Top-front
															   									   
		// Bottom face										   									   
		s_Data.CubeVertexData[16] = { {-0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,0}}; // Front-left
		s_Data.CubeVertexData[17] = { { 0.5f , -0.5f ,  0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,0}}; // Front-right
		s_Data.CubeVertexData[18] = { { 0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {1,1}}; // Back-right
		s_Data.CubeVertexData[19] = { {-0.5f , -0.5f , -0.5f , 1.0f}, { 0.0f, -1.0f,  0.0f, 1.0f}, {0,1}}; // Back-left
															   		    					   	   
		// Top face											   		    					   	   
		s_Data.CubeVertexData[20] = { {-0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,0}}; // Front-left
		s_Data.CubeVertexData[21] = { { 0.5f ,  0.5f ,  0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,0}}; // Front-right
		s_Data.CubeVertexData[22] = { { 0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {1,1}}; // Back-right
		s_Data.CubeVertexData[23] = { {-0.5f ,  0.5f , -0.5f , 1.0f}, { 0.0f,  1.0f,  0.0f, 1.0f}, {0,1}}; // Back-left
		
		
		//Materials

		s_Data.MaterialTextures = Texture2DArray::Create(256, 256);
		uint32_t init = 0xffffffff;
		//Defualt  Diffuse
		s_Data.MaterialTextures->PushTexture(&init, 1, 1);
		//Default Specular
		s_Data.MaterialTextures->PushTexture(&init, 1, 1);
		
		//Temp
		{
			s_Data.MaterialTextures->PushTexture("assets/textures/container2.png");
			s_Data.MaterialTextures->PushTexture("assets/textures/container2_specular.png");

		}


		

	}

	void Renderer3D::Shutdown() {

	}

	void Renderer3D::BeginScene(const OrthographicCamera& camera)
	{
		KD_PROFILE_FUNCTION();

		s_Data.CubeShader->Bind();
		s_Data.CubeShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		StartBatch();
	}
	
	void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		KD_PROFILE_FUNCTION();

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
		s_Data.CameraBuffer.ViewProjection = viewProj;
		s_Data.CameraBuffer.CameraPosition = transform*glm::vec4(0,0,0,0);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));

		StartBatch();
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		KD_PROFILE_FUNCTION();

		s_Data.CameraBuffer.ViewProjection = camera.GetProjection() * camera.GetViewMatrix();
		s_Data.CameraBuffer.CameraPosition = camera.GetPosition();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer3DData::CameraData));

		StartBatch();
	}

	void Renderer3D::EndScene() {

		Flush();
	}

	void Renderer3D::StartCubeBatch() {
		s_Data.CubeIndexCount = 0;
		s_Data.CubeVertexBufferArray.Reset();
		s_Data.TextureSlotIndex = 1;
	}

	void Renderer3D::StartBatch()
	{
		StartCubeBatch();
	}

	void Renderer3D::Flush() {
		std::scoped_lock<std::mutex> quadLock(s_Data.CubeMutex);
		FlushCubes(s_Data.CubeVertexBufferArray.Get(),s_Data.CubeVertexBufferArray.Size());
	}

	void Renderer3D::FlushCubes(CubeVertex* start,uint64_t size) {

		if (s_Data.CubeIndexCount) {
			s_Data.CubeVertexBuffer->SetData(start, (size) * sizeof(CubeVertex));
			s_Data.CubeVertexBuffer->Bind();
			s_Data.CubeVertexArray->GetIndexBuffer()->Bind();
			s_Data.CameraUniformBuffer->Bind();
			s_Data.CubeVertexArray->Bind();
			s_Data.CubeShader->Bind();
			s_Data.MaterialTextures->Bind(0);
			s_Data.CubeShader->SetInt("u_MaterialTextures", 0);
			//Bind textures
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; ++i) {
				s_Data.TextureSlots.Get()[i]->Bind(i);
			}
			RenderCommand::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
			s_Data.CubeShader->Unbind();
			s_Data.CubeVertexArray->Unbind();
			s_Data.CameraUniformBuffer->UnBind();
			s_Data.CubeVertexArray->GetIndexBuffer()->Unbind();
			s_Data.CubeVertexBuffer->Unbind();
			s_Data.Stats.DrawCalls++;
		}
	}

	void Renderer3D::NextBatch()
	{
		std::scoped_lock<std::mutex> lock(s_Data.CubeMutex);
		Flush();
		StartBatch();
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
		auto bvi = s_Data.CubeVertexBufferArray.Reserve(24);
		for (std::size_t i = 0; i < 24; ++i) {
			bvi[i] = std::move(data[i]);
		}


		s_Data.CubeIndexCount += 36;
		s_Data.Stats.CubeCount += 1;
	}

}
