#pragma once

#include "Kaidel/Renderer/RendererAPI.h"
#include "Kaidel/Renderer/GraphicsAPI/Buffer.h"
#include "Kaidel/Renderer/GraphicsAPI/VertexArray.h"
namespace Kaidel {

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		virtual void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) override;
		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void Shutdown()override;

		/*virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
		virtual void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount)override;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		virtual void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;



		virtual void SetLineWidth(float thickness) override;
		virtual void SetPointSize(float pixelSize) override;
		virtual void SetCullMode(CullMode cullMode)override;
		virtual void SetPatchVertexCount(uint32_t count)override;*/
		virtual void RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height)const override;
	private:
		Ref<VertexBuffer> m_FullScreenQuadVBO;
		Ref<VertexArray> m_FullScreenQuadVAO;
	};


}
