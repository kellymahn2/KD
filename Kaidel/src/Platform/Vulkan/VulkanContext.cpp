#include "KDpch.h"
#include "VulkanContext.h"

#include "Kaidel/Core/Application.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapChain.h"

#include <vector>



 
namespace Kaidel {


	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:KD_CORE_INFO(pCallbackData->pMessage); return VK_TRUE;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:KD_CORE_TRACE(pCallbackData->pMessage); return VK_TRUE;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:KD_CORE_WARN(pCallbackData->pMessage); return VK_TRUE;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:KD_CORE_ERROR(pCallbackData->pMessage); return VK_TRUE;
		}
		return VK_FALSE;
	}
	VulkanContext::VulkanContext(GLFWwindow* context)
		:m_Context(context)
	{
		KD_CORE_ASSERT(context, "Window handle is null");
	}


	static bool CheckValidationLayerSupport(const std::vector<VkLayerProperties> availableLayers,const char* layer) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layer, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}
	void VulkanContext::Init()
	{


		gladLoaderLoadVulkan(NULL, NULL, NULL);
		
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "VulkanApp";
		appInfo.pEngineName = "KaidelEngine";
		appInfo.apiVersion = VK_API_VERSION_1_3;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		std::vector<const char*> requestedValidationLayers;

		//Validation layers
#ifdef KD_DEBUG
		requestedValidationLayers.push_back("VK_LAYER_KHRONOS_validation");
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		
		VulkanInstance::Init(appInfo, extensions, requestedValidationLayers);

		auto vkInstance = VulkanInstance::Get()->GetVulkanInstance();

		gladLoaderLoadVulkan(vkInstance, NULL, NULL);

		//Init Debug messenger
#ifdef KD_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebugCallback;
		createInfo.pUserData = nullptr;
		vkCreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &m_DebugMessenger);
#endif


		//Init surface
		{
			VulkanSurface::Init(m_Context, vkInstance);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		std::vector<const char*> wantedPhysicalDeviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};


		//Init physical device
		{
			std::unordered_map<std::string, QueueFamilySpecification> queueSpecs;

			//Graphics queue
			{
				QueueFamilySpecification spec;
				spec.QueueFamilyFlags = VK_QUEUE_GRAPHICS_BIT;
				queueSpecs[VULKAN_GRAPHICS_QUEUE_NAME] = spec;
			}
			//Present queue
			{
				QueueFamilySpecification spec;
				spec.QueueFamilyFlags = 0;
				spec.Predicate = [surface = VulkanSurface::Get().GetSurface()](auto device, auto index, const auto& specs) {
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);
					return presentSupport;
					};
				queueSpecs[VULKAN_PRESENT_QUEUE_NAME] = spec;
			}


			deviceFeatures.geometryShader = VK_TRUE;
			deviceFeatures.tessellationShader = VK_TRUE;

					

			VulkanPhysicalDevice::Init(vkInstance, VulkanSurface::Get().GetSurface(), VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, queueSpecs, wantedPhysicalDeviceExtensions, deviceFeatures);
		}
	
		gladLoaderLoadVulkan(vkInstance, VulkanPhysicalDevice::Get().GetPhysicalDevice(), NULL);


		//Init logical device
		{
			VulkanLogicalDevice::Init(VulkanPhysicalDevice::Get(), wantedPhysicalDeviceExtensions, deviceFeatures);
		}


		gladLoaderLoadVulkan(vkInstance, VulkanPhysicalDevice::Get().GetPhysicalDevice(), VulkanLogicalDevice::Get().GetLogicalDevice());

		//Init Swap Chain

		SwapChainSpecification swapChainSpec{};
		swapChainSpec.ColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		swapChainSpec.PresentMode = VK_PRESENT_MODE_FIFO_KHR;
		swapChainSpec.SwapChainFormat = VK_FORMAT_R8G8B8A8_UNORM;
		swapChainSpec.SwapChainWidth = 1280;
		swapChainSpec.SwapChainHeight = 720;
		swapChainSpec.SwapChainImageCount = 4;
		swapChainSpec.Window = m_Context;


		VulkanSwapChain::Init(swapChainSpec);

	}
	

	void VulkanContext::InitPhysicalDevice()
	{
		
	}

	void VulkanContext::InitLogicalDevice()
	{

	}

	void VulkanContext::SwapBuffers()
	{
		glfwSwapBuffers(m_Context);
	}

	void VulkanContext::Shutdown() {

		VulkanSwapChain::Shutdown();

		VulkanLogicalDevice::Shutdown();
		VulkanPhysicalDevice::Shutdown();
		VulkanSurface::Shutdown();



#ifdef KD_DEBUG
		vkDestroyDebugUtilsMessengerEXT(VulkanInstance::Get()->GetVulkanInstance(), m_DebugMessenger, nullptr);
#endif

		VulkanInstance::Shutdown();
	}
	
}

