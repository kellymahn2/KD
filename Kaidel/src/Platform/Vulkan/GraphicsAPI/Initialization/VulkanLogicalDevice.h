#pragma once

#include "Platform/Vulkan/GraphicsAPI/VulkanBase.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanDefinitions.h"
#include "VulkanQueueManager.h"

namespace Kaidel {
	
	namespace Utils {
		struct LogicalDeviceCreateResult {
			VkDevice LogicalDevice;
			Vulkan::DeviceQueues Queues;
		};

		LogicalDeviceCreateResult CreateLogicalDevice(VkPhysicalDevice physicalDevice, Vulkan::VulkanQueueManager& queueManager, const Vulkan::LogicalDeviceSpecification& logicalDeviceSpecification);
	}

}
