#pragma once

#include "Kaidel/Renderer/RenderCommand.h"

#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

namespace Kaidel {


	using RendererSubmitFunction = std::function<void()>;

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();
		


		static void OnWindowResize(uint32_t width, uint32_t height);

		

		
		static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static Scope<SceneData> s_SceneData;

	};
}
