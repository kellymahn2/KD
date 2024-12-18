#pragma once
#pragma once
#include "Platform/Vulkan/VulkanDefinitions.h"
#include "VulkanInstance.h"
#include "VulkanQueueManager.h"
namespace Kaidel {
	class VulkanPhysicalDevice {
	public:

		VulkanPhysicalDevice(const VulkanInstance& instance, const std::vector<VulkanQueueGroupSpecification>& specs);

		~VulkanPhysicalDevice() = default;
		VulkanQueueManager& GetQueueManager() { return m_QueueManager; }
		VulkanQueue& GetQueue(const std::string& name) { return m_QueueManager[name]; }

		VkPhysicalDevice GetDevice()const { return m_Device; }
		const VkPhysicalDeviceProperties& GetProperties()const { return *m_Properties; }


	private:
		std::vector<VkPhysicalDevice> GetDevices(const VulkanInstance& instance);
	private:
		VkPhysicalDevice m_Device;
		VulkanQueueManager m_QueueManager;
		Scope<VkPhysicalDeviceProperties> m_Properties;
	};



}


