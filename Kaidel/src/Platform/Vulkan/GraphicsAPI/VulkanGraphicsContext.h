#pragma once


#include "VulkanBase.h"

#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"
#include "Kaidel/Core/Application.h"
#include "VulkanQueueManager.h"

#include "VulkanSwapchain.h"

#include <vector>
#include <string>
#include <optional>
#include <set>

struct GLFWwindow;


#define VK_DEVICE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetDevice()
#define VK_INSTANCE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetInstance()
#define VK_PHYSICAL_DEVICE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetPhysicalDevice()
#define VK_UNIQUE_INDICES ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetUniqueFamilyIndices()
#define VK_CURRENT_IMAGE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetCurrentFrame()
#define VK_MAIN_COMMAND_BUFFER ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetMainCommandBuffer()
#define VK_DEVICE_QUEUE(name) ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetQueue(name)
//#define VK_DEVICE ::Kaidel::Vulkan::VulkanGraphicsContext::GetDevice()

struct ImDrawData;

namespace Kaidel {
	namespace Vulkan {
	
		class VulkanGraphicsContext : public GraphicsContext{
		public:

			VulkanGraphicsContext(GLFWwindow* window);

			static VulkanGraphicsContext& Get() { return *s_GraphicsContext; }


			void Init() override;
			void SwapBuffers() override;
			void FlushCommandBuffers();
			void Shutdown() override;
			void OnResize(uint32_t width,uint32_t height);

			VkInstance GetInstance() const{ return m_Instance; }
			VkPhysicalDevice GetPhysicalDevice()const { return m_PhysicalDevice; }

			const VulkanQueue& GetQueue(const std::string& name)const { return m_QueueManager[name]; }

			VkDevice GetDevice()const { return m_LogicalDevice; }
			const auto& GetUniqueFamilyIndices()const { return m_UniqueQueueFamilyIndices; }
			VkCommandBuffer GetMainCommandBuffer()const { return m_MainCommandBuffer; }

			const auto& GetCurrentFrame()const { return m_Swapchain->GetFrames()[CurrentImage]; }

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
			VulkanQueueManager m_QueueManager;
			VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;

			//LogicalDevice
			LogicalDeviceSpecification m_LogicalDeviceSpecification = {};
			VkDevice m_LogicalDevice = VK_NULL_HANDLE;

			//Swapchain
			Ref<VulkanSwapchain> m_Swapchain = {};
			uint32_t CurrentImage = -1;

			//MainCommandBuffer
			VkCommandBuffer m_MainCommandBuffer = VK_NULL_HANDLE;


			std::vector<uint32_t> m_UniqueQueueFamilyIndices;

			static VulkanGraphicsContext* s_GraphicsContext;


			friend void ImGuiInit();
			friend void ImGuiRender(ImDrawData*);
			friend void ImGuiNewFrame();
			friend void ImGuiShutdown();

		};
	}

}

