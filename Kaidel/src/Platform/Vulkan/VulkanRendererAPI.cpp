#include "KDpch.h"
#include "VulkanRendererAPI.h"

#include "GraphicsAPI/VulkanVertexBuffer.h"
#include "GraphicsAPI/VulkanIndexBuffer.h"
#include "GraphicsAPI/VulkanRenderPass.h"
#include "GraphicsAPI/VulkanGraphicsPipeline.h"
#include "GraphicsAPI/VulkanUniformBuffer.h"
#include "GraphicsAPI/VulkanFramebuffer.h"
#include "GraphicsAPI/VulkanGraphicsContext.h"
#include "GraphicsAPI/VulkanShader.h"
#include "GraphicsAPI/VulkanDescriptorSet.h"

#include "GraphicsAPI/VulkanStorageBuffer.h"

namespace Kaidel {

	namespace Utils {
		static VkImageMemoryBarrier TransitionMipBarrier(const VulkanBackend::TextureInfo& info, uint32_t mip,
			VkImageLayout srcLayout, VkAccessFlags srcAccess,
			VkImageLayout dstLayout, VkAccessFlags dstAccess) {

			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.srcAccessMask = srcAccess;
			barrier.dstAccessMask = dstAccess;
			barrier.oldLayout = srcLayout;
			barrier.newLayout = dstLayout;
			barrier.image = info.ViewInfo.image;
			barrier.subresourceRange = info.ViewInfo.subresourceRange;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseMipLevel = mip;
			barrier.subresourceRange.layerCount = info.ImageInfo.arrayLayers;

			return barrier;
		}
	}

