#include "KDpch.h"
#include "VulkanLogicalDevice.h"
#include "VulkanInstance.h"


namespace Kaidel {

	static VkDeviceQueueCreateInfo* GetCreateInfoWithFamilyIndex(std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, uint32_t index) {
		for (auto& info : queueCreateInfos) {
			if (info.queueFamilyIndex == index)
				return &info;
		}
		return nullptr;
	}


	static std::vector<const char*> TurnToCStrings(const std::vector<std::string>& strs) {
		std::vector<const char*> vec;
		for (auto& str : strs)
			vec.push_back(str.c_str());
		return vec;
	}


	Scope<VulkanLogicalDevice> VulkanLogicalDevice::s_VulkanLogicalDevice;
	
	void VulkanLogicalDevice::Init(const VulkanPhysicalDevice& physicalDevice, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& enabledFeatures)
	{


		s_VulkanLogicalDevice = CreateScope<VulkanLogicalDevice>();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;


		for (auto& [name, familyIndex] : physicalDevice.GetQueueFamilyMap()) {

			{
				VkDeviceQueueCreateInfo* createInfo = GetCreateInfoWithFamilyIndex(queueCreateInfos, familyIndex);
				//Family index existed
				if (createInfo) {
					if (physicalDevice.GetQueueSpecs().at(name).CreateSeperate) {
						uint32_t index = createInfo->queueCount;
						createInfo->queueCount++;
						VulkanQueue queue{};
						queue.QueueFamilyIndex = familyIndex;
						queue.QueueIndexInsideFamily = index;
						s_VulkanLogicalDevice->m_Queues[name] = queue;
					}
					else {
						VulkanQueue queue{};
						queue.QueueFamilyIndex = familyIndex;
						queue.QueueIndexInsideFamily = 0;
						s_VulkanLogicalDevice->m_Queues[name] = queue;
					}
				}
				else {
					VkDeviceQueueCreateInfo createInfo{};
					createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					createInfo.queueCount = 1;
					createInfo.queueFamilyIndex = familyIndex;

					VulkanQueue queue{};
					queue.QueueFamilyIndex = familyIndex;
					queue.QueueIndexInsideFamily = 0;
					if (physicalDevice.GetQueueSpecs().at(name).CreateSeperate) {
						queue.QueueIndexInsideFamily = 1;
						createInfo.queueCount = 2;
					}

					queueCreateInfos.push_back(createInfo);
					s_VulkanLogicalDevice->m_Queues[name] = queue;
				}

			}
		}

		for (auto& queueCreateInfo : queueCreateInfos) {

			float* priorities = new float[queueCreateInfo.queueCount];
			queueCreateInfo.pQueuePriorities = priorities;
			for (uint32_t i = 0; i < queueCreateInfo.queueCount; ++i) {
				priorities[i] = 1.0f;
			}
		}



		VkDeviceCreateInfo deviceCreateInfo{};

		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();


		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		deviceCreateInfo.enabledExtensionCount = extensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = extensions.data();


		std::vector<const char*> layersVec = TurnToCStrings(VulkanInstance::Get()->GetAcceptedValidationLayers());

		deviceCreateInfo.enabledLayerCount = layersVec.size();
		deviceCreateInfo.ppEnabledLayerNames = layersVec.data();

		KD_CORE_ASSERT(vkCreateDevice(physicalDevice.GetPhysicalDevice(), &deviceCreateInfo, nullptr, &s_VulkanLogicalDevice->m_LogicalDevice) == VK_SUCCESS);


		for (auto& queueCreateInfo : queueCreateInfos) {
			float* priorities = const_cast<float*>(queueCreateInfo.pQueuePriorities);
			delete[] priorities;
		}


		for (auto& [name, queue] : s_VulkanLogicalDevice->m_Queues) {
			vkGetDeviceQueue(s_VulkanLogicalDevice->m_LogicalDevice, queue.QueueFamilyIndex, queue.QueueIndexInsideFamily, &queue.Queue);
		}

	}

	void VulkanLogicalDevice::Shutdown()
	{
		vkDestroyDevice(s_VulkanLogicalDevice->m_LogicalDevice, nullptr);
	}

	
}
