#include "KDpch.h"
#include "VulkanPhysicalDevice.h"


namespace Kaidel {

	VulkanPhysicalDevice::VulkanPhysicalDevice(
		const VulkanInstance& instance, const VulkanSurface& surface, 
		const std::unordered_map<std::string, QueueCapability>& requested)

		:m_Device(VK_NULL_HANDLE)
	{
		auto devices = GetDevices(instance);

		for (auto& device : devices) 
		{
			
			VkPhysicalDeviceProperties props{};
			vkGetPhysicalDeviceProperties(device, &props);
			
			auto queues = GetRequestedQueues(requested, device, surface.GetSurface());
			
			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && !queues.empty()) 
			{
				m_Device = device;
				
				m_Queues = std::move(queues);

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
