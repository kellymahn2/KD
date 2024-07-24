#include "KDpch.h"
#include "VulkanPhysicalDevice.h"


namespace Kaidel {
	VulkanPhysicalDevice::VulkanPhysicalDevice(const VulkanInstance& instance, const std::vector<VulkanQueueGroupSpecification>& specs)
		:m_Device(VK_NULL_HANDLE)
	{
		auto devices = GetDevices(instance);
		for (auto& device : devices) {
			m_QueueManager.FillQueueManager(specs, device);
			if (m_QueueManager.Filled()) {
				m_Device = device;
				VkPhysicalDeviceProperties props{};
				vkGetPhysicalDeviceProperties(m_Device, &props);
				KD_CORE_INFO("Chosen device {}", props.deviceName);
				m_Properties = CreateScope<VkPhysicalDeviceProperties>(props);

				break;
			}
		}
		KD_CORE_ASSERT(m_Device);
	}

	std::vector<VkPhysicalDevice> VulkanPhysicalDevice::GetDevices(const VulkanInstance& instance) {
		uint32_t deviceCount = 0;
		std::vector<VkPhysicalDevice> devices;
		VK_ASSERT(vkEnumeratePhysicalDevices(instance.GetInstance(), &deviceCount, nullptr));
		devices.resize(deviceCount);
		VK_ASSERT(vkEnumeratePhysicalDevices(instance.GetInstance(), &deviceCount, devices.data()));
		return devices;
	}
}
