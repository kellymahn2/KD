#include "KDpch.h"
#include "VulkanInstance.h"

#include <GLFW/glfw3.h>



namespace Kaidel {
	namespace Vulkan {

		namespace Utils {
			std::vector<VkExtensionProperties> QuerySupportedInstanceExtensions() {
				std::vector<VkExtensionProperties> supportedInstanceExtensions;
				uint32_t supportedInstanceExtensionCount = 0;
				VK_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &supportedInstanceExtensionCount, nullptr));
				supportedInstanceExtensions.resize(supportedInstanceExtensionCount);
				VK_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &supportedInstanceExtensionCount, supportedInstanceExtensions.data()));
				return supportedInstanceExtensions;
			}

			std::vector<VkLayerProperties> QuerySupportedInstanceLayers() {
				uint32_t supportedInstanceLayerCount = 0;
				std::vector<VkLayerProperties> supportedInstanceLayers;
				VK_ASSERT(vkEnumerateInstanceLayerProperties(&supportedInstanceLayerCount, nullptr));
				supportedInstanceLayers.resize(supportedInstanceLayerCount);
				VK_ASSERT(vkEnumerateInstanceLayerProperties(&supportedInstanceLayerCount, supportedInstanceLayers.data()));
				return supportedInstanceLayers;
			}

			InstanceCreateResult CreateInstance(const InstanceSpecification& instanceSpecification) {

				InstanceCreateResult result{};


				gladLoaderLoadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);

				//Query supported instance extensions
				std::vector<VkExtensionProperties> supportedInstanceExtensions = Utils::QuerySupportedInstanceExtensions();

				//Check wanted extesions against supported extensions
				for (auto& wantedExtensionName : instanceSpecification.WantedInstanceExtensions) {
					bool found = false;
					for (auto& supportedInstanceExtension : supportedInstanceExtensions) {
						if (strcmp(wantedExtensionName, supportedInstanceExtension.extensionName) == 0) {
							found = true;
							break;
						}
					}
					KD_CORE_ASSERT(found, "A wanted extension is not supported");
				}

				//Query supported instance layers
				std::vector<VkLayerProperties> supportedInstanceLayers = Utils::QuerySupportedInstanceLayers();

				//Check wanted layers against supported layers
				for (auto& wantedLayerName : instanceSpecification.WantedInstanceLayers) {
					bool found = false;
					for (auto& supportedInstanceLayer : supportedInstanceLayers) {
						if (strcmp(wantedLayerName, supportedInstanceLayer.layerName)) {
							found = true;
							break;
						}
					}
					KD_CORE_ASSERT(found, "A wanted layer is not supported");
				}

				//Query API version
				VK_ASSERT(vkEnumerateInstanceVersion(&result.VulkanAPIVersion));

				//Application info
				VK_STRUCT(VkApplicationInfo, applicationInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
				applicationInfo.apiVersion = result.VulkanAPIVersion;
				applicationInfo.applicationVersion = 1;
				applicationInfo.engineVersion = 1;
				applicationInfo.pEngineName = instanceSpecification.ApplicationName.c_str();
				applicationInfo.pApplicationName = instanceSpecification.ApplicationName.c_str();

				//Instance info
				VK_STRUCT(VkInstanceCreateInfo, instanceInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
				instanceInfo.pApplicationInfo = &applicationInfo;
				instanceInfo.enabledExtensionCount = (uint32_t)instanceSpecification.WantedInstanceExtensions.size();
				instanceInfo.ppEnabledExtensionNames = instanceSpecification.WantedInstanceExtensions.data();
				instanceInfo.enabledLayerCount = (uint32_t)instanceSpecification.WantedInstanceLayers.size();
				instanceInfo.ppEnabledLayerNames = instanceSpecification.WantedInstanceLayers.data();

				//Creating the vulkan instance
				VK_ASSERT(vkCreateInstance(&instanceInfo, VK_ALLOCATOR_PTR, &result.Instance));
				return result;
			}

			SurfaceCreateResult CreateSurface(VkInstance instance, GLFWwindow* window) {
				SurfaceCreateResult result{};
				VK_ASSERT(glfwCreateWindowSurface(instance, window, VK_ALLOCATOR_PTR, &result.Surface));
				return result;
			}

			std::vector<uint32_t> GetUniqueFamilyIndices(const QueueFamilyIndices& familyIndices) {
				uint32_t families[] = { familyIndices.GraphicsQueueFamilyIndex.value(),familyIndices.PresentQueueFamilyIndex.value() };
				uint32_t* uniqueFamiliesEnd = std::unique(families, families + ARRAYSIZE(families));
				return { families,uniqueFamiliesEnd };
			}

			DebugMessengerCreateResult CreateDebugMessenger(VkInstance instance,PFN_vkDebugUtilsMessengerCallbackEXT callback) {

				DebugMessengerCreateResult result{};

				gladLoaderLoadVulkan(instance, nullptr, nullptr);

				VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
				messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
				messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
				messengerCreateInfo.pfnUserCallback = callback;
				VK_ASSERT(vkCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, VK_ALLOCATOR_PTR, &result.Messenger));
				return result;
			}
		}



	}
}
