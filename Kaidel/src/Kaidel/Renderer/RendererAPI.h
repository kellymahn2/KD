#pragma once
#include <glm/glm.hpp>
#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
#include "Settings.h"
namespace Kaidel {

	
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1,DirectX=2,Vulkan = 3
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
		virtual void Shutdown() = 0;


		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		virtual void SetLineWidth(float thickness)=0;
		virtual void SetPointSize(float pixelSize) = 0;
		virtual void SetCullMode(CullMode cullMode) = 0;
		virtual int QueryMaxTextureSlots() = 0;
		virtual float QueryMaxTessellationLevel() = 0;
		virtual void SetPatchVertexCount(uint32_t count) = 0;

		virtual void SetDefaultTessellationLevels(const glm::vec4 & outer, const glm::vec2 & inner) = 0;

		virtual void RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height)const = 0;


		static RendererSettings& GetSettings() { return s_RendererSettings; }

		static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	protected:
		static inline RendererSettings s_RendererSettings;
	private:
		static API s_API;
	};
}
