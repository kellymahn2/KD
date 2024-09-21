#include "KDpch.h"
#include "Kaidel/Renderer/Renderer.h"
#include "Kaidel/Renderer/2D/Renderer2D.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"

namespace Kaidel {

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{

		RenderCommand::Init();

		ShaderLibrary::Init("assets/_shaders/cache", ".shader_cache");
		TextureLibrary::Init();
		//TextureLibrary::Init(2048, 2048, Format::RGBA8UN);
		//TextureLibrary::PushDefaultTextures();
		//Renderer2D::Init();
		////Renderer3D::Init();


		//{
		//	const std::vector<MeshVertex> vertices = {
		//		// Front face
		//		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		//		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
		//		{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		//		{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
		//		// Back face
		//		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
		//		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
		//		{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)},
		//		{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
		//		// Left face
		//		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		//		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		//		{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		//		{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		//		// Right face
		//		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		//		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		//		{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		//		{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		//		// Top face
		//		{glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		//		{glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		//		{glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		//		{glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		//		// Bottom face
		//		{glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		//		{glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
		//		{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		//		{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		//	};
		//	const std::vector<unsigned int> indices = {
		//		// Front face
		//		0, 1, 2,
		//		2, 3, 0,
		//		// Back face
		//		5, 4, 7,
		//		7, 6, 5,
		//		// Left face
		//		9, 8, 11,
		//		11, 10, 9,
		//		// Right face
		//		12, 13, 14,
		//		14, 15, 12,
		//		// Top face
		//		16, 17, 18,
		//		18, 19, 16,
		//		// Bottom face
		//		23, 22, 21,
		//		21, 20, 23,
		//	};
		//	Ref<Material> mat = {};
		//	//Primitives::CubePrimitive = SingleAssetManager<Mesh>::Manage(CreateRef<Mesh>("Cube", vertices, indices,glm::vec3(0,0,0)),UUID(CubeAssetID));
		//}

		////MaterialTexture::Init();
		//GlobalRenderer3DData* data = new GlobalRenderer3DData;
		//Kaidel::GlobalRendererData = data;
		
	}

	void Renderer::Shutdown()
	{
		//Renderer2D::Shutdown();
		TextureLibrary::Shutdown();
		ShaderLibrary::Shutdown();
		//TextureLibrary::Shutdown();
		/*
		Renderer3D::Shutdown();*/
		//delete Kaidel::GlobalRendererData;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		RenderCommand::SetViewport(0, 0, width, height);
	}

	
}
