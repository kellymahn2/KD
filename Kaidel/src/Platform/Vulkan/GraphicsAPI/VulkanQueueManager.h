#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"

namespace Kaidel {

	typedef int QueueCapability;

	enum QueueCapability_
	{
		QueueCapability_Graphics,
		QueueCapability_Compute,
		QueueCapability_Transfer,
		QueueCapability_Present,

		QueueCapability_General = QueueCapability_Graphics | QueueCapability_Transfer | QueueCapability_Compute
	};

	struct VulkanQueue {
		VkQueue Queue;
		uint32_t FamilyIndex;
		uint32_t QueueIndex;

		std::mutex SubmitMutex;
		TLS<VkFence> SingleSubmitFence;

		operator VkQueue()const { return Queue; }
	};


	static std::unordered_map<std::string, VulkanQueue> 
		GetRequestedQueues(const std::unordered_map<std::string, QueueCapability>& requested, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		uint32_t supportedPhysicalDeviceQueueFamilyCount = 0;

		std::vector<VkQueueFamilyProperties> supportedQueueFamilies;
		
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, nullptr);

		supportedQueueFamilies.resize(supportedPhysicalDeviceQueueFamilyCount);
		
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, supportedQueueFamilies.data());

		std::unordered_map<std::string, VulkanQueue> queues;

		std::unordered_map<uint32_t, uint32_t> acceptedQueueFamilies;

		for (const auto& [name, capability] : requested)
		{
			bool found = false;
		
			for (uint32_t i = 0; i < supportedQueueFamilies.size(); ++i)
			{
				const auto& family = supportedQueueFamilies[i];
				
				if (family.queueCount == acceptedQueueFamilies[i])
				{
					continue;
				}

				if ((capability & QueueCapability_Graphics) && !(family.queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					continue;
				}

				if ((capability & QueueCapability_Compute) && !(family.queueFlags & VK_QUEUE_COMPUTE_BIT))
				{
					continue;
				}

				if ((capability & QueueCapability_Transfer) && (!family.queueFlags & VK_QUEUE_TRANSFER_BIT))
				{
					continue;
				}

				if ((capability & QueueCapability_Present))
				{
					VkBool32 supported;
					VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supported));

					if (!supported)
						continue;
				}

				VulkanQueue& queue = queues[name];

				queue.FamilyIndex = i;
				queue.QueueIndex = acceptedQueueFamilies[i]++;
				
				found = true;

				break;
			}

			if (!found)
			{
				return {};
			}
		}

		return queues;
	}
}
