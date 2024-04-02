#pragma once

#include "Kaidel/Core/Base.h"

#include <glad/vulkan.h>


#include <string>
#include <vector>





namespace Kaidel {

	// bool(VkPhysicalDevice physicalDevice,uint32_t queueFamilyIndex,const VkQUeueFamilyProperties& queueFamilyProps);
	using QueueFamilyCheck = std::function<bool(VkPhysicalDevice,uint32_t, const VkQueueFamilyProperties&)>;

	struct QueueFamilySpecification {
		int QueueFamilyFlags;
		QueueFamilyCheck Predicate = [](auto&&...) {return true; };
		bool CreateSeperate = false;
	};


	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};


	class VulkanPhysicalDevice{
	public:

		const auto& GetQueueFamilyMap()const { return m_QueueFamilyMap; }
		const auto& GetQueueSpecs()const { return m_QueueSpecs; }
		const auto& GetWantedDeviceExtensions()const { return m_WantedDeviceExtensions; }

		const auto& GetSwapChainSupportDetails()const { return m_SwapChainSupportDetails; }

		VkPhysicalDevice GetPhysicalDevice()const { return m_PhysicalDevice; }

		static void Init(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDeviceType deviceType, const std::unordered_map<std::string, QueueFamilySpecification>& queueSpecs,
			const std::vector<const char*> extensions, const VkPhysicalDeviceFeatures& wantedDeviceFeatures, bool needsSwapChain = true);
		static void Shutdown();

		static VulkanPhysicalDevice& Get() { return *s_VulkanPhysicalDevice; }


	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

		std::unordered_map<std::string, uint32_t> m_QueueFamilyMap;
		std::unordered_map<std::string, QueueFamilySpecification> m_QueueSpecs;
		std::vector<std::string> m_WantedDeviceExtensions;
		SwapChainSupportDetails m_SwapChainSupportDetails;

		static Scope<VulkanPhysicalDevice> s_VulkanPhysicalDevice;

	};


}



