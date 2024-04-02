#pragma once

#include "Kaidel/Renderer/RendererAPI.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

namespace Kaidel {

	class RenderCommand
	{
	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}
		
		static void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) {
			s_RendererAPI->GetViewport(x, y, width, height);
		}

		static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		static void Clear()
		{
			s_RendererAPI->Clear();
		}

		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}
		static void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) {
			s_RendererAPI->DrawIndexedInstanced(vertexArray, indexCount, instanceCount);
		}
		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount = 0)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		static void DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
			s_RendererAPI->DrawPatches(vertexArray, vertexCount);
		}

		static void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
			s_RendererAPI->DrawPoints(vertexArray, vertexCount);
		}

		static void SetCullMode(CullMode cullMode) {
			s_RendererAPI->SetCullMode(cullMode);
		}
		static void SetLineWidth(float width) {
			s_RendererAPI->SetLineWidth(width);
		}


		static void SetPointSize(float pixelSize) {
			s_RendererAPI->SetPointSize(pixelSize);
		}


		static int QueryMaxTextureSlots() {
			return s_RendererAPI->QueryMaxTextureSlots();
		}
		
		static float QueryMaxTessellationLevel() {
			return s_RendererAPI->QueryMaxTessellationLevel();
		}

		static void SetPatchVertexCount(uint32_t count) {
			s_RendererAPI->SetPatchVertexCount(count);
		}


		static void SetDefaultTessellationLevels(const glm::vec4& outer={1,1,1,1}, const glm::vec2& inner= {1,1}) {
			s_RendererAPI->SetDefaultTessellationLevels(outer, inner);
		}

		static void RenderFullScreenQuad(Ref<Shader> shader,uint32_t width,uint32_t height) {
			s_RendererAPI->RenderFullScreenQuad(shader,width,height);
		}

		static Scope<RendererAPI>& GetRendererAPI() { return s_RendererAPI; }

		
	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
