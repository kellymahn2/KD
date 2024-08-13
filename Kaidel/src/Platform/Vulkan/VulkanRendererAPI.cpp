#include "KDpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicsAPI/VulkanVertexBuffer.h"
#include "GraphicsAPI/VulkanIndexBuffer.h"
#include "GraphicsAPI/VulkanRenderPass.h"
#include "GraphicsAPI/VulkanGraphicsPipeline.h"
#include "GraphicsAPI/VulkanUniformBuffer.h"
#include "GraphicsAPI/VulkanFramebuffer.h"
#include "GraphicsAPI/VulkanGraphicsContext.h"
#include "GraphicsAPI/VulkanTransferBuffer.h"


namespace Kaidel {
    void VulkanRendererAPI::Init()
    {
    }
    void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
    }
    void VulkanRendererAPI::GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height)
    {
    }
    void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
    {
    }
    void VulkanRendererAPI::Clear()
    {
    }
    void VulkanRendererAPI::Shutdown()
    {
		m_CurrentBoundPipeline = {};
    }
    void VulkanRendererAPI::Draw(uint32_t vertexCount, uint32_t firstVertexID)
    {
		vkCmdDraw(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), vertexCount, 1, firstVertexID, 0);
    }
    void VulkanRendererAPI::DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertexID)
    {
		vkCmdDraw(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), vertexCount, instanceCount, firstVertexID, 0);
    }
    void VulkanRendererAPI::DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
    {
		vkCmdDrawIndexed(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(),indexCount, 1, firstIndex, vertexOffset,0);
    }
    void VulkanRendererAPI::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset)
    {
		vkCmdDrawIndexed(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, 0);
	}
    void VulkanRendererAPI::RenderFullScreenQuad(Ref<ShaderModule> shader, uint32_t width, uint32_t height) const
    {
	}
    void VulkanRendererAPI::BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> vertexBuffers)
    {
		uint32_t i = 0;
		for (auto& vb : vertexBuffers) {
			VulkanVertexBuffer* vvb = static_cast<VulkanVertexBuffer*>(vb.Get());
			m_CurrentBoundBuffers[i] = vvb->GetBuffer().Buffer;
			++i;
		}
		vkCmdBindVertexBuffers(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), 0, vertexBuffers.size(), m_CurrentBoundBuffers.data(), m_CurrentBoundBufferOffsets.data());
    }
    void VulkanRendererAPI::BindIndexBuffer(Ref<IndexBuffer> indexBuffer)
    {
		VulkanIndexBuffer* vib = static_cast<VulkanIndexBuffer*>(indexBuffer.Get());
		vkCmdBindIndexBuffer(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), vib->GetBuffer().Buffer, 0, VK_INDEX_TYPE_UINT32);
    }
	void VulkanRendererAPI::BindDescriptorSet(Ref<DescriptorSet> descriptorSet, uint32_t setIndex)
	{

		if (!m_CurrentBoundPipeline)
			return;

		VulkanGraphicsPipeline* pipeline = (VulkanGraphicsPipeline*)m_CurrentBoundPipeline.Get();
		VkDescriptorSet set = (VkDescriptorSet)descriptorSet->GetSetID();

		vkCmdBindDescriptorSets(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), setIndex, 1, &set, 0, nullptr);
	}
    void VulkanRendererAPI::BeginRenderPass(Ref<Framebuffer> frameBuffer, Ref<RenderPass> renderPass)
    {
		Ref<VulkanRenderPass> rp = renderPass;
		Ref<VulkanFramebuffer> fb = frameBuffer;
		VkRenderPassBeginInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		info.clearValueCount = (uint32_t)rp->GetClearValues().size();
		info.pClearValues = rp->GetClearValues().data();
		info.framebuffer = (VkFramebuffer)frameBuffer->GetRendererID();
		info.renderPass = (VkRenderPass)rp->GetRendererID();
		VkRect2D renderArea{};
		renderArea.offset = { 0,0 };
		renderArea.extent = { fb->GetWidth(),fb->GetHeight() };
		info.renderArea = renderArea;
		vkCmdBeginRenderPass(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), &info, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport vp;
		vp.x = 0;
		vp.y = 0;
		vp.width = renderArea.extent.width;
		vp.height = renderArea.extent.height;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vkCmdSetViewport(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), 0, 1, &vp);
		vkCmdSetScissor(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), 0, 1, &renderArea);

		for (int i = 0; i < frameBuffer->GetColorAttachmentCount(); ++i) {
			Image& image = frameBuffer->GetImage(i);
			image.Layout = renderPass->GetSpecification().OutputColors[i].FinalLayout;
		}
    }
    void VulkanRendererAPI::EndRenderPass()
    {
		vkCmdEndRenderPass(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer());
    }
    void VulkanRendererAPI::BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline)
    {
		vkCmdBindPipeline(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)pipeline->GetRendererID());
		m_CurrentBoundPipeline = pipeline;
    }
    void VulkanRendererAPI::Submit(std::function<void()>&& func)
    {
		
    }
    void VulkanRendererAPI::Transition(Image& image, ImageLayout newLayout)
    {
		Utils::Transition(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), image, newLayout);
    }
	void VulkanRendererAPI::BindUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t index)
	{
		//if (!m_CurrentBoundPipeline)
		//	return;
		//
		//VulkanGraphicsPipeline* pipeline = (VulkanGraphicsPipeline*)m_CurrentBoundPipeline.Get();
		//VkDescriptorSet set = ((VulkanUniformBuffer*)uniformBuffer.Get())->GetDescriptorSet();
		//vkCmdBindDescriptorSets(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), index, 1,&set , 0, nullptr);
	}
	void VulkanRendererAPI::CopyBufferToTexture(Ref<TransferBuffer> src, Image& dst, const BufferToTextureCopyRegion& region)
	{
		bool needsTransition = dst.Layout != ImageLayout::TransferDstOptimal;
		ImageLayout oldLayout = dst.Layout;
		oldLayout = oldLayout == ImageLayout::None ? ImageLayout::ShaderReadOnlyOptimal : oldLayout;
		Ref<VulkanTransferBuffer> vulkanSrcBuffer = src;	

		if (needsTransition)
			Transition(dst, ImageLayout::TransferDstOptimal);
		//Copy
		VkBufferImageCopy copyRegion = {};
		
		copyRegion.bufferOffset = region.BufferOffset;
		
		copyRegion.imageOffset.x = region.TextureOffset.x;
		copyRegion.imageOffset.y = region.TextureOffset.y;
		copyRegion.imageOffset.z = region.TextureOffset.z;
		
		copyRegion.imageExtent.width = region.TextureRegionSize.x;
		copyRegion.imageExtent.height = region.TextureRegionSize.y;
		copyRegion.imageExtent.depth = region.TextureRegionSize.z;

		copyRegion.imageSubresource.aspectMask = Utils::GetAspectFlags(dst.ImageFormat);
		copyRegion.imageSubresource.mipLevel = region.Mipmap;
		copyRegion.imageSubresource.baseArrayLayer = region.StartLayer;
		copyRegion.imageSubresource.layerCount = region.LayerCount;

		vkCmdCopyBufferToImage(
			VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(),
			vulkanSrcBuffer->GetBuffer().Buffer, 
			(VkImage)dst._InternalImageID,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);

		//Revert transition
		if (needsTransition)
			Transition(dst, oldLayout);
	}
	void VulkanRendererAPI::ClearColorImage(Image& image, const AttachmentColorClearValue& clearValue, const TextureSubresourceRegion& region)
	{
		VkClearColorValue color{};
		memcpy(color.float32, &clearValue.RGBAF, sizeof(color.float32));

		VkImageSubresourceRange subresource{};
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.baseArrayLayer = region.StartLayer;
		subresource.layerCount = region.LayerCount;
		subresource.baseMipLevel = region.StarMip;
		subresource.levelCount = region.MipCount;

		VkImageLayout layout = Utils::ImageLayoutToVulkanImageLayout(image.Layout);


		vkCmdClearColorImage(
			VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(),
			(VkImage)image._InternalImageID,
			layout,
			&color,
			1,
			&subresource
		);

	}
	void VulkanRendererAPI::BindPushConstants(Ref<GraphicsPipeline> pipeline, ShaderType type, const void* data, uint64_t size)
	{
		Ref<VulkanGraphicsPipeline> vulkanPipeline = pipeline;
		vkCmdPushConstants(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), vulkanPipeline->GetLayout(), Utils::ShaderTypeToVulkanShaderStageFlag(type), 0, size, data);
	}
}
