#include "KDpch.h"
#include "Kaidel/Renderer/Renderer.h"
#include "Kaidel/Renderer/2D/Renderer2D.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "Kaidel/Scene/ModelLibrary.h"
#include "Kaidel/Renderer/RendererGlobals.h"
#include "Kaidel/Renderer/Renderer3D.h"

namespace Kaidel {

	Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

	void Renderer::Init()
	{
		RenderCommand::Init();
		RendererGlobals::Init();

		ShaderLibrary::Init("assets/_shaders/cache", ".shader_cache");
		TextureLibrary::Init();
		Renderer3D::Init();
		ModelLibrary::Init();
	}

	void Renderer::Shutdown()
	{
		//Renderer2D::Shutdown();
		Renderer3D::Shutdown();
		ModelLibrary::Shutdown();
		TextureLibrary::Shutdown();
		ShaderLibrary::Shutdown();
		RendererGlobals::Shutdown();

		//TextureLibrary::Shutdown();
		/*
		Renderer3D::Shutdown();*/
		//delete Kaidel::GlobalRendererData;
	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{
		//RenderCommand::SetViewport(0, 0, width, height);
	}

	
}
