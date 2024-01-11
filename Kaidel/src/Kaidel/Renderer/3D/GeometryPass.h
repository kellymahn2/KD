#pragma once
#include "Kaidel/Renderer/Framebuffer.h"
#include "Kaidel/Renderer/VertexArray.h"
#include <glm/glm.hpp>
namespace Kaidel {


	enum class GeometryPassRenderType {
		None =0,
		Mesh = 1,
		Cube = Mesh,
		Sphere = 2
	};

	struct GeometryPassInput {
		uint32_t Width, Height;
		GeometryPassRenderType RenderType;
		Ref<VertexArray> VAO;
		uint32_t IndexCount;
	};

	struct GeometryPassOutput {
		Ref<Framebuffer> G_Buffers;
	};

	class GeometryPass {
	public:
		GeometryPass(const GeometryPassInput& input);
		GeometryPassOutput Apply();
	private:
		GeometryPassInput m_Input;
	};

}
