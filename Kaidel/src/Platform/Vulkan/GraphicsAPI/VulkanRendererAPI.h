#pragma once

#include "VulkanBase.h"
#include "Kaidel/Renderer/RendererAPI.h"
#include "VulkanSyncObjects.h"
#include "PerFrameResource.h"
#include <queue>


namespace Kaidel {
	namespace Vulkan {

		struct SubmittingControlBlock {
			Ref<VulkanTimelineSemaphore> ProcessingFinished;
			Ref<VulkanTimelineSemaphore> RenderingFinished;
			uint64_t ProcessingFinishedSemaphoreValue;
			uint64_t RenderingFinishedSemaphoreValue;
			std::queue<std::function<void()>> Functions;
		};


		class VulkanRendererAPI : public RendererAPI{
		public:

			virtual void Init() override;
			virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
			virtual void GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) override;
			virtual void SetClearColor(const glm::vec4& color) override;
			virtual void Clear() override;

			virtual void Shutdown()override;

			virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) override;
			virtual void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount)override;
			virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
			virtual void DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
			virtual void DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

			virtual void SetLineWidth(float thickness) override;
			virtual void SetPointSize(float pixelSize) override;
			virtual void SetCullMode(CullMode cullMode) override;

			virtual void SetPatchVertexCount(uint32_t count)override;

			virtual void RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height)const override;

			virtual void BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> vertexBuffers) override;
			virtual void BindIndexBuffer(Ref<IndexBuffer> indexBuffer) override;

			virtual void BeginRenderPass(Ref<Framebuffer> frameBuffer, Ref<RenderPass> renderPass) override;
			virtual void EndRenderPass() override;

			virtual void BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline) override;

			void Submit(std::function<void()>&& func)override;

			PerFrameResource<SubmittingControlBlock>& GetControlBlocks() { return m_SubmittedFunctions; }

		private:
			VkSwapchainKHR m_Swapchain;

			static constexpr const uint32_t MaxVertexBuffersBoundAtOnce = 32;

			Ref<Framebuffer> m_BoundFramebuffer;
			std::vector<VkBuffer> m_CurrentBoundBuffers;
			std::vector<VkDeviceSize> m_CurrentBoundBufferOffsets;
			glm::vec4 m_ClearValues;


			//TODO: use semaphore as signal for render complete to start processing functions on seperate thread,
			//		then after all processes signal a process finished semaphore which must be waited on at the end of 
			//		every frame in which case dont make a seperate thread.
			

			PerFrameResource<SubmittingControlBlock> m_SubmittedFunctions;

			// Inherited via RendererAPI
			void Draw(uint32_t vertexCount, uint32_t firstVertexID) override;
			void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertexID) override;
			void DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset) override;
			void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset) override;
		};
	}
}
