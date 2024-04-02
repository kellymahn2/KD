#pragma once


#include "Kaidel/Core/Base.h"


#include <glad/vulkan.h>

namespace Kaidel {
	class VulkanInstance {
	public:

		static void Init(const VkApplicationInfo& applicationInfo,const std::vector<const char*>& extensions,const std::vector<const char*>& layers);

		static const auto& Get() { return s_Instance; }

		VkInstance GetVulkanInstance()const { return m_VkInstance; }
		const auto& GetEnabledExtensions()const { return m_EnabledExtensions; }
		const auto& GetAcceptedValidationLayers()const { return m_ValidationLayers; }
		const auto& GetApplicationInfo()const { return m_Info; }

		static void Shutdown();

	private:
		VkInstance m_VkInstance;

		std::vector<std::string> m_EnabledExtensions;
		std::vector<std::string> m_ValidationLayers;
		VkApplicationInfo m_Info;

		static Scope<VulkanInstance> s_Instance;
	};
}
