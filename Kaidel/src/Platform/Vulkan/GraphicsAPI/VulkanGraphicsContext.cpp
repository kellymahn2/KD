#include "KDpch.h"
#include "VulkanGraphicsContext.h"
#include "VulkanSingleShader.h"
#include <GLFW/glfw3.h>

#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
namespace Kaidel {

	


	namespace Vulkan {
		namespace Utils {
			static std::vector<VkExtensionProperties> QuerySupportedInstanceExtensions() {
				std::vector<VkExtensionProperties> supportedInstanceExtensions;
				uint32_t supportedInstanceExtensionCount = 0;
				VK_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &supportedInstanceExtensionCount, nullptr));
				supportedInstanceExtensions.resize(supportedInstanceExtensionCount);
				VK_ASSERT(vkEnumerateInstanceExtensionProperties(nullptr, &supportedInstanceExtensionCount, supportedInstanceExtensions.data()));
				return supportedInstanceExtensions;
			}

			static std::vector<VkLayerProperties> QuerySupportedInstanceLayers() {
				uint32_t supportedInstanceLayerCount = 0;
				std::vector<VkLayerProperties> supportedInstanceLayers;
				VK_ASSERT(vkEnumerateInstanceLayerProperties(&supportedInstanceLayerCount, nullptr));
				supportedInstanceLayers.resize(supportedInstanceLayerCount);
				VK_ASSERT(vkEnumerateInstanceLayerProperties(&supportedInstanceLayerCount, supportedInstanceLayers.data()));
				return supportedInstanceLayers;
			}

			struct InstanceCreateResult {
				uint32_t VulkanAPIVersion = 0;
				VkInstance Instance;
			};

			static InstanceCreateResult CreateInstance(const InstanceSpecification& instanceSpecification) {

				InstanceCreateResult result{};


				gladLoaderLoadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);

				//Query supported instance extensions
				std::vector<VkExtensionProperties> supportedInstanceExtensions = Utils::QuerySupportedInstanceExtensions();

				//Check wanted extesions against supported extensions
				for (auto& wantedExtensionName : instanceSpecification.WantedInstanceExtensions) {
					bool found = false;
					for (auto& supportedInstanceExtension : supportedInstanceExtensions) {
						if (strcmp(wantedExtensionName ,supportedInstanceExtension.extensionName) == 0) {
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
						if (strcmp(wantedLayerName ,supportedInstanceLayer.layerName)) {
							found = true;
							break;
						}
					}
					KD_CORE_ASSERT(found, "A wanted layer is not supported");
				}

				//Query API version
				VK_ASSERT(vkEnumerateInstanceVersion(&result.VulkanAPIVersion));

				//Application info
				VK_STRUCT(VkApplicationInfo, applicationInfo, APPLICATION_INFO);
				applicationInfo.apiVersion = result.VulkanAPIVersion;
				applicationInfo.applicationVersion = 1;
				applicationInfo.engineVersion = 1;
				applicationInfo.pEngineName = instanceSpecification.ApplicationName.c_str();
				applicationInfo.pApplicationName = instanceSpecification.ApplicationName.c_str();

				//Instance info
				VK_STRUCT(VkInstanceCreateInfo, instanceInfo, INSTANCE_CREATE_INFO);
				instanceInfo.pApplicationInfo = &applicationInfo;
				instanceInfo.enabledExtensionCount = (uint32_t)instanceSpecification.WantedInstanceExtensions.size();
				instanceInfo.ppEnabledExtensionNames = instanceSpecification.WantedInstanceExtensions.data();
				instanceInfo.enabledLayerCount = (uint32_t)instanceSpecification.WantedInstanceLayers.size();
				instanceInfo.ppEnabledLayerNames = instanceSpecification.WantedInstanceLayers.data();

				//Creating the vulkan instance
				VK_ASSERT(vkCreateInstance(&instanceInfo, VK_ALLOCATOR_PTR, &result.Instance));
				return result;
			}

			struct SurfaceCreateResult {
				VkSurfaceKHR Surface;
			};

