#pragma once



#include "VulkanBase.h"


namespace Kaidel {
	namespace Vulkan {
		
		


		class VulkanSwapchain : public IRCCounter<false> {
		public:

			VulkanSwapchain(const VulkanSwapchainSpecification& specification);
			~VulkanSwapchain();



			auto& GetSpecification()const { return m_Specification; }

			void Resize(uint32_t width,uint32_t height);
			void Present(uint32_t imageIndex);
			uint32_t AcquireImage();

		private:
			void DestroyCurrentSwapchain();
			void Invalidate();
		private:
			VulkanSwapchainSpecification m_Specification;
			std::vector<SwapchainFrame> m_Frames;
			VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		};
	}
}
