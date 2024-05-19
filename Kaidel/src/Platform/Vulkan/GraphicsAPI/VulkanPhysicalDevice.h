#pragma once

#include "VulkanBase.h"
#include "VulkanDefinitions.h"
#include "VulkanQueueManager.h"

namespace Kaidel {
	namespace Vulkan {
		namespace Utils {
			struct PhysicalDeviceChooseResult {
				VkPhysicalDevice PhysicalDevice;
				QueueFamilyIndices Indices;
				VulkanQueueManager QueueManager;
			};


			std::vector<VkQueueFamilyProperties> QuerySupportedPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice);
			bool QueryPhysicalDeviceQueueFamilyPresentSupport(VkPhysicalDevice physicalDevice, uint32_t familyIndex, VkSurfaceKHR surface);
			QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
			std::vector<VkExtensionProperties> QuerySupportedPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice);
			bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, const PhysicalDeviceSpecification& physicalDeviceSpecification);

			PhysicalDeviceChooseResult ChoosePhysicalDevice(VkInstance instance, const std::vector<VulkanQueueGroupSpecification>& queueGroups,const PhysicalDeviceSpecification& physicalDeviceSpecification);


		}
	}
}
