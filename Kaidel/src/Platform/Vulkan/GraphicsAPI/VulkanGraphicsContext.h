#pragma once

#include "Platform/Vulkan/VulkanDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"


#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanLogicalDevice.h"
#include "VulkanSurface.h"
#include "VulkanAllocator.h"
#include "VulkanStager.h"
#include "Backend.h"


#define VK_CONTEXT ::Kaidel::VulkanGraphicsContext::Get()
#define VK_INSTANCE VK_CONTEXT.GetInstance()
#define VK_PHYSICAL_DEVICE VK_CONTEXT.GetPhysicalDevice()
#define VK_DEVICE VK_CONTEXT.GetLogicalDevice()
#define VK_ALLOCATOR VK_CONTEXT.GetAllocator()
#define VK_BACKEND VK_CONTEXT.GetBackend()
#define VK_CURRENT_COMMAND_BUFFER VK_CONTEXT.GetCurrentCommandBuffer()

namespace Kaidel {


	/*struct PerSwapchainFrameData {
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
	};*/

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
		VulkanAllocator& GetAllocator() { return *m_Allocator; }

		const Scope<VulkanBackend::Backend>& GetBackend()const { return m_Backend; }

		VulkanBufferStager& GetBufferStager() { return *m_Stager;  }

		VkCommandBuffer GetCurrentCommandBuffer()const { return m_Swapchain.Frames[m_CurrentFrameNumber].MainCommandBuffer; }


		void AcquireImage() override;
		void PresentImage() override;

		uint32_t GetFramesInFlightCount()const { return m_MaxFramesInFlight; }
		uint32_t GetCurrentFrameNumber()const { return m_CurrentFrameNumber; }


		VkDescriptorSetLayout GetSingleDescriptorSetLayout(VkDescriptorType type, VkShaderStageFlags flags);

		const VulkanBackend::CommandPoolInfo& GetPrimaryCommandPool()const { return m_GlobalCommandPool; }
		VkFence GetSingleSubmitFence() { return m_SingleSubmitFence; }


		auto& GetQueue(const std::string& name)const { return m_PhysicalDevice->GetQueue(name); }
		auto& GetGraphicsQueue()const { return GetQueue("GraphicsQueue"); }
		auto& GetPresentQueue()const { return GetQueue("PresentQueue"); }

		virtual uint32_t GetMaxFramesInFlightCount()const override { return m_MaxFramesInFlight;}
		virtual uint32_t GetCurrentFrameIndex()const override { return m_CurrentFrameNumber; }

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
		Scope<VulkanAllocator> m_Allocator;

		VulkanBackend::SwapchainInfo m_Swapchain;

		Window* m_Window;

		//std::vector<PerSwapchainFrameData> m_FramesData;

		uint32_t m_MaxFramesInFlight = 0;
		uint32_t m_CurrentFrameNumber = 0;

		bool m_ThreadsRunning = true;

		VulkanBackend::CommandPoolInfo m_GlobalCommandPool;

		VkDescriptorPool m_ImGuiDescriptorPool;

		Scope<VulkanBackend::Backend> m_Backend;

		VkFence m_SingleSubmitFence;

		Scope<VulkanBufferStager> m_Stager;

		std::unordered_map<VkDescriptorType, std::unordered_map<VkShaderStageFlags,VkDescriptorSetLayout>> m_SingleSetLayouts;
	};
}
