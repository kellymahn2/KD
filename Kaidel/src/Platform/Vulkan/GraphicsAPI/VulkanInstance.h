#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"

namespace Kaidel {
	class VulkanInstance {
	public:
		VulkanInstance(const std::string& name, const std::vector<const char*>& enabledExtensions, const std::vector<const char*>& layers);
		~VulkanInstance();
		VkInstance GetInstance()const { return m_Instance; }
		VkDebugUtilsMessengerEXT GetDebugMessenger()const { return m_DebugMessenger; }

	private:
		static VkBool32 DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageTypes,
											const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData);
	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
	};

}
