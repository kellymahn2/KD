#include "KDpch.h"
#include "VulkanRendererAPI.h"
#include "VulkanGraphicsContext.h"
#include "VulkanBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanRenderPass.h"

namespace Kaidel {
	namespace Vulkan {
		void VulkanRendererAPI::Init()
		{
			m_CurrentBoundBufferOffsets = std::vector<VkDeviceSize>(MaxVertexBuffersBoundAtOnce, 0);
			m_CurrentBoundBuffers = std::vector<VkBuffer>(MaxVertexBuffersBoundAtOnce, VK_NULL_HANDLE);
		}
		void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			((VulkanGraphicsContext*)Application::Get().GetWindow().GetContext().get())->OnResize(width, height);
		}
		void VulkanRendererAPI::GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height)
		{

		}
		void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
		{
			m_ClearValues = color;
		}
		void VulkanRendererAPI::Clear()
		{
		}
		void VulkanRendererAPI::Shutdown()
		{
		}
		void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
		{
		}
		void VulkanRendererAPI::DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount)
		{
		}
		void VulkanRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
		}
		void VulkanRendererAPI::DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
		}
		void VulkanRendererAPI::DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
		}
		void VulkanRendererAPI::SetLineWidth(float thickness)
		{
		}
		void VulkanRendererAPI::SetPointSize(float pixelSize)
		{
		}
		void VulkanRendererAPI::SetCullMode(CullMode cullMode)
		{
		}
		void VulkanRendererAPI::SetPatchVertexCount(uint32_t count)
		{
		}
		void VulkanRendererAPI::RenderFullScreenQuad(Ref<Shader> shader, uint32_t width, uint32_t height) const
		{
		}
		void VulkanRendererAPI::BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> vertexBuffers)
		{
			uint32_t i = 0;
			for (auto& vb : vertexBuffers) {
				m_CurrentBoundBuffers[i] = ((VulkanVertexBuffer*)vb.Get())->GetBuffer().Buffer;
				++i;
			}
			vkCmdBindVertexBuffers(VK_CONTEXT.GetGraphicsCommandBuffer(), 0, vertexBuffers.size(),m_CurrentBoundBuffers.data(),m_CurrentBoundBufferOffsets.data());
		}
		void VulkanRendererAPI::BindIndexBuffer(Ref<IndexBuffer> indexBuffer)
		{
			VkBuffer buffer = ((VulkanIndexBuffer*)indexBuffer.Get())->GetBuffer();
			vkCmdBindIndexBuffer(VK_CONTEXT.GetGraphicsCommandBuffer(), buffer, 0, VK_INDEX_TYPE_UINT32);
		}
		void VulkanRendererAPI::BeginRenderPass(Ref<Framebuffer> frameBuffer, Ref<RenderPass> renderPass)
		{
			VulkanRenderPass* vkRenderPass = (VulkanRenderPass*)renderPass.Get();
			VulkanFramebuffer* vkFramebuffer = (VulkanFramebuffer*)frameBuffer.Get();

			VK_STRUCT(VkRenderPassBeginInfo, beginInfo,VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			beginInfo.framebuffer = vkFramebuffer->GetFramebuffer();
			beginInfo.renderPass = vkRenderPass->GetRenderPass();
			beginInfo.pClearValues = vkRenderPass->GetClearColors().data();
			beginInfo.clearValueCount = (uint32_t)vkRenderPass->GetClearColors().size();
			VkRect2D renderArea{};
			renderArea.extent = { vkFramebuffer->GetSpecification().Width,vkFramebuffer->GetSpecification().Height};
			renderArea.offset = { 0,0 };
			beginInfo.renderArea = renderArea;

			vkCmdBeginRenderPass(VK_CONTEXT.GetGraphicsCommandBuffer(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
			m_BoundFramebuffer = frameBuffer;
		}
		void VulkanRendererAPI::EndRenderPass()
		{
			vkCmdEndRenderPass(VK_CONTEXT.GetGraphicsCommandBuffer());
		}
		void VulkanRendererAPI::BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline)
		{
			VulkanGraphicsPipeline* vkPipeline = (VulkanGraphicsPipeline*)pipeline.Get();
			VkDescriptorSet ubSet = vkPipeline->GetDescriptorSet();
			vkCmdBindDescriptorSets(VK_CONTEXT.GetGraphicsCommandBuffer(), 
										VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetLayout(), 0, 1, &ubSet,0,nullptr);
			VkViewport viewport{};
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = m_BoundFramebuffer->GetSpecification().Width;
			viewport.height = m_BoundFramebuffer->GetSpecification().Height;
			VkRect2D scissor{};
			scissor.offset = { 0,0 };
			scissor.extent = { (uint32_t)viewport.width,(uint32_t)viewport.height };
			vkCmdSetViewport(VK_CONTEXT.GetGraphicsCommandBuffer(), 0, 1,&viewport);
			vkCmdSetScissor(VK_CONTEXT.GetGraphicsCommandBuffer(), 0, 1, &scissor);
			vkCmdBindPipeline(VK_CONTEXT.GetGraphicsCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetPipeline());
		}
		void VulkanRendererAPI::Draw(uint32_t vertexCount, uint32_t firstVertexID)
		{
			vkCmdDraw(VK_CONTEXT.GetGraphicsCommandBuffer(), vertexCount, 1, firstVertexID, 0);
		}
		void VulkanRendererAPI::DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertexID)
		{
			vkCmdDraw(VK_CONTEXT.GetGraphicsCommandBuffer(), vertexCount, instanceCount, firstVertexID, 0);
		}
		void VulkanRendererAPI::DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
		{
			vkCmdDrawIndexed(VK_CONTEXT.GetGraphicsCommandBuffer(),indexCount,1,firstIndex,vertexOffset,0);
		}
		void VulkanRendererAPI::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset)
		{
			vkCmdDrawIndexed(VK_CONTEXT.GetGraphicsCommandBuffer(), indexCount, instanceCount, firstIndex, vertexOffset, 0);
		}
	}
}
