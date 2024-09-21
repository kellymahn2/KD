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

		const VkClearValue* clearBegin = (const VkClearValue*)clearValues.begin();
		const VkClearValue* clearEnd = clearBegin + clearValues.size();

		VK_BACKEND->CommandBeginRenderPass(
			VK_CURRENT_COMMAND_BUFFER,
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
		VK_BACKEND->CommandEndRenderPass(VK_CURRENT_COMMAND_BUFFER);

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
		VK_BACKEND->CommandNextSubpass(VK_CURRENT_COMMAND_BUFFER, VK_SUBPASS_CONTENTS_INLINE);
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
			VK_CURRENT_COMMAND_BUFFER,
			std::initializer_list<const VulkanBackend::BufferInfo*>(vkBuffers, vkBuffers + buffers.size()),
			offsets
		);
	}
	void VulkanRendererAPI::BindIndexBuffer(Ref<IndexBuffer> buffer, uint64_t offset)
	{
		Ref<VulkanIndexBuffer> vkBuffer = buffer;
		VK_BACKEND->CommandBindIndexBuffer(
			VK_CURRENT_COMMAND_BUFFER,
			vkBuffer->GetBufferInfo(),
			Utils::IndexTypeToVulkanIndexType(vkBuffer->GetIndexType()),
			offset);
	}
    void VulkanRendererAPI::BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline)
    {
		Ref<VulkanGraphicsPipeline> vkPipeline = pipeline;
		VK_BACKEND->CommandBindGraphicsPipeline(
			VK_CURRENT_COMMAND_BUFFER,
			vkPipeline->GetPipeline()
		);
    }
	void VulkanRendererAPI::BindComputePipeline(Ref<ComputePipeline> pipeline)
	{
		Ref<VulkanComputePipeline> vkPipeline = pipeline;
		VK_BACKEND->CommandBindComputePipeline(
			VK_CURRENT_COMMAND_BUFFER,
			vkPipeline->GetPipeline()
		);
	}
	void VulkanRendererAPI::BindPushConstants(Ref<Shader> shader, uint32_t firstIndex, const uint8_t* values, uint64_t size)
	{
		Ref<VulkanShader> vkShader = shader;
		VK_BACKEND->CommandBindPushConstants(
			VK_CURRENT_COMMAND_BUFFER,
			vkShader->GetShaderInfo(),
			firstIndex, values, size
		);
	}
	void VulkanRendererAPI::BindDescriptorSet(Ref<Shader> shader, Ref<DescriptorSet> set, uint32_t setIndex)
	{
		Ref<VulkanShader> vkShader = shader;
		Ref<VulkanDescriptorSet> vkSet = set;
		VK_BACKEND->CommandBindDescriptorSet(
			VK_CURRENT_COMMAND_BUFFER,
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
			VK_CURRENT_COMMAND_BUFFER,
			{ viewport }
		);
	}
	void VulkanRendererAPI::SetScissor(uint32_t width, uint32_t height, int32_t offsetX, int32_t offsetY)
	{
		VkRect2D scissor{};
		scissor.extent = { width,height };
		scissor.offset = { offsetX,offsetY };
		
		VK_BACKEND->CommandSetScissor(
			VK_CURRENT_COMMAND_BUFFER,
			{ scissor }
		);
	}
	void VulkanRendererAPI::SetBlendConstants(const float color[4])
	{
		VK_BACKEND->CommandSetBlendConstants(
			VK_CURRENT_COMMAND_BUFFER,
			color
		);
	}
	void VulkanRendererAPI::SetLineWidth(float width)
	{
		VK_BACKEND->CommandSetLineWidth(
			VK_CURRENT_COMMAND_BUFFER, 
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
			VK_CURRENT_COMMAND_BUFFER,
			std::initializer_list<VkClearAttachment>(vkClears, vkClears + clearValues.size()),
			std::initializer_list<VkClearRect>(vkRects, vkRects + clearValues.size())
		);
	}
	void VulkanRendererAPI::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset)
	{
		VK_BACKEND->CommandDraw(
			VK_CURRENT_COMMAND_BUFFER,
			vertexCount, instanceCount, vertexOffset, instanceOffset
		);
	}
	void VulkanRendererAPI::DrawIndexed(uint32_t indexCount, uint32_t vertexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset)
	{
		VK_BACKEND->CommandDrawIndexed(
			VK_CURRENT_COMMAND_BUFFER,
			indexCount,vertexCount,instanceCount,indexOffset,vertexOffset,instanceOffset
		);
	}
	void VulkanRendererAPI::Dispatch(uint32_t x, uint32_t y, uint32_t z)
	{
		VK_BACKEND->CommandDispatch(
			VK_CURRENT_COMMAND_BUFFER,
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
			VK_CURRENT_COMMAND_BUFFER,
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
			VK_CURRENT_COMMAND_BUFFER,
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
			VK_CURRENT_COMMAND_BUFFER,
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
			VK_CURRENT_COMMAND_BUFFER,
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
    }
}
