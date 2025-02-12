#pragma once

#include "Kaidel/Renderer/RendererAPI.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"

namespace Kaidel {


	class RenderCommand
	{
	public:
		static void Init()
		{
			s_RendererAPI->Init();
		}

		//static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		//{
		//	s_RendererAPI->SetViewport(x, y, width, height);
		//}

		static void DeviceWaitIdle() {
			s_RendererAPI->DeviceWaitIdle();
		}

		static void BeginRenderPass(Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer,
			std::initializer_list<AttachmentClearValue> clearValues){
			s_RendererAPI->BeginRenderPass(renderPass, framebuffer, clearValues);
		}
		static void EndRenderPass(){
			s_RendererAPI->EndRenderPass();
		}
		static void NextSubpass(){
			s_RendererAPI->NextSubpass();
		}

		static void BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> buffers, std::initializer_list<uint64_t> offsets) {
			s_RendererAPI->BindVertexBuffers(buffers, offsets);
		}
		static void BindIndexBuffer(Ref<IndexBuffer> buffer, uint64_t offset) {
			s_RendererAPI->BindIndexBuffer(buffer, offset);
		}

		static void BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline) {
			s_RendererAPI->BindGraphicsPipeline(pipeline);
		}
		static void BindComputePipeline(Ref<ComputePipeline> pipeline) {
			s_RendererAPI->BindComputePipeline(pipeline);
		}
		
		static void BindPushConstants(Ref<Shader> shader, uint64_t offset,const uint8_t* ptr,uint64_t size) {
			s_RendererAPI->BindPushConstants(shader, (uint32_t)offset,ptr,size);
		}

		template<typename T,typename... Args>
		static void BindPushConstants(Ref<Shader> shader, uint64_t offset, T&& value, Args&&... args) {
			s_RendererAPI->BindPushConstants(shader, (uint32_t)offset, (const uint8_t*)&value, sizeof(T));
			BindPushConstants(shader, offset + sizeof(T), std::forward<Args>(args)...);
		}
		template<typename T>
		static void BindPushConstants(Ref<Shader> shader, uint64_t offset, T&& value) {
			s_RendererAPI->BindPushConstants(shader, (uint32_t)offset, (const uint8_t*)&value, sizeof(T));
		}

		static void BindDescriptorSet(Ref<Shader> shader, Ref<DescriptorSet> set, uint32_t setIndex) {
			s_RendererAPI->BindDescriptorSet(shader, set, setIndex);
		}

		static void SetViewport(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) {
			s_RendererAPI->SetViewport(width, height, offsetX, offsetY);
		}
		static void SetScissor(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) {
			s_RendererAPI->SetScissor(width, height, offsetX, offsetY);
		}
		static void SetBlendConstants(const float color[4]) {
			s_RendererAPI->SetBlendConstants(color);
		}
		static void SetLineWidth(float width) {
			s_RendererAPI->SetLineWidth(width);
		}

		static void ClearAttachments(std::initializer_list<AttachmentClear> clearValues, std::initializer_list<ClearRect> rects) {
			s_RendererAPI->ClearAttachments(clearValues, rects);
		}

		static void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset) {
			s_RendererAPI->Draw(vertexCount, instanceCount, vertexOffset, instanceOffset);
		}
		static void DrawIndexed(uint32_t indexCount, uint32_t vertexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset) {
			s_RendererAPI->DrawIndexed(indexCount, vertexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
		}

		static void Dispatch(uint32_t x, uint32_t y, uint32_t z) {
			s_RendererAPI->Dispatch(x, y, z);
		}

		//void ClearBuffer(Ref<VertexBuffer> buffer, uint64_t offset, uint64_t size);
		//void CopyBuffer(In<BufferInfo> srcBuffer, In<BufferInfo> dstBuffer, std::initializer_list<VkBufferCopy> regions);
		/*static void CopyTexture(Ref<Texture> srcTexture,
			Ref<Texture> dstTexture, std::initializer_list<VkImageCopy> regions){

		}*/

		static void CopyTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip,
			Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip) {
			s_RendererAPI->CopyTexture(srcTexture, srcLayer, srcMip, dstTexture, dstLayer, dstMip);
		}

		static void ResolveTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip,
			Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip) {
			s_RendererAPI->ResolveTexture(srcTexture, srcLayer, srcMip, dstTexture, dstLayer, dstMip);
		}
		static void ClearColorTexture(Ref<Texture> texture,
			const AttachmentColorClearValue& clear) {
			s_RendererAPI->ClearColorTexture(texture, clear);
		}
		//void CopyBufferToTexture(In<BufferInfo> srcBuffer, In<TextureInfo> dstTexture, VkImageLayout dstLayout,
		//	std::initializer_list<VkBufferImageCopy> regions);
		//void CopyTextureToBuffer(In<TextureInfo> srcTexture, VkImageLayout srcLayout, In<BufferInfo> dstBuffer,
		//	std::initializer_list<VkBufferImageCopy> regions);

		static void PipelineBarrier(
			PipelineStages srcStages,
			PipelineStages dstStages,
			std::initializer_list<MemoryBarrier> memoryBarriers,
			std::initializer_list<BufferMemoryBarrier> bufferBarriers,
			std::initializer_list<ImageMemoryBarrier> textureBarriers) {
			s_RendererAPI->PipelineBarrier(srcStages, dstStages, memoryBarriers, bufferBarriers, textureBarriers);
		}
		
		static Scope<RendererAPI>& Get() { return s_RendererAPI; }

	private:
		static Scope<RendererAPI> s_RendererAPI;
	};

}
