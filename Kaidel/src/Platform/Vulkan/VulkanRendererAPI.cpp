#include "KDpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicsAPI/VulkanVertexBuffer.h"
#include "GraphicsAPI/VulkanIndexBuffer.h"
#include "GraphicsAPI/VulkanRenderPass.h"
#include "GraphicsAPI/VulkanGraphicsPipeline.h"
#include "GraphicsAPI/VulkanUniformBuffer.h"
#include "GraphicsAPI/VulkanFramebuffer.h"
#include "GraphicsAPI/VulkanGraphicsContext.h"

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
    void VulkanRendererAPI::RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height) const
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
		if (newLayout == image.Layout)
			return;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = Utils::ImageLayoutToVulkanImageLayout(image.Layout);
		barrier.newLayout = Utils::ImageLayoutToVulkanImageLayout(newLayout);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = (VkImage)image._InternalImageID;

		if (Utils::IsDepthFormat(image.ImageFormat)) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (image.ImageFormat == Format::Depth24Stencil8) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = image.Levels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = image.Layers;

		VkPipelineStageFlags sourceStage = Utils::GetPipelineStageForLayout(image.Layout);
		VkPipelineStageFlags destinationStage = Utils::GetPipelineStageForLayout(newLayout);

		VkCommandBuffer cmdBuffer = VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer();

		vkCmdPipelineBarrier(
			cmdBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		image.Layout = newLayout;
		image.IntendedLayout = newLayout;
    }
	void VulkanRendererAPI::BindUniformBuffer(Ref<UniformBuffer> uniformBuffer, uint32_t index)
	{
		if (!m_CurrentBoundPipeline)
			return;

		VulkanGraphicsPipeline* pipeline = (VulkanGraphicsPipeline*)m_CurrentBoundPipeline.Get();
		VkDescriptorSet set = ((VulkanUniformBuffer*)uniformBuffer.Get())->GetDescriptorSet();
		vkCmdBindDescriptorSets(VK_CONTEXT.GetActiveCommandBuffer()->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), index, 1,&set , 0, nullptr);
	}
}
