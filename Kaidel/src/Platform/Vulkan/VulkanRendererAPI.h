#pragma once

#include "VulkanDefinitions.h"

#include "Kaidel/Renderer/RendererAPI.h"

#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"
#include "GraphicsAPI/VulkanRenderPass.h"
#include "GraphicsAPI/VulkanFramebuffer.h"

namespace Kaidel {

	class VulkanRendererAPI :public RendererAPI {
		struct RenderPassInstance {
			Ref<VulkanRenderPass> RenderPass;
			Ref<VulkanFramebuffer> Framebuffer;
		};
	public:
		// Inherited via RendererAPI
		void Init() override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void DeviceWaitIdle()override;
		void Shutdown() override;

		void Submit(std::function<void()>&& func) override;

		virtual void BeginRenderPass(Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer,
			std::initializer_list<AttachmentClearValue> clearValues)override;
		virtual void EndRenderPass()override;
		virtual void NextSubpass()override;

		virtual void BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> buffers, std::initializer_list<uint64_t> offsets)override;
		virtual void BindIndexBuffer(Ref<IndexBuffer> buffer, uint64_t offset)override;

		virtual void BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline)override;
		virtual void BindPushConstants(Ref<Shader> shader, uint32_t firstIndex, const uint8_t* values, uint64_t size)override;
		virtual void BindDescriptorSet(Ref<Shader> shader, Ref<DescriptorSet> set, uint32_t setIndex)override;

		virtual void SetViewport(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0)override;
		virtual void SetScissor(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0)override;
		virtual void SetBlendConstants(const float color[4])override;
		virtual void SetLineWidth(float width)override;

		virtual void ClearAttachments(std::initializer_list<AttachmentClear> clearValues, std::initializer_list<ClearRect> rects)override;

		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset)override;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset)override;

		virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z)override;

		//void ClearBuffer(Ref<VertexBuffer> buffer, uint64_t offset, uint64_t size);
		//void CopyBuffer(In<BufferInfo> srcBuffer, In<BufferInfo> dstBuffer, std::initializer_list<VkBufferCopy> regions);
		/*virtual void CopyTexture(Ref<Texture> srcTexture,
			Ref<Texture> dstTexture, std::initializer_list<VkImageCopy> regions)override;
		virtual void ResolveTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip,
			Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip)override;*/
		virtual void ResolveTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip,
			Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip)override;
		virtual void ClearColorTexture(Ref<Texture> texture,
			const AttachmentColorClearValue& clear)override;
		//void CopyBufferToTexture(In<BufferInfo> srcBuffer, In<TextureInfo> dstTexture, VkImageLayout dstLayout,
		//	std::initializer_list<VkBufferImageCopy> regions);
		//void CopyTextureToBuffer(In<TextureInfo> srcTexture, VkImageLayout srcLayout, In<BufferInfo> dstBuffer,
		//	std::initializer_list<VkBufferImageCopy> regions);

		virtual void PipelineBarrier(
			PipelineStages srcStages,
			PipelineStages dstStages,
			std::initializer_list<MemoryBarrier> memoryBarriers,
			std::initializer_list<BufferMemoryBarrier> bufferBarriers,
			std::initializer_list<ImageMemoryBarrier> textureBarriers)override;

	private:
		static inline std::vector<VkBuffer> m_CurrentBoundBuffers = { 32 , 0 };
		static inline std::vector<VkDeviceSize> m_CurrentBoundBufferOffsets = std::vector<VkDeviceSize>(32, VkDeviceSize(0));
		static inline std::vector<VkDescriptorSet> m_DescriptorSets = { 32,VK_NULL_HANDLE };

		Ref<GraphicsPipeline> m_CurrentBoundPipeline = {};
		RenderPassInstance m_RenderPassInstance{};
	};
}
