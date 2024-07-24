#include "KDpch.h"
#include "VulkanGraphicsContext.h"
#include "Kaidel/Core/Window.h"

//ImGui
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_dx11.h>



#include "VulkanRenderPass.h"


namespace Kaidel {




	VulkanGraphicsContext* VulkanGraphicsContext::s_GraphicsContext;

	
	

	uint32_t GetFramesInFlightCount() {
		return VK_CONTEXT.GetFramesInFlightCount();
	}
	uint32_t GetCurrentFrameNumber() {
		return VK_CONTEXT.GetCurrentFrameNumber();
	}

	
	VulkanGraphicsContext::VulkanGraphicsContext(Window* window)
		:m_Window(window)
	{
	}

	void VulkanGraphicsContext::Init()
	{
		KD_CORE_ASSERT(!s_GraphicsContext);
		s_GraphicsContext = this;

		gladLoaderLoadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
		std::vector<const char*> extensions = m_Window->GetRequiredInstanceExtensions();
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		std::vector<const char*> layers;
		layers.push_back("VK_LAYER_KHRONOS_validation");
		m_Instance = CreateScope<VulkanInstance>("Vulkan Renderer", extensions, layers);


		m_Surface = CreateScope<VulkanSurface>(m_Window->GetNativeWindow());

		VulkanQueueGroupSpecification groupSpec{};
		{
			{
				VulkanQueueSpecification spec{};
				spec.Validator = [](const VkQueueFamilyProperties& queueProps, uint32_t familyIndex, VkPhysicalDevice device) {
					return queueProps.queueFlags & VK_QUEUE_GRAPHICS_BIT;
					};
				groupSpec.QueueSpecifications["GraphicsQueue"] = spec;
			}
			{
				VulkanQueueSpecification spec{};
				spec.Validator = [](const VkQueueFamilyProperties& queueProps, uint32_t familyIndex, VkPhysicalDevice device) {
					return queueProps.queueFlags & VK_QUEUE_TRANSFER_BIT;
					};
				groupSpec.QueueSpecifications["TransferQueue"] = spec;
			}
			{
				VulkanQueueSpecification spec{};
				spec.Validator = [this](const VkQueueFamilyProperties& queueProps, uint32_t familyIndex, VkPhysicalDevice device) {
					VkBool32 supported;
					VK_ASSERT(vkGetPhysicalDeviceSurfaceSupportKHR(device, familyIndex, m_Surface->GetSurface(), &supported));
					return static_cast<bool>(supported);
				};
				groupSpec.QueueSpecifications["PresentQueue"] = spec;
			}
		}

		m_PhysicalDevice = CreateScope<VulkanPhysicalDevice>(*m_Instance, std::vector<VulkanQueueGroupSpecification>{ groupSpec });

		VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures feature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES,nullptr,VK_TRUE };

		m_LogicalDevice = CreateScope<VulkanLogicalDevice>(*m_PhysicalDevice, std::vector<const char*>{ VK_KHR_SWAPCHAIN_EXTENSION_NAME },
																layers, VkPhysicalDeviceFeatures{}, &feature);


		gladLoaderLoadVulkan(m_Instance->GetInstance(), m_PhysicalDevice->GetDevice(), m_LogicalDevice->GetDevice());

