#pragma once


#include "VulkanBase.h"

#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"
#include "Kaidel/Core/Application.h"

#include "VulkanSwapchain.h"

#include <vector>
#include <string>
#include <optional>
#include <set>

struct GLFWwindow;


#define VK_DEVICE ((::Kaidel::Vulkan::VulkanGraphicsContext*)(::Kaidel::Application::Get().GetWindow().GetContext().get()))->GetDevice()

namespace Kaidel {
	namespace Vulkan {
	
		


		class VulkanGraphicsContext : public GraphicsContext{
		public:
			VulkanGraphicsContext(GLFWwindow* window);

			void Init() override;
			void SwapBuffers() override;
			void Shutdown() override;

			VkDevice GetDevice()const { return m_LogicalDevice; }
			const DeviceQueues& GetDeviceQueues()const { return m_DeviceQueues; }

		private:
			GLFWwindow* m_Window = nullptr;

			//Instance
			InstanceSpecification m_InstanceSpecification = {};
			uint32_t m_VulkanAPIVersion = 0;
			VkInstance m_Instance = VK_NULL_HANDLE;
			
			//Debug messenger
			VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

			//Surface
			VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
			
			//PhysicalDevice
			PhysicalDeviceSpecification m_PhysicalDeviceSpecification = {};
			QueueFamilyIndices m_QueueFamilyIndices = {};
			VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

			//LogicalDevice
			LogicalDeviceSpecification m_LogicalDeviceSpecification = {};
			DeviceQueues m_DeviceQueues = {};
			VkDevice m_LogicalDevice = VK_NULL_HANDLE;

			//Swapchain
			Ref<VulkanSwapchain> m_Swapchain = {};

		};
	}

}

