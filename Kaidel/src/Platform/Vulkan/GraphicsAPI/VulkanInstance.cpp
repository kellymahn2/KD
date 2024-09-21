#include "KDpch.h"
#include "VulkanInstance.h"
namespace Kaidel {


	VkBool32 VulkanInstance::DebugMessengerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:KD_CORE_INFO(pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:KD_CORE_WARN(pCallbackData->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			std::cout << pCallbackData->pMessage << std::endl;
			break;
		}
		return VK_FALSE;
	}

	VulkanInstance::VulkanInstance(const std::string& name, const std::vector<const char*>& enabledExtensions, const std::vector<const char*>& layers) {

		uint32_t version;
		//Query API version
		VK_ASSERT(vkEnumerateInstanceVersion(&version));


		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pEngineName = name.c_str();
		appInfo.pApplicationName = name.c_str();
		appInfo.engineVersion = 1;
		appInfo.apiVersion = version;


		VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
		instanceInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceInfo.ppEnabledLayerNames = layers.data();
		instanceInfo.enabledLayerCount = (uint32_t)layers.size();
		instanceInfo.pApplicationInfo = &appInfo;

		VK_ASSERT(vkCreateInstance(&instanceInfo, nullptr, &m_Instance));

		gladLoaderLoadVulkan(m_Instance, VK_NULL_HANDLE, VK_NULL_HANDLE);

		VkDebugUtilsMessengerCreateInfoEXT messengerInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
		messengerInfo.pfnUserCallback = DebugMessengerCallback;
		messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
									VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
									VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | 
										VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
										VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		VK_ASSERT(vkCreateDebugUtilsMessengerEXT(m_Instance, &messengerInfo, nullptr, &m_DebugMessenger));

	}

	VulkanInstance::~VulkanInstance() {
		vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}
}
