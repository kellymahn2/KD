#pragma once



#include "VulkanDefinitions.h"


namespace Kaidel {
	namespace Vulkan {
		

		class VulkanSwapchain : public IRCCounter<false> {
		public:

			VulkanSwapchain(const VulkanSwapchainSpecification& specification);
			~VulkanSwapchain();

			auto& GetSpecification()const { return m_Specification; }

			void Resize(uint32_t width,uint32_t height);
			void Present(uint32_t imageIndex);
			uint32_t AcquireImage(uint32_t lastImageIndex);

			auto GetSwapchain()const { return m_Swapchain; }

			auto& GetFrames() const{ return m_Frames; }

			uint32_t GetLastAcquiredImage()const { return m_LastAcquiredImage; }
			VkRenderPass GetSwapchainRenderPass()const { return m_RenderPass; }


		private:
			void DestroyCurrentSwapchain();
			void Invalidate();
		private:
			uint32_t m_LastAcquiredImage = -1;

			VkRenderPass m_RenderPass;

			VulkanSwapchainSpecification m_Specification;
			std::vector<SwapchainFrame> m_Frames;
			VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		};
	}
}
