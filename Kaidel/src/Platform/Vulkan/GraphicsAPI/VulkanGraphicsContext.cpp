#include "KDpch.h"
#include "VulkanGraphicsContext.h"
#include "Kaidel/Core/Window.h"

//ImGui
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_dx11.h>




namespace Kaidel {



	namespace Utils {
		static VkDescriptorSetLayout CreateSingleDescriptorSetLayout(VkDevice device, VkDescriptorType type, uint32_t binding, uint32_t descriptorCount,VkShaderStageFlags flags) {
			VkDescriptorSetLayoutBinding setBinding{};
			setBinding.binding = binding;
			setBinding.descriptorCount = descriptorCount;
			setBinding.descriptorType = type;
			setBinding.stageFlags = flags;

			VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			layoutInfo.bindingCount = 1;
			layoutInfo.pBindings = &setBinding;
			VkDescriptorSetLayout layout{};
			vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout);
			return layout;
		}
	}

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

		#pragma region Instance
		gladLoaderLoadVulkan(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE);
		std::vector<const char*> extensions = m_Window->GetRequiredInstanceExtensions();
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		std::vector<const char*> layers;
		layers.push_back("VK_LAYER_KHRONOS_validation");
		m_Instance = CreateScope<VulkanInstance>("Vulkan Renderer", extensions, layers);

		#pragma endregion
		#pragma region Surface
		m_Surface = CreateScope<VulkanSurface>(m_Window->GetNativeWindow());
		#pragma endregion
		#pragma region PhysicalDevice
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
		#pragma endregion
		#pragma region LogicalDevice
		VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures feature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES,nullptr,VK_TRUE };
		VkPhysicalDeviceFeatures features{};
		features.geometryShader = true;
		features.tessellationShader = true;
		m_LogicalDevice = CreateScope<VulkanLogicalDevice>(*m_PhysicalDevice, std::vector<const char*>{ VK_KHR_SWAPCHAIN_EXTENSION_NAME },
																layers, features, &feature);

		gladLoaderLoadVulkan(m_Instance->GetInstance(), m_PhysicalDevice->GetDevice(), m_LogicalDevice->GetDevice());
		#pragma endregion
		m_Allocator = CreateScope<VulkanAllocator>();
		m_Backend = CreateScope<VulkanBackend::Backend>(m_LogicalDevice->GetDevice(), m_PhysicalDevice->GetDevice(), m_Allocator->GetAllocator());
		#pragma region Swapchain
		
		m_GlobalCommandPool = m_Backend->CreateCommandPool(m_PhysicalDevice->GetQueue("GraphicsQueue").FamilyIndex, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		{
			VulkanBackend::SurfaceInfo info{};
			info.Width = 1280;
			info.Height = 720;
			info.PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			info.Surface = m_Surface->GetSurface();
			m_Swapchain = m_Backend->CreateSwapchain(info, GetGraphicsQueue().Queue, m_GlobalCommandPool, 4);
		}

		#pragma endregion

		m_MaxFramesInFlight = std::min(m_Swapchain.ImageCount,2U);

		/*m_FramesData.resize(m_MaxFramesInFlight);
		


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
		}*/


		m_CurrentFrameNumber = 0;

		m_SingleSubmitFence = m_Backend->CreateFence();

		CreateImGuiDescriptorPool();
		
		/*{
			std::vector<DescriptorPoolSize> sizes;
			sizes.push_back({ DescriptorType::Sampler, 1000 });
			sizes.push_back({ DescriptorType::CombinedSampler, 1000 });
			sizes.push_back({ DescriptorType::Texture, 1000 });
			sizes.push_back({ DescriptorType::UniformBuffer, 1000 });
			sizes.push_back({ DescriptorType::StorageBuffer, 1000 });

			m_GlobalDescriptorPool = CreateScope<VulkanDescriptorPool>(sizes, 1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		}
		
		*/
		m_Stager = CreateScope<VulkanBufferStager>(500 * 1024 * 1024, 4);
	}

	VkDescriptorSetLayout VulkanGraphicsContext::GetSingleDescriptorSetLayout(VkDescriptorType type, VkShaderStageFlags flags)
	{
		auto& map = m_SingleSetLayouts[type];
		auto it = map.find(flags);
		if (it != map.end())
			return it->second;
		auto layout = Utils::CreateSingleDescriptorSetLayout(m_LogicalDevice->GetDevice(), type, 0, 1,flags);
		return map[flags] = layout;
	}

	void VulkanGraphicsContext::SwapBuffers()
	{
	}

	void VulkanGraphicsContext::Shutdown()
	{
		vkDeviceWaitIdle(m_LogicalDevice->GetDevice());
		m_ThreadsRunning = false;
		//for (auto& frame : m_FramesData) {
		//	frame.TaskConditionVariable->notify_one();
		//	frame.TaskWorker->join();
		//}

		for (auto& [type,descriptorMap] : m_SingleSetLayouts) {
			for (auto& [flags, layout] : descriptorMap) {
				vkDestroyDescriptorSetLayout(m_LogicalDevice->GetDevice(), layout, nullptr);
			}
		}

		//m_FramesData.clear();
	}

	void VulkanGraphicsContext::AcquireImage()
	{
		auto& frame = m_Swapchain.Frames[m_CurrentFrameNumber];
		m_Backend->FenceWait(frame.InFlightFence);
		//std::unique_lock<std::mutex> lock(*frame.TaskSyncMutex);

		VK_ASSERT(vkAcquireNextImageKHR(m_LogicalDevice->GetDevice(), m_Swapchain.Swapchain, UINT64_MAX, frame.ImageAvailable
			, VK_NULL_HANDLE, &m_Swapchain.ImageIndex));

		m_Backend->CommandBufferBegin(m_Swapchain.Frames[m_Swapchain.ImageIndex].MainCommandBuffer);
		m_Stager->Reset();
	}

	void VulkanGraphicsContext::PresentImage()
	{
		VkCommandBuffer commandBuffer = m_Swapchain.Frames[m_Swapchain.ImageIndex].MainCommandBuffer;
		m_Backend->CommandBufferEnd(commandBuffer);

		auto& frame = m_Swapchain.Frames[m_CurrentFrameNumber];
		VkSemaphore waitSemaphores[] = { frame.ImageAvailable };
		VkSemaphore signalSemaphores[] = { frame.RenderFinished };
		VkCommandBuffer commandBuffers[] = { commandBuffer };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffers;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_ASSERT(vkQueueSubmit(m_PhysicalDevice->GetQueue("GraphicsQueue").Queue, 1, &submitInfo, frame.InFlightFence));

		VkSwapchainKHR swapchain = m_Swapchain.Swapchain;

		VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.pImageIndices = &m_Swapchain.ImageIndex;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkResult presentResult = vkQueuePresentKHR(m_PhysicalDevice->GetQueue("PresentQueue").Queue, &presentInfo);
		bool needsResize = (presentResult == VK_SUBOPTIMAL_KHR) || (presentResult == VK_ERROR_OUT_OF_DATE_KHR);

		if (needsResize) {
			vkDeviceWaitIdle(m_LogicalDevice->GetDevice());
			m_Backend->DestroySwapchain(m_Swapchain);
			VulkanBackend::SurfaceInfo info{};
			info.Surface = m_Surface->GetSurface();
			info.Width = m_Window->GetWidth();
			info.Height = m_Window->GetHeight();
			info.PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			m_Swapchain = m_Backend->CreateSwapchain(info, GetPresentQueue().Queue, m_GlobalCommandPool, 4);
			//m_Swapchain->Resize(m_Window->GetWidth(), m_Window->GetHeight());
		}

		//m_FramesData[m_CurrentFrameNumber].TasksReady = true;
		//m_FramesData[m_CurrentFrameNumber].TaskConditionVariable->notify_one();

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
		info.ImageCount = m_Swapchain.ImageCount;
		info.MinImageCount = m_Swapchain.ImageCount;
		info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		auto& graphicsQueue = m_PhysicalDevice->GetQueue("GraphicsQueue");
		info.Queue = graphicsQueue.Queue;
		info.QueueFamily = graphicsQueue.FamilyIndex;
		info.Subpass = 0;
		info.DescriptorPool = m_ImGuiDescriptorPool;

		ImGui_ImplVulkan_Init(&info, m_Swapchain.RenderPass);

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
		VkCommandBuffer commandBuffer = GetCurrentCommandBuffer();
		VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassInfo.renderPass = m_Swapchain.RenderPass;
		renderPassInfo.framebuffer = m_Swapchain.Framebuffers[m_Swapchain.ImageIndex];
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;
		renderPassInfo.renderArea.extent = { m_Swapchain.Extent.width, m_Swapchain.Extent.height };

		VkClearValue clearColor{};
		clearColor.color = { 1.0f,1.0f,1.0f,1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		renderPassInfo.renderPass = m_Swapchain.RenderPass;

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
		std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> sizes;

		sizes[VK_DESCRIPTOR_TYPE_SAMPLER] = { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 };
		sizes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 };
		sizes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 };
		sizes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 };
		sizes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 };

		m_ImGuiDescriptorPool = m_Backend->CreateDescriptorPool(sizes, 1000);
	}

	void VulkanGraphicsContext::LoadImGuiFonts()const
	{
		// Use any command queue
		VkCommandBuffer cb = m_Backend->CreateCommandBuffer(m_GlobalCommandPool);
		m_Backend->CommandBufferBegin(cb);

		ImGui_ImplVulkan_CreateFontsTexture(cb);

		m_Backend->CommandBufferEnd(cb);

		m_Backend->SubmitCommandBuffers(GetGraphicsQueue().Queue, { cb },m_SingleSubmitFence);
		m_Backend->FenceWait(m_SingleSubmitFence);
		m_Backend->DestroyCommandBuffer(cb, m_GlobalCommandPool);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

}