    void VulkanRendererAPI::Init()
    {
    }
    void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
    }
	void VulkanRendererAPI::DeviceWaitIdle()
	{
		vkDeviceWaitIdle(VK_DEVICE.GetDevice());
	}
    void VulkanRendererAPI::Shutdown()
    {
		m_CurrentBoundPipeline = {};
    }
	void VulkanRendererAPI::BeginRenderPass(
		Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer,
		std::initializer_list<AttachmentClearValue> clearValues)
	{
		Ref<VulkanRenderPass> rp = renderPass;
		Ref<VulkanFramebuffer> fb = framebuffer;

		VkRect2D renderArea{};
		renderArea.offset = { 0,0 };
		renderArea.extent = { fb->GetSpecification().Width,fb->GetSpecification().Height };

		const VkClearValue* clearBegin = rp->GetVkClearValues().data();
		const VkClearValue* clearEnd = rp->GetVkClearValues().data() + rp->GetVkClearValues().size();

		VK_BACKEND->CommandBeginRenderPass(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			rp->GetRenderPass(), fb->GetFramebuffer(),
			VK_SUBPASS_CONTENTS_INLINE,
			renderArea,
			std::initializer_list<VkClearValue>(clearBegin,clearEnd));

		for (auto& val : fb->GetResources().Textures) {
			ImageLayout layout = 
				Utils::IsDepthFormat(val->GetTextureSpecification().Format) ? 
				ImageLayout::DepthAttachmentOptimal : ImageLayout::ColorAttachmentOptimal;
			val->SetImageLayout(layout);
		}
		m_RenderPassInstance.RenderPass = rp;
		m_RenderPassInstance.Framebuffer = fb;
	}
	void VulkanRendererAPI::EndRenderPass()
    {
		VK_BACKEND->CommandEndRenderPass(m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER);

		uint32_t colorIndex = 0;
		for (auto& val : m_RenderPassInstance.Framebuffer->GetResources().Textures) {
			if (Utils::IsDepthFormat(val->GetTextureSpecification().Format)) {
				val->SetImageLayout(m_RenderPassInstance.RenderPass->GetSpecification().DepthStencil.FinalLayout);
			}
			else {
				val->SetImageLayout(m_RenderPassInstance.RenderPass->GetSpecification().Colors[colorIndex].FinalLayout);
				++colorIndex;
			}
		}

		m_RenderPassInstance = {};
    }
	void VulkanRendererAPI::NextSubpass()
	{
		VK_BACKEND->CommandNextSubpass(m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER, VK_SUBPASS_CONTENTS_INLINE);
	}
	void VulkanRendererAPI::BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> buffers, std::initializer_list<uint64_t> offsets)
	{
		VulkanBackend::BufferInfo const** vkBuffers = ALLOCA_ARRAY(VulkanBackend::BufferInfo const*, buffers.size());
		for (uint32_t i = 0; i < buffers.size(); ++i) {
			const Ref<VertexBuffer>* buffer = buffers.begin() + i;
			Ref<VulkanVertexBuffer> vkBuffer = *buffer;
			vkBuffers[i] = &vkBuffer->GetBufferInfo();
		}
		
		VK_BACKEND->CommandBindVertexBuffers(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			std::initializer_list<const VulkanBackend::BufferInfo*>(vkBuffers, vkBuffers + buffers.size()),
			offsets
		);
	}
	void VulkanRendererAPI::BindIndexBuffer(Ref<IndexBuffer> buffer, uint64_t offset)
	{
		Ref<VulkanIndexBuffer> vkBuffer = buffer;
		VK_BACKEND->CommandBindIndexBuffer(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			vkBuffer->GetBufferInfo(),
			Utils::IndexTypeToVulkanIndexType(vkBuffer->GetIndexType()),
			offset);
	}
    void VulkanRendererAPI::BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline)
    {
		if (m_CurrentBoundPipeline == pipeline)
			return;
		Ref<VulkanGraphicsPipeline> vkPipeline = pipeline;
		VK_BACKEND->CommandBindGraphicsPipeline(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			vkPipeline->GetPipeline()
		);
		m_CurrentBoundPipeline = pipeline;
    }
	void VulkanRendererAPI::BindComputePipeline(Ref<ComputePipeline> pipeline)
	{
		Ref<VulkanComputePipeline> vkPipeline = pipeline;
		VK_BACKEND->CommandBindComputePipeline(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			vkPipeline->GetPipeline()
		);
	}
	void VulkanRendererAPI::BindPushConstants(Ref<Shader> shader, uint32_t firstIndex, const uint8_t* values, uint64_t size)
	{
		Ref<VulkanShader> vkShader = shader;
		VK_BACKEND->CommandBindPushConstants(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			vkShader->GetShaderInfo(),
			firstIndex, values, size
		);
	}
	void VulkanRendererAPI::BindDescriptorSet(Ref<Shader> shader, Ref<DescriptorSet> set, uint32_t setIndex)
	{
		Ref<VulkanShader> vkShader = shader;
		Ref<VulkanDescriptorSet> vkSet = set;
		VK_BACKEND->CommandBindDescriptorSet(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			vkShader->GetShaderInfo(),
			vkSet->GetSetInfo(),
			setIndex
		);
	}
	void VulkanRendererAPI::SetViewport(uint32_t width, uint32_t height, int32_t offsetX, int32_t offsetY)
	{
		VkRect2D viewport{};
		viewport.extent = { width,height };
		viewport.offset = { offsetX,offsetY };

		VK_BACKEND->CommandSetViewport(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			{ viewport }
		);
	}
	void VulkanRendererAPI::SetScissor(uint32_t width, uint32_t height, int32_t offsetX, int32_t offsetY)
	{
		VkRect2D scissor{};
		scissor.extent = { width,height };
		scissor.offset = { offsetX,offsetY };
		
		VK_BACKEND->CommandSetScissor(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			{ scissor }
		);
	}
	void VulkanRendererAPI::SetBlendConstants(const float color[4])
	{
		VK_BACKEND->CommandSetBlendConstants(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			color
		);
	}
	void VulkanRendererAPI::SetLineWidth(float width)
	{
		VK_BACKEND->CommandSetLineWidth(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER, 
			width
		);
	}
	void VulkanRendererAPI::ClearAttachments(std::initializer_list<AttachmentClear> clearValues, std::initializer_list<ClearRect> rects)
	{
		VkClearAttachment* vkClears = ALLOCA_ARRAY(VkClearAttachment, clearValues.size());
		for (uint32_t i = 0; i < clearValues.size(); ++i) {
			const AttachmentClear* clear = clearValues.begin() + i;
			vkClears[i].aspectMask = clear->Aspect;
			vkClears[i].colorAttachment = clear->ColorAttachment;
			memcpy(&vkClears[i].clearValue, &clear->Value, sizeof(VkClearValue));
		}

		VkClearRect* vkRects = ALLOCA_ARRAY(VkClearRect, rects.size());
		for (uint32_t i = 0; i < rects.size(); ++i) {
			const ClearRect* rect = rects.begin() + i;
			vkRects->baseArrayLayer = rect->StartLayer;
			vkRects->layerCount = rect->LayerCount;
			vkRects->rect.extent = { rect->Rect.Size.x, rect->Rect.Size.y };
			vkRects->rect.offset = { rect->Rect.Offset.x, rect->Rect.Offset.y };
		}
		
		VK_BACKEND->CommandClearAttachments(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			std::initializer_list<VkClearAttachment>(vkClears, vkClears + clearValues.size()),
			std::initializer_list<VkClearRect>(vkRects, vkRects + clearValues.size())
		);
	}
	void VulkanRendererAPI::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset)
	{
		VK_BACKEND->CommandDraw(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			vertexCount, instanceCount, vertexOffset, instanceOffset
		);
	}
	void VulkanRendererAPI::DrawIndexed(uint32_t indexCount, uint32_t vertexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset)
	{
		VK_BACKEND->CommandDrawIndexed(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			indexCount,vertexCount,instanceCount,indexOffset,vertexOffset,instanceOffset
		);
	}
	void VulkanRendererAPI::Dispatch(uint32_t x, uint32_t y, uint32_t z)
	{
		VK_BACKEND->CommandDispatch(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			x, y, z
		);
	}
	void VulkanRendererAPI::CopyTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip, Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip)
	{
		const VulkanBackend::TextureInfo* srcInfo = (const VulkanBackend::TextureInfo*)srcTexture->GetBackendInfo();
		const VulkanBackend::TextureInfo* dstInfo = (const VulkanBackend::TextureInfo*)dstTexture->GetBackendInfo();
		VkImageCopy region{};
		region.extent = { srcTexture->GetTextureSpecification().Width,srcTexture->GetTextureSpecification().Height,srcTexture->GetTextureSpecification().Depth };
		region.srcOffset = { 0,0,0 };
		region.dstOffset = { 0,0,0 };
		region.srcSubresource.aspectMask = srcInfo->ViewInfo.subresourceRange.aspectMask;
		region.srcSubresource.baseArrayLayer = srcLayer;
		region.srcSubresource.layerCount = 1;
		region.srcSubresource.mipLevel = srcMip;
		region.dstSubresource.aspectMask = dstInfo->ViewInfo.subresourceRange.aspectMask;
		region.dstSubresource.baseArrayLayer = dstLayer;
		region.dstSubresource.layerCount = 1;
		region.dstSubresource.mipLevel = dstMip;

		VK_BACKEND->CommandCopyTexture(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			*srcInfo,
			Utils::ImageLayoutToVulkanImageLayout(srcTexture->GetTextureSpecification().Layout),
			*dstInfo,
			Utils::ImageLayoutToVulkanImageLayout(dstTexture->GetTextureSpecification().Layout),
			{ region }
		);
	}
	void VulkanRendererAPI::ResolveTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip, Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip)
	{
		const VulkanBackend::TextureInfo* srcInfo = (const VulkanBackend::TextureInfo*)srcTexture->GetBackendInfo();
		const VulkanBackend::TextureInfo* dstInfo = (const VulkanBackend::TextureInfo*)dstTexture->GetBackendInfo();

		VK_BACKEND->CommandResolveTexture(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			*srcInfo,
			Utils::ImageLayoutToVulkanImageLayout(srcTexture->GetTextureSpecification().Layout),
			srcLayer, srcMip,
			*dstInfo,
			Utils::ImageLayoutToVulkanImageLayout(dstTexture->GetTextureSpecification().Layout),
			dstLayer, dstMip);
	}
	void VulkanRendererAPI::ClearColorTexture(Ref<Texture> texture, const AttachmentColorClearValue& clear)
	{
		const auto& info =  *(const VulkanBackend::TextureInfo*)texture->GetBackendInfo();
		VkImageSubresourceRange range{};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer = 0;
		range.baseMipLevel = 0;
		range.layerCount = texture->GetTextureSpecification().Layers;
		range.levelCount = texture->GetTextureSpecification().Mips;
		VK_BACKEND->CommandClearColorTexture(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			info,
			Utils::ImageLayoutToVulkanImageLayout(texture->GetTextureSpecification().Layout),
			(const VkClearColorValue&)clear,
			range
		);
	}
	void VulkanRendererAPI::PipelineBarrier(
		PipelineStages srcStages, 
		PipelineStages dstStages, 
		std::initializer_list<MemoryBarrier> memoryBarriers, 
		std::initializer_list<BufferMemoryBarrier> bufferBarriers, 
		std::initializer_list<ImageMemoryBarrier> textureBarriers)
	{
		VkPipelineStageFlags vkSrcStages = Utils::PipelineStagesToVulkanPipelineStageFlags(srcStages);
		VkPipelineStageFlags vkDstStages = Utils::PipelineStagesToVulkanPipelineStageFlags(dstStages);

		VkMemoryBarrier* vkMemoryBarriers = ALLOCA_ARRAY(VkMemoryBarrier, memoryBarriers.size());
		for (uint32_t i = 0; i < memoryBarriers.size(); ++i) {
			vkMemoryBarriers[i] = {};

			const MemoryBarrier* barrier = memoryBarriers.begin() + i;
			vkMemoryBarriers[i].sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			vkMemoryBarriers[i].srcAccessMask = Utils::AccessFlagsToVulkanAccessFlags(barrier->Src);
			vkMemoryBarriers[i].dstAccessMask = Utils::AccessFlagsToVulkanAccessFlags(barrier->Dst);
		}

		VkBufferMemoryBarrier* vkBufferBarriers = ALLOCA_ARRAY(VkBufferMemoryBarrier, bufferBarriers.size());
		for (uint32_t i = 0; i < bufferBarriers.size(); ++i) {
			vkBufferBarriers[i] = {};

			const BufferMemoryBarrier* barrier = bufferBarriers.begin() + i;
			
			const VulkanBackend::BufferInfo* info = (const VulkanBackend::BufferInfo*)barrier->Buffer->GetBackendID();
			
			uint64_t allowedSize = info->BufferSize - barrier->Offset;

			vkBufferBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			vkBufferBarriers[i].srcAccessMask = Utils::AccessFlagsToVulkanAccessFlags(barrier->Src);
			vkBufferBarriers[i].dstAccessMask = Utils::AccessFlagsToVulkanAccessFlags(barrier->Dst);
			vkBufferBarriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkBufferBarriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkBufferBarriers[i].buffer = info->Buffer;
			vkBufferBarriers[i].offset = barrier->Offset;
			vkBufferBarriers[i].size = std::min(allowedSize,barrier->Size);
		}

		VkImageMemoryBarrier* vkImageBarriers = ALLOCA_ARRAY(VkImageMemoryBarrier, textureBarriers.size());
		for (uint32_t i = 0; i < textureBarriers.size(); ++i) {
			vkImageBarriers[i] = {};

			const ImageMemoryBarrier* barrier = textureBarriers.begin() + i;
			const auto& info = *(VulkanBackend::TextureInfo*)barrier->Image->GetBackendInfo();

			vkImageBarriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vkImageBarriers[i].srcAccessMask = Utils::AccessFlagsToVulkanAccessFlags(barrier->Src);
			vkImageBarriers[i].dstAccessMask = Utils::AccessFlagsToVulkanAccessFlags(barrier->Dst);
			vkImageBarriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkImageBarriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			vkImageBarriers[i].image = info.ViewInfo.image;
			vkImageBarriers[i].oldLayout = Utils::ImageLayoutToVulkanImageLayout(barrier->OldLayout);
			vkImageBarriers[i].newLayout = Utils::ImageLayoutToVulkanImageLayout(barrier->NewLayout);
		
			vkImageBarriers[i].subresourceRange.aspectMask = info.ViewInfo.subresourceRange.aspectMask;
			vkImageBarriers[i].subresourceRange.baseArrayLayer = barrier->Subresource.StartLayer;
			vkImageBarriers[i].subresourceRange.baseMipLevel = barrier->Subresource.StarMip;
			vkImageBarriers[i].subresourceRange.layerCount = barrier->Subresource.LayerCount;
			vkImageBarriers[i].subresourceRange.levelCount = barrier->Subresource.MipCount;
		}

		VK_BACKEND->CommandPipelineBarrier(
			m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER,
			vkSrcStages,
			vkDstStages,
			std::initializer_list<VkMemoryBarrier>(vkMemoryBarriers, vkMemoryBarriers + memoryBarriers.size()),
			std::initializer_list<VkBufferMemoryBarrier>(vkBufferBarriers, vkBufferBarriers + bufferBarriers.size()),
			std::initializer_list<VkImageMemoryBarrier>(vkImageBarriers, vkImageBarriers + textureBarriers.size())
		);

		for (uint32_t i = 0; i < textureBarriers.size(); ++i) {
			const ImageMemoryBarrier* barrier = textureBarriers.begin() + i;
			barrier->Image->SetImageLayout(barrier->NewLayout);
		}
	}

	void VulkanRendererAPI::Submit(std::function<void()>&& func)
    {
		m_OverrideCommandBuffer = VK_BACKEND->CreateCommandBuffer(VK_CONTEXT.GetPrimaryCommandPool());
		VK_BACKEND->CommandBufferBegin(m_OverrideCommandBuffer);

		func();

		VK_BACKEND->CommandBufferEnd(m_OverrideCommandBuffer);

		VK_BACKEND->SubmitCommandBuffers(VK_CONTEXT.GetGraphicsQueue(), { m_OverrideCommandBuffer }, VK_CONTEXT.GetSingleSubmitFence());
		VK_BACKEND->FenceWait(VK_CONTEXT.GetSingleSubmitFence());

		VK_BACKEND->DestroyCommandBuffer(m_OverrideCommandBuffer, VK_CONTEXT.GetPrimaryCommandPool());
		m_OverrideCommandBuffer = VK_NULL_HANDLE;
    }

	void VulkanRendererAPI::GenerateMips(const Ref<Texture> texture)
	{
		KD_CORE_ASSERT(texture->GetTextureSpecification().Layout == ImageLayout::TransferSrcOptimal);

		auto& specs = texture->GetTextureSpecification();

		auto& info = *(VulkanBackend::TextureInfo*)texture->GetBackendInfo();

		VkCommandBuffer cb = m_OverrideCommandBuffer ? m_OverrideCommandBuffer : VK_CURRENT_COMMAND_BUFFER;

		int w = (uint32_t)specs.Width;
		int h = (uint32_t)specs.Height;


		for (uint32_t i = 1; i < specs.Mips; ++i) {

			//Transform mip i to dst.
			{
				VkImageMemoryBarrier barrier =
					Utils::TransitionMipBarrier(
						info, i,
						VK_IMAGE_LAYOUT_UNDEFINED, 0,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT);
				VK_BACKEND->CommandPipelineBarrier(
					cb,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					{},
					{},
					{ barrier });
			}
			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { w, h, 1 };
			blit.srcSubresource.aspectMask = info.ViewInfo.subresourceRange.aspectMask;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = info.ImageInfo.arrayLayers;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { w > 1 ? w / 2 : 1, h > 1 ? h / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = info.ViewInfo.subresourceRange.aspectMask;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = info.ImageInfo.arrayLayers;

			VK_BACKEND->CommandBlitTexture(
				cb,
				info, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				info, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				blit);

			//Transform mip i to src for next iteration.
			{
				VkImageMemoryBarrier barrier =
					Utils::TransitionMipBarrier(
						info, i,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT);
				VK_BACKEND->CommandPipelineBarrier(
					cb,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					{}, {}, { barrier }
				);
			}

			if (w > 1)
				w /= 2;
			if (h > 1)
				h /= 2;
		}
	}
}
