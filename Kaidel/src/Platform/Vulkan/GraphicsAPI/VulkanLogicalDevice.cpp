#include "KDpch.h"
#include "VulkanLogicalDevice.h"



namespace Kaidel {
	namespace Vulkan {
		namespace Utils {
			LogicalDeviceCreateResult CreateLogicalDevice(VkPhysicalDevice physicalDevice, VulkanQueueManager& queueManager, const LogicalDeviceSpecification& logicalDeviceSpecification) {
				LogicalDeviceCreateResult result{};

				KD_CORE_ASSERT(physicalDevice);

				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

				for (auto& [name,queue] : queueManager) {
					if (auto ptr = std::find_if(queueCreateInfos.begin(), queueCreateInfos.end(), [&queue](const VkDeviceQueueCreateInfo& x) {return x.queueFamilyIndex == queue.FamilyIndex;});
						ptr != queueCreateInfos.end())
					{
						++ptr->queueCount;
					}
					else {
						VK_STRUCT(VkDeviceQueueCreateInfo, queueInfo,VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
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

				VK_STRUCT(VkDeviceCreateInfo, deviceInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
				deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
				deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
				deviceInfo.enabledExtensionCount = (uint32_t)logicalDeviceSpecification.Extensions.size();
				deviceInfo.ppEnabledExtensionNames = logicalDeviceSpecification.Extensions.data();
				deviceInfo.enabledLayerCount = (uint32_t)logicalDeviceSpecification.Layers.size();
				deviceInfo.ppEnabledLayerNames = logicalDeviceSpecification.Layers.data();
				deviceInfo.pEnabledFeatures = &logicalDeviceSpecification.Features;

				VK_ASSERT(vkCreateDevice(physicalDevice, &deviceInfo, VK_ALLOCATOR_PTR, &result.LogicalDevice));
				for (auto& queueInfo : queueCreateInfos) {
					delete[] queueInfo.pQueuePriorities;
				}

				for (auto& [name, queue] : queueManager) {
					vkGetDeviceQueue(result.LogicalDevice, queue.FamilyIndex, queue.QueueIndex, &queue.Queue);
				}


				/*VK_STRUCT(VkDeviceCreateInfo, deviceInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
				deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
				deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
				deviceInfo.enabledExtensionCount = (uint32_t)logicalDeviceSpecification.Extensions.size();
				deviceInfo.ppEnabledExtensionNames = logicalDeviceSpecification.Extensions.data();
				deviceInfo.enabledLayerCount = (uint32_t)logicalDeviceSpecification.Layers.size();
				deviceInfo.ppEnabledLayerNames = logicalDeviceSpecification.Layers.data();
				deviceInfo.pEnabledFeatures = &logicalDeviceSpecification.Features;

				VK_ASSERT(vkCreateDevice(physicalDevice, &deviceInfo, VK_ALLOCATOR_PTR, &result.LogicalDevice));

				vkGetDeviceQueue(result.LogicalDevice, familyIndices.GraphicsQueueFamilyIndex.value(), queueIndex[graphicsQueueArrayIndex], &result.Queues.GraphicsQueue);
				vkGetDeviceQueue(result.LogicalDevice, familyIndices.PresentQueueFamilyIndex.value(), queueIndex[presentQueueArrayIndex], &result.Queues.PresentQueue);*/
				///vkGetDeviceQueue(result.LogicalDevice, familyIndices.TransferQueueFamilyIndex.value(), queueIndex[transferQueueArrayIndex], &result.Queues.TransferQueue);


				return result;
			}
		}
	}
}
