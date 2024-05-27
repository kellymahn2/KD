#pragma once


#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "PerFrameResource.h"

namespace Kaidel {

	namespace Vulkan {


		class VulkanFramebuffer : public Framebuffer {
		public:

			VulkanFramebuffer(const FramebufferSpecification& specification);
			~VulkanFramebuffer();

			void Bind() override;

			void Unbind() override;

			void Resize(uint32_t width, uint32_t height) override;

			void Resample(uint32_t newSampleCount) override;

			void ClearAttachment(uint32_t attachmentIndex, const void* colors) override;

			void ClearDepthAttachment(float value) override;

			uint64_t GetColorAttachmentRendererID(uint32_t index) const override { return (uint64_t)m_FramebufferResource->ColorAttachments[index].ImageView; }

			FramebufferAttachmentHandle GetAttachmentHandle(uint32_t index) const override;

			FramebufferImageHandle GetImageHandle(uint64_t index) const override;

			const FramebufferSpecification& GetSpecification() const override { return m_Specification; }


			void ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) override;

			Ref<RenderPass> GetRenderPass() const override { return m_RenderPass; }


			VkFramebuffer GetFramebuffer()const { return (*m_FramebufferResource).Framebuffer; };
		private:
			struct FramebufferImage {
				VkImage Image = VK_NULL_HANDLE;
				VkDeviceMemory Memory = VK_NULL_HANDLE;
				VkImageView ImageView = VK_NULL_HANDLE;
			};

			struct FramebufferFrameResource {
				VkFramebuffer Framebuffer = VK_NULL_HANDLE;
				std::vector<FramebufferImage> ColorAttachments;
				FramebufferImage DepthAttachment;
			};
		private:

			void Invalidate();

			void DeleteOldBuffers();

			void CreateRenderPass();

			bool HasDepthAttachment();

			void DestroyAttachment(FramebufferImage& image);

		private:

			PerFrameResource<FramebufferFrameResource> m_FramebufferResource;

			std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
			FramebufferTextureSpecification m_DepthAttachmentSpecification = TextureFormat::None;

			FramebufferSpecification m_Specification;

			Ref<RenderPass> m_RenderPass;

			// Inherited via Framebuffer

		};

	}


}
