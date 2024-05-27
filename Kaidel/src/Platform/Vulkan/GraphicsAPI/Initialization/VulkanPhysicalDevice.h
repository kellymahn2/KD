#pragma once

#include "Platform/Vulkan/GraphicsAPI/VulkanBase.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanDefinitions.h"
#include "VulkanQueueManager.h"

namespace Kaidel {

	namespace Utils {
		struct PhysicalDeviceChooseResult {
			VkPhysicalDevice PhysicalDevice;
			Vulkan::QueueFamilyIndices Indices;
			Vulkan::VulkanQueueManager QueueManager;
		};


		std::vector<VkQueueFamilyProperties> QuerySupportedPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice);
		bool QueryPhysicalDeviceQueueFamilyPresentSupport(VkPhysicalDevice physicalDevice, uint32_t familyIndex, VkSurfaceKHR surface);
		std::vector<VkExtensionProperties> QuerySupportedPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice);
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, const Vulkan::PhysicalDeviceSpecification& physicalDeviceSpecification);

		PhysicalDeviceChooseResult ChoosePhysicalDevice(VkInstance instance, const std::vector<Vulkan::VulkanQueueGroupSpecification>& queueGroups, const Vulkan::PhysicalDeviceSpecification& physicalDeviceSpecification);


	}


}
