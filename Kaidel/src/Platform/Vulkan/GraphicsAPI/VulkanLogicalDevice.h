#pragma once

#include "VulkanBase.h"
#include "VulkanDefinitions.h"
#include "VulkanQueueManager.h"

namespace Kaidel {
	namespace Vulkan {
		namespace Utils {
			struct LogicalDeviceCreateResult {
				VkDevice LogicalDevice;
				DeviceQueues Queues;
			};

			LogicalDeviceCreateResult CreateLogicalDevice(VkPhysicalDevice physicalDevice, VulkanQueueManager& queueManager, const LogicalDeviceSpecification& logicalDeviceSpecification);
		}
	}
}
