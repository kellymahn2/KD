#pragma once


#include "VulkanBase.h"

#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"
#include "Kaidel/Core/Application.h"
#include "Initialization/VulkanQueueManager.h"
#include "VulkanCommandPool.h"

#include "Initialization/VulkanSwapchain.h"

#include <vector>
#include <string>
#include <optional>
#include <set>

struct GLFWwindow;


#define VK_CONTEXT ::Kaidel::Vulkan::VulkanGraphicsContext::Get()

#define VK_DEVICE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetDevice()
#define VK_INSTANCE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetInstance()
#define VK_PHYSICAL_DEVICE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetPhysicalDevice()
#define VK_UNIQUE_INDICES ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetUniqueFamilyIndices()
#define VK_CURRENT_IMAGE ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetCurrentFrame()
#define VK_DEVICE_QUEUE(name) ::Kaidel::Vulkan::VulkanGraphicsContext::Get().GetQueue(name)
//#define VK_DEVICE ::Kaidel::Vulkan::VulkanGraphicsContext::GetDevice()

struct ImDrawData;

namespace Kaidel {
	namespace Vulkan {
	
		//50 mb
		static constexpr const uint32_t GlobalStagingBufferSize = 50 * 1024 * 1024;


		class VulkanGraphicsContext : public GraphicsContext{
		public:

			VulkanGraphicsContext(GLFWwindow* window);

			static VulkanGraphicsContext& Get() { return *s_GraphicsContext; }

			void Init() override;
			void SwapBuffers() override;
			void FlushCommandBuffers();
			void Shutdown() override;
			void OnResize(uint32_t width,uint32_t height);

			//Instance
			VkInstance GetInstance() const{ return m_Instance; }
			
			const VulkanQueue& GetQueue(const std::string& name)const { return m_QueueManager[name]; }

			//Surface
			VkSurfaceKHR GetSurface()const { return m_Surface; }

			//PhysicalDevice
			VkPhysicalDevice GetPhysicalDevice()const { return m_PhysicalDevice; }
			VulkanQueueManager& GetQueueManager() { return m_QueueManager; }

			//LogicalDevice
			VkDevice GetDevice()const { return m_LogicalDevice; }

			const auto& GetUniqueFamilyIndices()const { return m_UniqueQueueFamilyIndices; }

			VkCommandBuffer GetGraphicsCommandBuffer() { return m_GraphicsCommandBuffers[m_AcquiredImage]; }

			
			VkCommandBuffer GetMainCommandBuffer()const { return m_MainCommandBuffer; }

			//Swapchain
			Ref<VulkanSwapchain> GetSwapchain()const { return m_Swapchain; }
			const auto& GetCurrentFrame()const { return m_Swapchain->GetFrames()[m_CurrentFrame]; }
			uint32_t GetCurrentFrameIndex()const { return m_AcquiredImage; }

			//Command pools
			Ref<VulkanCommandPool> GetTransferCommandPool()const { return m_TransferCommandPool; }


			//Global staging buffer
			const VulkanBuffer& GetGlobalStagingBuffer()const { return m_GlobalStagingBuffer; }

		private:

			void StartSwapchain();
			void Present();

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

			//MainCommandBuffer
			VkCommandBuffer m_MainCommandBuffer = VK_NULL_HANDLE;

			//Frames in flight
			uint32_t m_MaxFramesInFlight;
			uint32_t m_CurrentFrame;
			uint32_t m_AcquiredImage;

			//Ref<VulkanCommandBuffer> m_GraphicsCommandBuffer;

			std::vector<uint32_t> m_UniqueQueueFamilyIndices;

			static VulkanGraphicsContext* s_GraphicsContext;
			bool m_SingleFramePassed = false;

			//Command pools
			Ref<VulkanCommandPool> m_TransferCommandPool;

			//Global staging buffer
			VulkanBuffer m_GlobalStagingBuffer;

			std::vector<VkCommandBuffer> m_GraphicsCommandBuffers;


			friend void ImGuiInit();
			friend void ImGuiRender(ImDrawData*);
			friend void ImGuiNewFrame();
			friend void ImGuiShutdown();

		};
	}

}