			static SurfaceCreateResult CreateSurface(VkInstance instance, GLFWwindow* window) {
				SurfaceCreateResult result{};
				VK_ASSERT(glfwCreateWindowSurface(instance, window, VK_ALLOCATOR_PTR, &result.Surface));
				return result;
			}

			struct PhysicalDeviceChooseResult {
				VkPhysicalDevice PhysicalDevice;
				QueueFamilyIndices Indices;
			};

			static std::vector<VkQueueFamilyProperties> QuerySupportedPhysicalDeviceQueueFamilies(VkPhysicalDevice physicalDevice) {
				uint32_t supportedPhysicalDeviceQueueFamilyCount = 0;
				std::vector<VkQueueFamilyProperties> supportedQueueFamilies;
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, nullptr);
				supportedQueueFamilies.resize(supportedPhysicalDeviceQueueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &supportedPhysicalDeviceQueueFamilyCount, supportedQueueFamilies.data());
				return supportedQueueFamilies;
			}

			static bool QueryPhysicalDeviceQueueFamilyPresentSupport(VkPhysicalDevice physicalDevice, uint32_t familyIndex, VkSurfaceKHR surface) {
				VkBool32 supported = VK_TRUE;
				vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supported);
				return supported;
			}

			static QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

				QueueFamilyIndices indices{};


				std::vector<VkQueueFamilyProperties> queueFamilyProperties = QuerySupportedPhysicalDeviceQueueFamilies(physicalDevice);


				uint32_t i = 0;
				for (auto& family : queueFamilyProperties) {
					if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
						indices.GraphicsQueueFamilyIndex = i;
					}

					if (QueryPhysicalDeviceQueueFamilyPresentSupport(physicalDevice,i,surface) == VK_TRUE) {
						indices.PresentQueueFamilyIndex = i;
					}


					if (indices.Valid())
						break;

					++i;
				}

				return indices;
			}

			static std::vector<VkExtensionProperties> QuerySupportedPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice) {
				uint32_t supportedPhysicalDeviceExtensionCount = 0;
				std::vector<VkExtensionProperties> supportedPhysicalDeviceExtensions;
				VK_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedPhysicalDeviceExtensionCount, nullptr));
				supportedPhysicalDeviceExtensions.resize(supportedPhysicalDeviceExtensionCount);
				VK_ASSERT(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &supportedPhysicalDeviceExtensionCount, supportedPhysicalDeviceExtensions.data()));
				return supportedPhysicalDeviceExtensions;
			}

			static bool IsPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, const PhysicalDeviceSpecification& physicalDeviceSpecification) {
				std::set<std::string> requiredExtensions(physicalDeviceSpecification.WantedPhysicalDeviceExtensions.begin(), physicalDeviceSpecification.WantedPhysicalDeviceExtensions.end());

				std::vector<VkExtensionProperties> supportedPhysicalDeviceExtensions = QuerySupportedPhysicalDeviceExtensions(physicalDevice);

				for (VkExtensionProperties& extension : supportedPhysicalDeviceExtensions) {
					requiredExtensions.erase(extension.extensionName);
				}

				return requiredExtensions.empty();
			}

			static PhysicalDeviceChooseResult ChoosePhysicalDevice(VkInstance instance,VkSurfaceKHR surface, const PhysicalDeviceSpecification& physicalDeviceSpecification) {
				PhysicalDeviceChooseResult result{};

				uint32_t physicalDeviceCount = 0;
				vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
				std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount, VK_NULL_HANDLE);
				vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

				for (auto& physicalDevice : physicalDevices) {
					if (IsPhysicalDeviceSuitable(physicalDevice, surface,physicalDeviceSpecification)) {
						QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice, surface);
						if (indices.Valid()) {
							result.PhysicalDevice = physicalDevice;
							result.Indices = indices;
							break;
						}
					}
				}

				return result;
			}

			struct LogicalDeviceCreateResult {
				VkDevice LogicalDevice;
				DeviceQueues Queues;
			};

			static std::vector<uint32_t> GetUniqueFamilyIndices(const QueueFamilyIndices& familyIndices) {
				uint32_t families[] = { familyIndices.GraphicsQueueFamilyIndex.value(),familyIndices.PresentQueueFamilyIndex.value() };
				uint32_t* uniqueFamiliesEnd = std::unique(families, families + ARRAYSIZE(families));
				return { families,uniqueFamiliesEnd };
			}

			static LogicalDeviceCreateResult CreateLogicalDevice(VkPhysicalDevice physicalDevice,const QueueFamilyIndices& familyIndices, const LogicalDeviceSpecification& logicalDeviceSpecification) {
				LogicalDeviceCreateResult result{};

				KD_CORE_ASSERT(familyIndices.Valid());

				uint32_t families[] = { familyIndices.GraphicsQueueFamilyIndex.value(),familyIndices.PresentQueueFamilyIndex.value() };
				uint32_t* uniqueFamilies = std::unique(families, families + ARRAYSIZE(families));
				
				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
				float queuePriority = 1.0f;

				for (uint32_t* currFamily = families; currFamily != uniqueFamilies; ++currFamily) {
					VK_STRUCT(VkDeviceQueueCreateInfo, queueCreateInfo, DEVICE_QUEUE_CREATE_INFO);
					queueCreateInfo.pQueuePriorities = &queuePriority;
					queueCreateInfo.queueCount = 1;
					queueCreateInfo.queueFamilyIndex = *currFamily;
					queueCreateInfos.push_back(queueCreateInfo);
				}

				VK_STRUCT(VkDeviceCreateInfo, deviceInfo, DEVICE_CREATE_INFO);
				deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
				deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
				deviceInfo.enabledExtensionCount = (uint32_t)logicalDeviceSpecification.Extensions.size();
				deviceInfo.ppEnabledExtensionNames = logicalDeviceSpecification.Extensions.data();
				deviceInfo.enabledLayerCount = (uint32_t)logicalDeviceSpecification.Layers.size();
				deviceInfo.ppEnabledLayerNames = logicalDeviceSpecification.Layers.data();
				deviceInfo.pEnabledFeatures = &logicalDeviceSpecification.Features;

				VK_ASSERT(vkCreateDevice(physicalDevice, &deviceInfo, VK_ALLOCATOR_PTR, &result.LogicalDevice));

				vkGetDeviceQueue(result.LogicalDevice, familyIndices.GraphicsQueueFamilyIndex.value(), 0, &result.Queues.GraphicsQueue);
				vkGetDeviceQueue(result.LogicalDevice, familyIndices.PresentQueueFamilyIndex.value(), 0, &result.Queues.PresentQueue);

				return result;
			}

			struct DebugMessengerCreateResult {
				VkDebugUtilsMessengerEXT Messenger;
			};

			static VkBool32 DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
				VkDebugUtilsMessageTypeFlagsEXT messageTypes,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData)
			{
				switch (messageSeverity)
				{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: KD_CORE_INFO(pCallbackData->pMessage);break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:KD_CORE_WARN(pCallbackData->pMessage);break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: KD_CORE_ERROR(pCallbackData->pMessage); break;
				}
				return VK_FALSE;
			}

			static DebugMessengerCreateResult CreateDebugMessenger(VkInstance instance) {

				DebugMessengerCreateResult result{};

				gladLoaderLoadVulkan(instance, nullptr, nullptr);
				
				VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
				messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
				messengerCreateInfo.messageType =
					VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
					| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
					| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
				messengerCreateInfo.messageSeverity =
					VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
					| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
					| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
					;

				messengerCreateInfo.pfnUserCallback = DebugMessengerCallback;
				VK_ASSERT(vkCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, VK_ALLOCATOR_PTR, &result.Messenger));
				return result;
			}
		}
		VulkanGraphicsContext::VulkanGraphicsContext(GLFWwindow* window)
			:m_Window(window)
		{
			KD_CORE_ASSERT(window, "Window handle is null!");
		}

		void VulkanGraphicsContext::Init()
		{
			bool isDebug = false;

#ifdef KD_DEBUG
			isDebug = true;
#endif // KD_DEBUG

			m_InstanceSpecification.ApplicationName = "Kaidel";
			//GLFW extensions
			{
				uint32_t glfwExtensionCount = 0;
				const char** glfwExtensions;
				glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

				m_InstanceSpecification.WantedInstanceExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
				if (isDebug) {
					m_InstanceSpecification.WantedInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}
			}

			//Layers
			{
				if (isDebug) {
					m_InstanceSpecification.WantedInstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
				}
			}

			//Instance creation
			auto instanceCreateResult = Utils::CreateInstance(m_InstanceSpecification);
			m_Instance = instanceCreateResult.Instance;
			m_VulkanAPIVersion = instanceCreateResult.VulkanAPIVersion;

			//Debug messenger creation
			auto debugMessengerCreateResult = Utils::CreateDebugMessenger(m_Instance);
			m_DebugMessenger = debugMessengerCreateResult.Messenger;

			//Surface creation
			auto surfaceCreateResult = Utils::CreateSurface(m_Instance, m_Window);
			m_Surface = surfaceCreateResult.Surface;

			m_PhysicalDeviceSpecification.WantedPhysicalDeviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			//Physical device choosing
			auto physicalDeviceChooseResult = Utils::ChoosePhysicalDevice(m_Instance, m_Surface, m_PhysicalDeviceSpecification);
			m_QueueFamilyIndices = physicalDeviceChooseResult.Indices;
			m_PhysicalDevice = physicalDeviceChooseResult.PhysicalDevice;

			m_LogicalDeviceSpecification.Extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			if (isDebug) {
				m_LogicalDeviceSpecification.Layers.push_back("VK_LAYER_KHRONOS_validation");
			}

			//Logical device creation
			auto logicalDeviceCreateResult = Utils::CreateLogicalDevice(m_PhysicalDevice, m_QueueFamilyIndices, m_LogicalDeviceSpecification);
			m_DeviceQueues = logicalDeviceCreateResult.Queues;
			m_LogicalDevice = logicalDeviceCreateResult.LogicalDevice;


			gladLoaderLoadVulkan(m_Instance, m_PhysicalDevice, m_LogicalDevice);

			//Swapchain
			VulkanSwapchainSpecification swapchainSpecification{};
			swapchainSpecification.Extent = { 1280,720 };
			swapchainSpecification.ImageCount = 4;
			swapchainSpecification.LogicalDevice = m_LogicalDevice;
			swapchainSpecification.PhysicalDevice = m_PhysicalDevice;
			swapchainSpecification.PresentQueue = m_DeviceQueues.PresentQueue;

			auto uniqueFamilies = Utils::GetUniqueFamilyIndices(m_QueueFamilyIndices);
			swapchainSpecification.QueueFamiliesToShare = uniqueFamilies;
			if (swapchainSpecification.QueueFamiliesToShare.size() > 1) {
				swapchainSpecification.SharingMode = ImageSharingMode::Share;
			}
			else {
				swapchainSpecification.SharingMode = ImageSharingMode::NoShare;
			}
			swapchainSpecification.Surface = m_Surface;
			swapchainSpecification.SwapchainFormat = VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			swapchainSpecification.SwapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;


			m_Swapchain = CreateRef<VulkanSwapchain>(swapchainSpecification);
		}

		void VulkanGraphicsContext::Shutdown() {

			//Swapchain
			m_Swapchain = nullptr;

			//Logical device
			vkDestroyDevice(m_LogicalDevice, VK_ALLOCATOR_PTR);
			//Surface
			vkDestroySurfaceKHR(m_Instance, m_Surface, VK_ALLOCATOR_PTR);
			//Debug messenger
			vkDestroyDebugUtilsMessengerEXT(m_Instance,m_DebugMessenger, VK_ALLOCATOR_PTR);
			//Instance
			vkDestroyInstance(m_Instance, VK_ALLOCATOR_PTR);
		}

		void VulkanGraphicsContext::SwapBuffers()
		{
			//TODO: implement buffer swapping
		}
	}
}



