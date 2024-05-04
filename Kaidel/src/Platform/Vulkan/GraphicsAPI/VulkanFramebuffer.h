#pragma once


#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"


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

			uint64_t GetColorAttachmentRendererID(uint32_t index) const override;

			FramebufferAttachmentHandle GetAttachmentHandle(uint32_t index) const override;

			FramebufferImageHandle GetImageHandle(uint64_t index) const override;

			const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

			void BindColorAttachmentToSlot(uint32_t attachmentIndex, uint32_t slot) override;

			void BindColorAttachmentToImageSlot(uint32_t attachmnetIndex, uint32_t slot, ImageBindingMode bindingMode) override;

			void BindDepthAttachmentToSlot(uint32_t slot) override;

			void CopyColorAttachment(uint32_t dstAttachmentIndex, uint32_t srcAttachmentIndex, Ref<Framebuffer> src) override;

			void CopyDepthAttachment(Ref<Framebuffer> src) override;

			void EnableColorAttachment(uint32_t attachmentIndex) override;

			void DisableColorAttachment(uint32_t attachmentIndex) override;

			void ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) override;

			void SetAttachment(const TextureHandle& handle, uint32_t index) override;

			void SetAttachment(const TextureArrayHandle& handle, uint32_t index) override;

			void SetDepthAttachment(const TextureHandle& handle) override;

			void SetDepthAttachment(const TextureArrayHandle& handle) override;


		private:

			void Invalidate();

		private:

			struct FramebufferImage {
				VkImage Image = VK_NULL_HANDLE;
				VkImageView ImageView = VK_NULL_HANDLE;
			};

			VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
			std::vector<FramebufferImage> m_ColorAttachments;
			FramebufferImage m_DepthAttachment;


			std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
			FramebufferTextureSpecification m_DepthAttachmentSpecification = TextureFormat::None;

			FramebufferSpecification m_Specification;

		};

	}


}
