#pragma once


#include "Platform/Vulkan/GraphicsAPI/VulkanBase.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanDefinitions.h"

struct GLFWwindow;

namespace Kaidel {
	
	namespace Utils {
		

		struct InstanceCreateResult {
			uint32_t VulkanAPIVersion = 0;
			VkInstance Instance;
		};

		struct SurfaceCreateResult {
			VkSurfaceKHR Surface;
		};


		struct DebugMessengerCreateResult {
			VkDebugUtilsMessengerEXT Messenger;
		};



		std::vector<VkExtensionProperties> QuerySupportedInstanceExtensions();
		std::vector<VkLayerProperties> QuerySupportedInstanceLayers();
		InstanceCreateResult CreateInstance(const Vulkan::InstanceSpecification& instanceSpecification);
		SurfaceCreateResult CreateSurface(VkInstance instance, GLFWwindow* window);
		DebugMessengerCreateResult CreateDebugMessenger(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback);
	}

}
