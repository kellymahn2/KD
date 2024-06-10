				#pragma once


#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "PerFrameResource.h"
#include "VulkanImage.h"

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

			uint64_t GetColorAttachmentRendererID(uint32_t index) const override { return 0; }
			Image* GetImage(uint32_t index)override { return m_FramebufferResource->ColorAttachments[index].Image; }


			FramebufferAttachmentHandle GetAttachmentHandle(uint32_t index) const override;

			FramebufferImageHandle GetImageHandle(uint64_t index) const override;

			const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

			uint32_t GetWidth()const override { return m_FramebufferResource->FramebufferWidth; }
			uint32_t GetHeight()const override { return m_FramebufferResource->FramebufferHeight; }

			void ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) override;

			Ref<RenderPass> GetRenderPass() const override { return m_RenderPass; }

			VkFramebuffer GetFramebuffer()const { return m_FramebufferResource->Framebuffer; };
		private:
			struct FramebufferImage {
				VulkanFramebufferImage* Image;
			};

			struct FramebufferFrameResource {
				VkFramebuffer Framebuffer = VK_NULL_HANDLE;
				std::vector<FramebufferImage> ColorAttachments;
				FramebufferImage DepthAttachment;

				uint32_t FramebufferWidth, FramebufferHeight;
			};
		private:

			void Invalidate();

			void DeleteOldBuffers();

			void CreateRenderPass();
			void CreateDescriptorPool();
			void CreateDescriptorSetLayout();

			bool HasDepthAttachment();

			void DestroyAttachment(FramebufferImage& image);

			void Invalidate(FramebufferFrameResource& resource);
			void DeleteFramebufferFrameResource(FramebufferFrameResource& resource);

		private:
			
			VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
			VkDescriptorSetLayout m_SetLayout = VK_NULL_HANDLE;


			PerFrameResource<FramebufferFrameResource> m_FramebufferResource;

			std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
			FramebufferTextureSpecification m_DepthAttachmentSpecification = TextureFormat::None;

			FramebufferSpecification m_Specification;

			Ref<RenderPass> m_RenderPass;

			// Inherited via Framebuffer

		};

	}


}
