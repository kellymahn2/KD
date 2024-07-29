#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"


#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanSurface.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanAllocator.h"
#include "VulkanDescriptorPool.h"
#include "VulkanStager.h"


#define VK_CONTEXT ::Kaidel::VulkanGraphicsContext::Get()
#define VK_INSTANCE VK_CONTEXT.GetInstance()
#define VK_PHYSICAL_DEVICE VK_CONTEXT.GetPhysicalDevice()
#define VK_DEVICE VK_CONTEXT.GetLogicalDevice()
#define VK_ALLOCATOR VK_CONTEXT.GetAllocator()

namespace Kaidel {


	struct PerSwapchainFrameData {
		Ref<VulkanCommandBuffer> CommandBuffer;

		Ref<VulkanSemaphore> RenderFinished;
		Ref<VulkanSemaphore> ImageAvailable;
		Ref<VulkanFence> InFlightFence;

		std::shared_ptr<std::mutex> TaskSyncMutex;
		std::shared_ptr<std::condition_variable> TaskConditionVariable;
		std::shared_ptr<std::thread> TaskWorker;
		bool TasksReady = false;
		std::queue<std::function<void()>> Tasks;

		~PerSwapchainFrameData() {
			TaskWorker = {};
			TaskConditionVariable = {};
			TaskSyncMutex = {};

		}
	};

	class VulkanGraphicsContext : public GraphicsContext{
	public:

		VulkanGraphicsContext(Window* window);
		void Init() override;
		void SwapBuffers() override;
		void Shutdown() override;

		static VulkanGraphicsContext& Get() { return *s_GraphicsContext; }

		VulkanInstance& GetInstance() { return *m_Instance; }
		VulkanPhysicalDevice& GetPhysicalDevice() { return *m_PhysicalDevice; }  
		VulkanLogicalDevice& GetLogicalDevice() { return *m_LogicalDevice; }  
		VulkanSwapchain& GetSwapchain() { return *m_Swapchain; }
		VulkanAllocator& GetAllocator() { return *m_Allocator; }

		VulkanBufferStager& GetBufferStager() { return *m_BufferStager; }


		void AcquireImage() override;
		void PresentImage() override;

		uint32_t GetFramesInFlightCount()const { return m_MaxFramesInFlight; }
		uint32_t GetCurrentFrameNumber()const { return m_CurrentFrameNumber; }

		VulkanDescriptorPool& GetGlobalDescriptorPool() { return *m_GlobalDescriptorPool; }

		Ref<VulkanCommandBuffer> GetActiveCommandBuffer()const { return m_FramesData[m_AcquiredImage].CommandBuffer; }


		VkDescriptorSetLayout GetSingleDescriptorSetLayout(VkDescriptorType type, VkShaderStageFlags flags);

		//ImGui callbacks
		void ImGuiInit()const override;
		void ImGuiBegin()const override;
		void ImGuiEnd()const override;
		void ImGuiShutdown()const override;
	private:


		void CreateImGuiDescriptorPool();
		void LoadImGuiFonts()const;
	private:

		static VulkanGraphicsContext* s_GraphicsContext;
		Scope<VulkanInstance> m_Instance;
		Scope<VulkanSurface> m_Surface;
		Scope<VulkanPhysicalDevice> m_PhysicalDevice;
		Scope<VulkanLogicalDevice> m_LogicalDevice;
		Scope<VulkanSwapchain> m_Swapchain;
		Scope<VulkanAllocator> m_Allocator;

		Window* m_Window;

		std::vector<PerSwapchainFrameData> m_FramesData;

		uint32_t m_MaxFramesInFlight = 0;
		uint32_t m_CurrentFrameNumber = 0;
		uint32_t m_AcquiredImage = 0;

		bool m_ThreadsRunning = true;

		Ref<VulkanCommandPool> m_CommandPool;

		Scope<VulkanDescriptorPool> m_ImGuiDescriptorPool;
		Scope<VulkanDescriptorPool> m_GlobalDescriptorPool;
		Scope<VulkanBufferStager> m_BufferStager;

		std::unordered_map<VkDescriptorType, std::unordered_map<VkShaderStageFlags,VkDescriptorSetLayout>> m_SingleSetLayouts;
	};
}
