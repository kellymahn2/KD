#pragma once

#include "VulkanPhysicalDevice.h"
#include <glad/vulkan.h>


namespace Kaidel {

	struct VulkanQueue {
		VkQueue Queue;
		uint32_t QueueIndexInsideFamily;
		uint32_t QueueFamilyIndex;
	};

	class VulkanLogicalDevice{
	public:

		const auto& GetVulkanQueue(const std::string& name)const {
			KD_CORE_ASSERT(QueueExists(name));
			return m_Queues.at(name);
		}

		VkDevice GetLogicalDevice()const { return m_LogicalDevice; }


		static void Init(const VulkanPhysicalDevice& physicalDevice,const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& enabledFeatures);
		static void Shutdown();

		static VulkanLogicalDevice& Get() { return *s_VulkanLogicalDevice; }


	private:

		bool QueueExists(const std::string& name)const {
			return m_Queues.find(name) != m_Queues.end();
		}
		
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		std::unordered_map<std::string, VulkanQueue> m_Queues;

		static Scope<VulkanLogicalDevice> s_VulkanLogicalDevice;

	};
}
