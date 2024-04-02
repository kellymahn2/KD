#pragma once


#include "Kaidel/Renderer/RendererAPI.h"
#include <glad/vulkan.h>


namespace Kaidel {

	class VulkanRendererAPI : public RendererAPI{
	public:
		void Init() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear() override;
		void Shutdown() override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
		void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount) override;
		void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		void DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
		void SetLineWidth(float thickness) override;
		void SetPointSize(float pixelSize) override;
		void SetCullMode(CullMode cullMode) override;
		int QueryMaxTextureSlots() override;
		float QueryMaxTessellationLevel() override;
		void SetPatchVertexCount(uint32_t count) override;
		void SetDefaultTessellationLevels(const glm::vec4& outer, const glm::vec2& inner) override;
		void RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height) const override;

	private:
	};

}
