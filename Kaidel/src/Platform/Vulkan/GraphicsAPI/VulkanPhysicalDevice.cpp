#include "KDpch.h"
#include "VulkanPhysicalDevice.h"

#include <set>


namespace Kaidel {
	namespace Vulkan {
		namespace Utils {
		

			std::vector<VkQueueFamilyProperties> QuerySupportedPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice) {
				uint32_t supportedPhysicalDeviceQueueFamilyCount = 0;
				std::vector<VkQueueFamilyProperties> supportedQueueFamilies;
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, nullptr);
				supportedQueueFamilies.resize(supportedPhysicalDeviceQueueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, supportedQueueFamilies.data());
				return supportedQueueFamilies;
			}

			bool QueryPhysicalDeviceQueueFamilyPresentSupport(VkPhysicalDevice physicalDevice, uint32_t familyIndex, VkSurfaceKHR surface) {
				VkBool32 supported = VK_TRUE;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supported);
				return supported;
			}

			QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

				QueueFamilyIndices indices{};


				std::vector<VkQueueFamilyProperties> queueFamilyProperties = QuerySupportedPhysicalDeviceQueueFamilies(physicalDevice);


				uint32_t i = 0;
				for (auto& family : queueFamilyProperties) {
					if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						indices.GraphicsQueueFamilyIndex = i;
					}

					if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
						indices.TransferQueueFamilyIndex = i;
					}

					if (QueryPhysicalDeviceQueueFamilyPresentSupport(physicalDevice, i, surface) == VK_TRUE) {
						indices.PresentQueueFamilyIndex = i;
					}


					if (indices.Valid())
						break;

					++i;
				}

				return indices;
			}

			std::vector<VkExtensionProperties> QuerySupportedPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice) {
				uint32_t supportedPhysicalDeviceExtensionCount = 0;
				std::vector<VkExtensionProperties> supportedPhysicalDeviceExtensions;
				VK_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedPhysicalDeviceExtensionCount, nullptr));
				supportedPhysicalDeviceExtensions.resize(supportedPhysicalDeviceExtensionCount);
				VK_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedPhysicalDeviceExtensionCount, supportedPhysicalDeviceExtensions.data()));
				return supportedPhysicalDeviceExtensions;
			}

			bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, const PhysicalDeviceSpecification& physicalDeviceSpecification) {
				std::set<std::string> requiredExtensions(physicalDeviceSpecification.WantedPhysicalDeviceExtensions.begin(), physicalDeviceSpecification.WantedPhysicalDeviceExtensions.end());

				std::vector<VkExtensionProperties> supportedPhysicalDeviceExtensions = QuerySupportedPhysicalDeviceExtensions(physicalDevice);

				for (VkExtensionProperties& extension : supportedPhysicalDeviceExtensions) {
					requiredExtensions.erase(extension.extensionName);
				}

				return requiredExtensions.empty();
			}

			PhysicalDeviceChooseResult ChoosePhysicalDevice(VkInstance instance, const std::vector<VulkanQueueGroupSpecification>& queueGroups,const PhysicalDeviceSpecification& physicalDeviceSpecification) {
				PhysicalDeviceChooseResult result{};

				uint32_t physicalDeviceCount = 0;
				vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
				std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
				vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

				for (auto& physicalDevice : physicalDevices) {


					if (IsPhysicalDeviceSuitable(physicalDevice,physicalDeviceSpecification)) {
						result.QueueManager.FillQueueManager(queueGroups,physicalDevice);

						if (result.QueueManager.Filled()) {
							result.PhysicalDevice = physicalDevice;
							break;
						}
					}

					/*if (IsPhysicalDeviceSuitable(physicalDevice, surface, physicalDeviceSpecification)) {
						QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice, surface);
						if (indices.Valid()) {
							result.PhysicalDevice = physicalDevice;
							result.Indices = indices;
							break;
						}
					}*/
				}

				return result;
			}
		}
	}
}