		m_Swapchain = CreateScope<VulkanSwapchain>(m_Surface->GetSurface(),
			VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM,VK_COLORSPACE_SRGB_NONLINEAR_KHR }, VK_PRESENT_MODE_MAILBOX_KHR,
			1280, 720, 4, m_PhysicalDevice->GetQueueManager().GetUniqueFamilyIndices());


		m_MaxFramesInFlight = m_Swapchain->GetImageCount();

		m_FramesData.resize(m_MaxFramesInFlight);
		
		m_CommandPool = CreateRef<VulkanCommandPool>(CommandPoolOperationType::Graphics,CommandPoolFlags_CommandBufferReset);

		for (auto& frame : m_FramesData) {
		
			frame.CommandBuffer = CreateRef<VulkanCommandBuffer>(m_CommandPool,CommandBufferType::Primary,0);
			frame.RenderFinished =  CreateRef<VulkanSemaphore>();
			frame.ImageAvailable =  CreateRef<VulkanSemaphore>();
			frame.InFlightFence = CreateRef<VulkanFence>(VK_FENCE_CREATE_SIGNALED_BIT);

			frame.TaskSyncMutex = std::make_shared<std::mutex>();

			frame.TaskConditionVariable = std::make_shared<std::condition_variable>();
			frame.TaskWorker = std::make_shared<std::thread>([this, &frame]() {

				while (m_ThreadsRunning) {
					std::unique_lock<std::mutex> lock(*frame.TaskSyncMutex);

					frame.TaskConditionVariable->wait(lock, [this, &frame]() {return frame.TasksReady || !m_ThreadsRunning; });
					if (!m_ThreadsRunning) break;

					frame.InFlightFence->Wait();
					while (!frame.Tasks.empty()) {
						frame.Tasks.front()();
						frame.Tasks.pop();
					}

					frame.TasksReady = false;
				}
			});
		}

		m_Allocator = CreateScope<VulkanAllocator>();

		m_CurrentFrameNumber = 0;

		CreateImGuiDescriptorPool();

		m_UniformBufferDescriptorPool = CreateScope<VulkanDescriptorPool>(std::vector<VkDescriptorPoolSize>{ VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000} }, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
	
		for (uint32_t i = 0; i < 32; ++i) {
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = i;
			binding.descriptorCount = 1;
			binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			VkDescriptorSetLayoutCreateInfo setInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			setInfo.bindingCount = 1;
			setInfo.pBindings = &binding;
			VkDescriptorSetLayout layout{};
			
			VK_ASSERT(vkCreateDescriptorSetLayout(m_LogicalDevice->GetDevice(), &setInfo, nullptr, &layout));
			m_UniformBufferDescriptorSetLayouts.push_back(layout);
		}

		m_BufferStager = CreateScope<VulkanBufferStager>(10 * 1024 * 1024, 4);
	}

	void VulkanGraphicsContext::SwapBuffers()
	{
	}

	void VulkanGraphicsContext::Shutdown()
	{
		vkDeviceWaitIdle(m_LogicalDevice->GetDevice());
		m_ThreadsRunning = false;
		for (auto& frame : m_FramesData) {
			frame.TaskConditionVariable->notify_one();
			frame.TaskWorker->join();
		}

		m_FramesData.clear();
	}

	void VulkanGraphicsContext::AcquireImage()
	{
		auto& frame = m_FramesData[m_CurrentFrameNumber];
		frame.InFlightFence->Wait();
		std::unique_lock<std::mutex> lock(*frame.TaskSyncMutex);
		frame.InFlightFence->Reset();

		VK_ASSERT(vkAcquireNextImageKHR(m_LogicalDevice->GetDevice(), m_Swapchain->GetSwapchain(), UINT64_MAX, frame.ImageAvailable->GetSemaphore()
			, VK_NULL_HANDLE, &m_AcquiredImage));
		m_FramesData[m_AcquiredImage].CommandBuffer->Begin(0);
		m_BufferStager->Reset();
	}

	void VulkanGraphicsContext::PresentImage()
	{
		m_FramesData[m_AcquiredImage].CommandBuffer->End();

		auto& frame = m_FramesData[m_CurrentFrameNumber];
		VkSemaphore waitSemaphores[] = { frame.ImageAvailable->GetSemaphore() };
		VkSemaphore signalSemaphores[] = { frame.RenderFinished->GetSemaphore() };
		VkCommandBuffer commandBuffer[] = { m_FramesData[m_AcquiredImage].CommandBuffer->GetCommandBuffer() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffer;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_ASSERT(vkQueueSubmit(m_PhysicalDevice->GetQueue("GraphicsQueue").Queue, 1, &submitInfo, frame.InFlightFence->GetFence()));

		VkSwapchainKHR swapchain = m_Swapchain->GetSwapchain();

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pImageIndices = &m_AcquiredImage;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VK_ASSERT(vkQueuePresentKHR(m_PhysicalDevice->GetQueue("PresentQueue").Queue, &presentInfo));
		m_FramesData[m_CurrentFrameNumber].TasksReady = true;
		m_FramesData[m_CurrentFrameNumber].TaskConditionVariable->notify_one();

		m_CurrentFrameNumber = (m_CurrentFrameNumber + 1) % m_MaxFramesInFlight;
	}

	void VulkanGraphicsContext::ImGuiInit()const
	{
#ifdef KD_PLATFORM_WINDOWS
		ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(m_Window->GetNativeWindow()), true);
#endif // KD_PLATFORM_WINDOWS

		ImGui_ImplVulkan_InitInfo info{};
		info.Allocator = nullptr;
		info.Instance = m_Instance->GetInstance();
		info.PhysicalDevice = m_PhysicalDevice->GetDevice();
		info.Device = m_LogicalDevice->GetDevice();
		info.ImageCount = m_Swapchain->GetImageCount();
		info.MinImageCount = m_Swapchain->GetImageCount();
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		auto& graphicsQueue = m_PhysicalDevice->GetQueue("GraphicsQueue");
		info.Queue = graphicsQueue.Queue;
		info.QueueFamily = graphicsQueue.FamilyIndex;
		info.Subpass = 0;
		info.DescriptorPool = m_ImGuiDescriptorPool->GetDescriptorPool();

		ImGui_ImplVulkan_Init(&info, m_Swapchain->GetRenderPass());

		LoadImGuiFonts();
	}
	void VulkanGraphicsContext::ImGuiBegin()const
	{
		ImGui_ImplVulkan_NewFrame();
#ifdef KD_PLATFORM_WINDOWS
		ImGui_ImplGlfw_NewFrame();
#endif // KD_PLATFORM_WINDOWS

	}

	void VulkanGraphicsContext::ImGuiEnd()const
	{
		VkCommandBuffer commandBuffer = GetActiveCommandBuffer()->GetCommandBuffer();
		VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassInfo.renderPass = m_Swapchain->GetRenderPass();
		renderPassInfo.framebuffer = m_Swapchain->GetFrames()[m_AcquiredImage].Framebuffer;
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = { m_Swapchain->GetExtent().width, m_Swapchain->GetExtent().height };

		VkClearValue clearColor{};
		clearColor.color = { 1.0f,1.0f,1.0f,1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		renderPassInfo.renderPass = m_Swapchain->GetRenderPass();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		vkCmdEndRenderPass(commandBuffer);
	}

	void VulkanGraphicsContext::ImGuiShutdown()const 
	{
		ImGui_ImplVulkan_Shutdown();
	}

	void VulkanGraphicsContext::CreateImGuiDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 });
		sizes.push_back({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 });

		m_ImGuiDescriptorPool = CreateScope<VulkanDescriptorPool>(sizes, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
	}

	void VulkanGraphicsContext::LoadImGuiFonts()const
	{
		// Use any command queue
		VkCommandBuffer cb = m_CommandPool->BeginSingleTimeCommands(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		ImGui_ImplVulkan_CreateFontsTexture(cb);


		m_CommandPool->EndSingleTimeCommands(cb, m_PhysicalDevice->GetQueue("GraphicsQueue").Queue);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

}
