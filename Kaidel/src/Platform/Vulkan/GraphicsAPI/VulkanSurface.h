#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"

namespace Kaidel {
	class VulkanSurface {
	public:
		VulkanSurface(void* window);
		~VulkanSurface();
		VkSurfaceKHR GetSurface()const { return m_Surface; }

	private:
		VkSurfaceKHR m_Surface;
	};
}
