#pragma once
#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "VulkanInstance.h"
#include "VulkanQueueManager.h"
#include "VulkanSurface.h"

namespace Kaidel {
	class VulkanPhysicalDevice {
	public:

		VulkanPhysicalDevice(const VulkanInstance& instance, const VulkanSurface& surface, const std::unordered_map<std::string, QueueCapability>& requested);

		~VulkanPhysicalDevice() = default;
		VulkanQueue& GetQueue(const std::string& name) { return m_Queues[name]; }

		auto& GetQueues() { return m_Queues; }

		VkPhysicalDevice GetDevice()const { return m_Device; }
		const VkPhysicalDeviceProperties& GetProperties()const { return *m_Properties; }


	private:
		std::vector<VkPhysicalDevice> GetDevices(const VulkanInstance& instance);
	private:
		VkPhysicalDevice m_Device;
		Scope<VkPhysicalDeviceProperties> m_Properties;
		std::unordered_map<std::string, VulkanQueue> m_Queues;
	};



}


