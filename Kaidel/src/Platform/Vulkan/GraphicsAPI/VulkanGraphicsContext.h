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
#define VK_INSTANCE ((::Kaidel::Vulkan::VulkanGraphicsContext*)(::Kaidel::Application::Get().GetWindow().GetContext().get()))->GetInstance()
#define VK_PHYSICAL_DEVICE ((::Kaidel::Vulkan::VulkanGraphicsContext*)(::Kaidel::Application::Get().GetWindow().GetContext().get()))->GetPhysicalDevice()

//#define VK_DEVICE ::Kaidel::Vulkan::VulkanGraphicsContext::GetDevice()

struct ImDrawData;

namespace Kaidel {
	namespace Vulkan {
	
		class VulkanGraphicsContext : public GraphicsContext{
		public:
			VulkanGraphicsContext(GLFWwindow* window);

			void Init() override;
			void SwapBuffers() override;
			void FlushCommandBuffers();
			void Shutdown() override;
			void OnResize(uint32_t width,uint32_t height);


			VkInstance GetInstance() const{ return m_Instance; }
			VkPhysicalDevice GetPhysicalDevice()const { return m_PhysicalDevice; }
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

			uint32_t CurrentImage = -1;

			friend void ImGuiInit();
			friend void ImGuiRender(ImDrawData*);
			friend void ImGuiNewFrame();
			friend void ImGuiShutdown();

		};
	}

}

