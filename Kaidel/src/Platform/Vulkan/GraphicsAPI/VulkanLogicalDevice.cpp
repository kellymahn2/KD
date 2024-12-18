#include "KDpch.h"
#include "VulkanLogicalDevice.h"



namespace Kaidel {

	VulkanLogicalDevice::VulkanLogicalDevice(VulkanPhysicalDevice& physicalDevice, const std::vector<const char*>& extensions,
												const std::vector<const char*>& layers, const VkPhysicalDeviceFeatures& features, void* additionalFeatures)
		:m_Device(VK_NULL_HANDLE)
	{

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		for (auto& [name, queue] : physicalDevice.GetQueueManager()) {
			if (auto ptr = std::find_if(queueCreateInfos.begin(), queueCreateInfos.end(), [&queue](const VkDeviceQueueCreateInfo& x) {return x.queueFamilyIndex == queue.FamilyIndex; });
				ptr != queueCreateInfos.end())
			{
				ptr->queueCount = std::max(ptr->queueCount, queue.QueueIndex + 1);
			}
			else {
				VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
				queueInfo.queueCount = 1;
				queueInfo.queueFamilyIndex = queue.FamilyIndex;
				queueCreateInfos.push_back(queueInfo);
			}
		}


		for (auto& queueInfo : queueCreateInfos) {
			float* priorities = new float[queueInfo.queueCount];
			for (uint32_t i = 0; i < queueInfo.queueCount; ++i) {
				priorities[i] = 1.0f;
			}
			queueInfo.pQueuePriorities = priorities;
		}

		VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
		deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
		deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceInfo.enabledExtensionCount = (uint32_t)extensions.size();
		deviceInfo.ppEnabledExtensionNames = extensions.data();
		deviceInfo.enabledLayerCount = (uint32_t)layers.size();
		deviceInfo.ppEnabledLayerNames = layers.data();
		deviceInfo.pEnabledFeatures = &features;
		deviceInfo.pNext = additionalFeatures;

		VK_ASSERT(vkCreateDevice(physicalDevice.GetDevice(), &deviceInfo, nullptr, &m_Device));
		for (auto& queueInfo : queueCreateInfos) {
			delete[] queueInfo.pQueuePriorities;
		}

		for (auto& [name, queue] : physicalDevice.GetQueueManager()) {
			vkGetDeviceQueue(m_Device, queue.FamilyIndex, queue.QueueIndex, &queue.Queue);
		}

	}

	VulkanLogicalDevice::~VulkanLogicalDevice() {
		vkDestroyDevice(m_Device, nullptr);
	}
}
