#pragma once

#include "VulkanDefinitions.h"

#include "Kaidel/Renderer/RendererAPI.h"

#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"

namespace Kaidel {

	class VulkanRendererAPI :public RendererAPI {
	public:

	private:
		// Inherited via RendererAPI
		void Init() override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) override;

		void SetClearColor(const glm::vec4& color) override;

		void Clear() override;

		void Shutdown() override;

		void Draw(uint32_t vertexCount, uint32_t firstVertexID) override;

		void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertexID) override;

		void DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset) override;

		void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset) override;

		void RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height) const override;

		void BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> vertexBuffers) override;

		void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) override;

		void BeginRenderPass(Ref<Framebuffer> frameBuffer, Ref<RenderPass> renderPass) override;

		void EndRenderPass() override;

		void BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline) override;

		void Submit(std::function<void()>&& func) override;

		void Transition(Image& image, ImageLayout newLayout) override;
		void BindUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t index) override;

	private:
		static inline std::vector<VkBuffer> m_CurrentBoundBuffers = { 32 , 0 };
		static inline std::vector<VkDeviceSize> m_CurrentBoundBufferOffsets = std::vector<VkDeviceSize>(32, VkDeviceSize(0));

		Ref<GraphicsPipeline> m_CurrentBoundPipeline = {};

		// Inherited via RendererAPI
		//static inline Ref<Framebuffer> m_BoundFramebuffer{};
	};
}
