#pragma once
#include <glm/glm.hpp>
#include "Kaidel/Renderer/VertexArray.h"
namespace Kaidel {
	enum class CullMode {
		None,Front,Back,FrontAndBack
	};
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1,DirectX=2
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void SetLineWidth(float thickness)=0;
		virtual void SetCullMode(CullMode cullMode) = 0;
		virtual int QueryMaxTextureSlots() = 0;

		static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	private:
		static API s_API;
	};
}
