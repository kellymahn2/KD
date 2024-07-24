#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "VulkanPhysicalDevice.h"



namespace Kaidel {
	class VulkanLogicalDevice {
	public:
		VulkanLogicalDevice(VulkanPhysicalDevice& physicalDevice, const std::vector<const char*>& extensions,
			const std::vector<const char*>& layers, const VkPhysicalDeviceFeatures& features, void* additionalFeatures = nullptr);
		~VulkanLogicalDevice();
		VkDevice GetDevice()const { return m_Device; }
	private:
		VkDevice m_Device;
	};

}
