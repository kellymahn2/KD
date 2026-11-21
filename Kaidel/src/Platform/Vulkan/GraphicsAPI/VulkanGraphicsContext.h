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

#define VK_GRAPHICS_Q VK_CONTEXT.GetGraphicsQueue()
#define VK_TRANSFER_Q VK_CONTEXT.GetTransferQueue()

namespace Kaidel{
	class VulkanGraphicsContext : public GraphicsContext {
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

		VkCommandBuffer GetCurrentCommandBuffer()const 
		{ 
			return (*m_OverrideCommandBuffers).empty() ? 
				m_Swapchain.Frames[m_CurrentFrameNumber].MainCommandBuffer : 
				(*m_OverrideCommandBuffers).back(); 
		}

		void AcquireImage() override;
		void PresentImage() override;

		uint32_t GetFramesInFlightCount()const { return m_MaxFramesInFlight; }
		uint32_t GetCurrentFrameNumber()const { return m_CurrentFrameNumber; }
		void SubmitDeferredDelete(std::function<void()>&& func)
		{
			m_Swapchain.Frames[m_CurrentFrameNumber].DeferredDeletes.emplace(func);
		}

		void SubmitSingle(VulkanQueue& queue, std::function<void(VkCommandBuffer)>&& func);

		VkDescriptorSetLayout GetSingleDescriptorSetLayout(VkDescriptorType type, VkShaderStageFlags flags);

		const VulkanBackend::CommandPoolInfo& GetPrimaryCommandPool(VulkanQueue& queue);

		VkFence GetSingleSubmitFence(VulkanQueue& queue);

		auto& GetQueue(const std::string& name) { return m_PhysicalDevice->GetQueue(name); }
		auto& GetGraphicsQueue() { return GetQueue("GraphicsQueue"); }
		auto& GetPresentQueue() { return GetQueue("PresentQueue"); }
		auto& GetTransferQueue() { return GetQueue("TransferQueue"); }

		virtual uint32_t GetMaxFramesInFlightCount()const override { return m_MaxFramesInFlight;}
		virtual uint32_t GetCurrentFrameIndex()const override { return m_CurrentFrameNumber; }

		void PushOverrideCommandBuffer(VkCommandBuffer cb) { (*m_OverrideCommandBuffers).push_back(cb); }

		void PopOverrideCommandBuffer() { (*m_OverrideCommandBuffers).pop_back(); }

		//ImGui callbacks
		void ImGuiInit()const override;
		void ImGuiBegin()const override;
		void ImGuiEnd()const override;
		void ImGuiShutdown()const override;
	private:
		void CreateImGuiDescriptorPool();
		void LoadImGuiFonts() const;
	private:

		static VulkanGraphicsContext* s_GraphicsContext;
		Scope<VulkanInstance> m_Instance;
		Scope<VulkanSurface> m_Surface;
		Scope<VulkanPhysicalDevice> m_PhysicalDevice;
		Scope<VulkanLogicalDevice> m_LogicalDevice;
		Scope<VulkanAllocator> m_Allocator;

		VulkanBackend::SwapchainInfo m_Swapchain;

		Window* m_Window;

		TLS<std::vector<VkCommandBuffer>> m_OverrideCommandBuffers;

		//std::vector<PerSwapchainFrameData> m_FramesData;

		uint32_t m_MaxFramesInFlight = 0;
		uint32_t m_CurrentFrameNumber = 0;

		bool m_ThreadsRunning = true;

		TLS<VulkanBackend::CommandPoolInfo> m_GlobalCommandPool;
		TLS<VkFence> m_SingleSubmitFence;

		VkDescriptorPool m_ImGuiDescriptorPool;

		Scope<VulkanBackend::Backend> m_Backend;

		Scope<VulkanBufferStager> m_Stager;

		std::unordered_map<VkDescriptorType, std::unordered_map<VkShaderStageFlags,VkDescriptorSetLayout>> m_SingleSetLayouts;
	};
}
