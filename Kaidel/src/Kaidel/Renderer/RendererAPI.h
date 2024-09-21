#pragma once
#include "Settings.h"

#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"

#include "Kaidel/Renderer/GraphicsAPI/VertexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/IndexBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsPipeline.h"
#include "Kaidel/Renderer/GraphicsAPI/ComputePipeline.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include <glm/glm.hpp>

namespace Kaidel {
	
	struct MemoryBarrier {
		AccessFlags Src = 0;
		AccessFlags Dst = 0;
	};

	struct ImageMemoryBarrier {
		Ref<Texture> Image;
		AccessFlags Src = 0;
		AccessFlags Dst = 0;
		ImageLayout OldLayout = ImageLayout::None;
		ImageLayout NewLayout = ImageLayout::None;
		TextureSubresourceRegion Subresource;

		ImageMemoryBarrier() = default;
		
		//Used for layout transitions for all layers and all mips.
		ImageMemoryBarrier(Ref<Texture> image, ImageLayout newLayout, AccessFlags src, AccessFlags dst)
			:Image(image), Src(src), Dst(dst), NewLayout(newLayout)
		{
			OldLayout = image->GetTextureSpecification().Layout;
			Subresource.LayerCount = image->GetTextureSpecification().Layers;
			Subresource.MipCount = image->GetTextureSpecification().Mips;
			Subresource.StartLayer = 0;
			Subresource.StarMip = 0;
		}
	};

	struct BufferMemoryBarrier {
		Ref<Buffer> Buffer;
		AccessFlags Src = 0;
		AccessFlags Dst = 0;
		uint64_t Offset = 0;
		uint64_t Size = -1;
	};

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1,DirectX = 2,Vulkan = 3
		};
	public:
		virtual ~RendererAPI() = default;

		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void Submit(std::function<void()>&& func) = 0;
		virtual void DeviceWaitIdle() = 0;
		virtual void Shutdown() = 0;
		
		//Commands
		virtual void BeginRenderPass(Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer,
			std::initializer_list<AttachmentClearValue> clearValues) = 0;
		virtual void EndRenderPass() = 0;
		virtual void NextSubpass() = 0;

		virtual void BindVertexBuffers(std::initializer_list<Ref<VertexBuffer>> buffers, std::initializer_list<uint64_t> offsets) = 0;
		virtual void BindIndexBuffer(Ref<IndexBuffer> buffer, uint64_t offset) = 0;

		virtual void BindGraphicsPipeline(Ref<GraphicsPipeline> pipeline) = 0;
		virtual void BindComputePipeline(Ref<ComputePipeline> pipeline) = 0;
		virtual void BindPushConstants(Ref<Shader> shader, uint32_t firstIndex, const uint8_t* values, uint64_t size) = 0;
		virtual void BindDescriptorSet(Ref<Shader> shader, Ref<DescriptorSet> set, uint32_t setIndex) = 0;

		virtual void SetViewport(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) = 0;
		virtual void SetScissor(uint32_t width, uint32_t height, int32_t offsetX = 0, int32_t offsetY = 0) = 0;
		virtual void SetBlendConstants(const float color[4]) = 0;
		virtual void SetLineWidth(float width) = 0;

		virtual void ClearAttachments(std::initializer_list<AttachmentClear> clearValues, std::initializer_list<ClearRect> rects) = 0;

		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t vertexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset) = 0;

		virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

		//void ClearBuffer(Ref<VertexBuffer> buffer, uint64_t offset, uint64_t size);
		//void CopyBuffer(In<BufferInfo> srcBuffer, In<BufferInfo> dstBuffer, std::initializer_list<VkBufferCopy> regions);
		/*
		*/
		virtual void CopyTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip,
			Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip) = 0;
		virtual void ResolveTexture(Ref<Texture> srcTexture, uint32_t srcLayer, uint32_t srcMip,
			Ref<Texture> dstTexture, uint32_t dstLayer, uint32_t dstMip) = 0;
		virtual void ClearColorTexture(Ref<Texture> texture,
			const AttachmentColorClearValue& clear) = 0;
		//void CopyBufferToTexture(In<BufferInfo> srcBuffer, In<TextureInfo> dstTexture, VkImageLayout dstLayout,
		//	std::initializer_list<VkBufferImageCopy> regions);
		//void CopyTextureToBuffer(In<TextureInfo> srcTexture, VkImageLayout srcLayout, In<BufferInfo> dstBuffer,
		//	std::initializer_list<VkBufferImageCopy> regions);

		virtual void PipelineBarrier(
			PipelineStages srcStages,
			PipelineStages dstStages,
			std::initializer_list<MemoryBarrier> memoryBarriers,
			std::initializer_list<BufferMemoryBarrier> bufferBarriers,
			std::initializer_list<ImageMemoryBarrier> textureBarriers) = 0;


		static RendererSettings& GetSettings() { return s_RendererSettings; }

		static API GetAPI() { return s_API; }
		static Scope<RendererAPI> Create();
	protected:
		static inline RendererSettings s_RendererSettings;
	private:
		static API s_API;
	};
}
