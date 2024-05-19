#pragma once


#include "VulkanBase.h"
#include "VulkanDefinitions.h"

struct GLFWwindow;

namespace Kaidel {
	namespace Vulkan {
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
			InstanceCreateResult CreateInstance(const InstanceSpecification& instanceSpecification);
			SurfaceCreateResult CreateSurface(VkInstance instance, GLFWwindow* window);
			std::vector<uint32_t> GetUniqueFamilyIndices(const QueueFamilyIndices& familyIndices);
			DebugMessengerCreateResult CreateDebugMessenger(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT callback);
		}
	}
}
