#include "KDpch.h"
#include "VulkanInstance.h"



namespace Kaidel {

	Scope<VulkanInstance> VulkanInstance::s_Instance = {};


	static bool CheckValidationLayerSupport(const std::vector<VkLayerProperties>& availableLayers, const char* layer) {
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

	static auto FilterAvailLayers(const std::vector<const char*>& layers) {
		std::vector<const char*> actualLayers;

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : layers) {
			if (CheckValidationLayerSupport(availableLayers, layerName))
				actualLayers.push_back(layerName);
		}

		return actualLayers;
	}

	static auto TurnToStrings(const std::vector<const char*>& vec) {
		std::vector<std::string> strs;
		for (auto& str : vec) {
			strs.push_back(std::string(str));
		}
		return strs;
	}



	void VulkanInstance::Init(const VkApplicationInfo& applicationInfo, const std::vector<const char*>& extensions, const std::vector<const char*>& layers)
	{
		s_Instance = CreateScope<VulkanInstance>();

		auto actualLayers = FilterAvailLayers(layers);
		
		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &applicationInfo;

		//Extensions 
		instanceInfo.enabledExtensionCount = extensions.size();
		instanceInfo.ppEnabledExtensionNames = extensions.data();

		//Layers
		instanceInfo.enabledLayerCount = actualLayers.size();
		instanceInfo.ppEnabledLayerNames = actualLayers.data();
		
		KD_CORE_ASSERT(vkCreateInstance(&instanceInfo, nullptr, &s_Instance->m_VkInstance) == VK_SUCCESS);

		s_Instance->m_EnabledExtensions = TurnToStrings(extensions);
		s_Instance->m_ValidationLayers = TurnToStrings(actualLayers);
		s_Instance->m_Info = applicationInfo;




	}

	void VulkanInstance::Shutdown()
	{
		vkDestroyInstance(s_Instance->m_VkInstance, nullptr);
	}

}
