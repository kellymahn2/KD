#include "KDpch.h"
#include "VulkanPhysicalDevice.h"





namespace Kaidel {

	namespace Utils {

		static bool QueueFamilyHasFlags(const VkQueueFamilyProperties& queueFamily, uint32_t flag) {
			return (queueFamily.queueFlags & (flag)) == flag;
		}

		static std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance) {
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
			return devices;
		}

		static VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures(VkPhysicalDevice device) {
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
			return deviceFeatures;
		}

		static bool CompareFeatures(const VkPhysicalDeviceFeatures& features, const VkPhysicalDeviceFeatures& wantedDeviceFeatures){

			const char* featuresIt = reinterpret_cast<const char*>(&features);
			const char* wantedDeviceFeaturesIt = reinterpret_cast<const char*>(&wantedDeviceFeatures);


			for (uint32_t i = 0; i < sizeof(VkPhysicalDeviceFeatures); ++i) {
				if (wantedDeviceFeaturesIt[i] != 0 && wantedDeviceFeaturesIt[i] != featuresIt[i])
					return false;
			}
			return true;
		}


		static std::vector<VkQueueFamilyProperties> GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device) {
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
			return queueFamilies;
		}


		static SwapChainSupportDetails GetPhysicalDeviceSwapChainSupportDetails(VkPhysicalDevice device,VkSurfaceKHR surface) {
			SwapChainSupportDetails details;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

			if (formatCount != 0) {
				details.Formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.PresentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
			}

			return details;
		}

		static uint32_t GetQueueFamilyWithSpec(VkPhysicalDevice device,const QueueFamilySpecification& spec, const std::vector<VkQueueFamilyProperties>& queueFamilyProperties) {
			uint32_t i = 0;
			for (auto& queueProps : queueFamilyProperties) {
				if (QueueFamilyHasFlags(queueProps, spec.QueueFamilyFlags) && spec.Predicate(device,i, queueProps))
					return i;
				++i;
			}
			return -1;
		}


		static bool PhysicalDeviceHasExtensions(VkPhysicalDevice device,const std::vector<const char*>& wantedExtensions) {
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			auto finder = [](const auto& vec, const char* str) {

				for (auto& extension : vec) {
					if (std::strcmp(str, extension.extensionName) == 0)
						return true;
				}
				return false;
			};

			for (auto& wantedExtension : wantedExtensions) {
				if (!finder(availableExtensions, wantedExtension))
					return false;
			}
			return true;
		}
		static auto TurnToStrings(const std::vector<const char*>& vec) {
			std::vector<std::string> strs;
			for (auto& str : vec) {
				strs.push_back(std::string(str));
			}
			return strs;
		}

		static bool IsPhysicalDeviceSwapChainAdequate(VkPhysicalDevice physicalDevice,VkSurfaceKHR surface) {
			SwapChainSupportDetails swapChainSupport = GetPhysicalDeviceSwapChainSupportDetails(physicalDevice,surface);
			return !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

	}

	Scope<VulkanPhysicalDevice> VulkanPhysicalDevice::s_VulkanPhysicalDevice;

	void VulkanPhysicalDevice::Init(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDeviceType deviceType, const std::unordered_map<std::string, QueueFamilySpecification>& queueSpecs, const std::vector<const char*> extensions, const VkPhysicalDeviceFeatures& wantedDeviceFeatures, bool needsSwapChain)
	{


		s_VulkanPhysicalDevice = CreateScope<VulkanPhysicalDevice>();
		std::vector<VkPhysicalDevice> physicalDevices = std::move(Utils::GetPhysicalDevices(instance));

		bool deviceChosen = false;
		

		for (auto& physicalDevice : physicalDevices) {

			if (!Utils::CompareFeatures(Utils::GetPhysicalDeviceFeatures(physicalDevice), wantedDeviceFeatures) || !Utils::PhysicalDeviceHasExtensions(physicalDevice, extensions))
				continue;

			if (needsSwapChain && !Utils::IsPhysicalDeviceSwapChainAdequate(physicalDevice, surface))
				continue;

			if (needsSwapChain)
				s_VulkanPhysicalDevice->m_SwapChainSupportDetails = Utils::GetPhysicalDeviceSwapChainSupportDetails(physicalDevice, surface);


			std::vector<VkQueueFamilyProperties> queueFamilyProperties = Utils::GetPhysicalDeviceQueueFamilyProperties(physicalDevice);

			std::unordered_map<std::string, uint32_t> queueFamilies;

			bool deviceHasQueueFamilies = true;
			for (auto& [name, spec] : queueSpecs) {
				uint32_t familyIndex = Utils::GetQueueFamilyWithSpec(physicalDevice, spec, queueFamilyProperties);
				if (familyIndex == -1) {
					deviceHasQueueFamilies = false;
					break;
				}

				queueFamilies[name] = familyIndex;
			}

			if (!deviceHasQueueFamilies)
				continue;


			deviceChosen = true;
			s_VulkanPhysicalDevice->m_PhysicalDevice = physicalDevice;
			s_VulkanPhysicalDevice->m_QueueFamilyMap = std::move(queueFamilies);
			break;
		}

		KD_CORE_ASSERT(deviceChosen);

		s_VulkanPhysicalDevice->m_QueueSpecs = queueSpecs;
		s_VulkanPhysicalDevice->m_WantedDeviceExtensions = Utils::TurnToStrings(extensions);
	}
	void VulkanPhysicalDevice::Shutdown()
	{
	}
}
