#include "KDpch.h"
#include "Backend.h"
#include "VulkanGraphicsContext.h"

bool operator==(const VkDescriptorPoolSize& lhs, const VkDescriptorPoolSize& rhs) {
	return lhs.descriptorCount == rhs.descriptorCount && lhs.type == rhs.type;
}

namespace VulkanBackend {
	VkFence Backend::CreateFence(uint32_t flags)
	{
		VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fenceInfo.flags = flags;

		VkFence fence{};
		vkCreateFence(m_Device, &fenceInfo, nullptr, &fence);

		return fence;
	}

	void Backend::FenceWait(VkFence fence)
	{
		vkWaitForFences(m_Device, 1, &fence, VK_TRUE, UINT64_MAX);
		vkResetFences(m_Device, 1, &fence);
	}
	
	void Backend::DestroyFence(VkFence fence)
	{
		vkDestroyFence(m_Device, fence, nullptr);
	}
	
	VkSemaphore Backend::CreateSemaphore()
	{
		VkSemaphore semaphore = VK_NULL_HANDLE;
		VkSemaphoreCreateInfo create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(m_Device, &create_info, nullptr, &semaphore);

		return semaphore;
	}
	
	void Backend::DestroySemaphore(VkSemaphore semaphore)
	{
		vkDestroySemaphore(m_Device, semaphore, nullptr);
	}
	
	CommandPoolInfo Backend::CreateCommandPool(uint32_t queueFamily, VkCommandBufferLevel level)
	{
		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = queueFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VkCommandPool commandPool = VK_NULL_HANDLE;
		vkCreateCommandPool(m_Device, &poolInfo, nullptr, &commandPool);
	
		CommandPoolInfo result{};
		result.Pool = commandPool;
		result.BufferLevel = level;
		return result;
	}
	
	void Backend::DestroyCommandPool(InOut<CommandPoolInfo> commandPool)
	{
		vkDestroyCommandPool(m_Device, commandPool.Pool, nullptr);
		commandPool = {};
	}
	
	VkCommandBuffer Backend::CreateCommandBuffer(In<CommandPoolInfo> commandPool)
	{
		VkCommandBufferAllocateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.commandPool = commandPool.Pool;
		bufferInfo.commandBufferCount = 1;
		bufferInfo.level = commandPool.BufferLevel;


		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		vkAllocateCommandBuffers(m_Device, &bufferInfo, &commandBuffer);

		return commandBuffer;
	}
	
	void Backend::CommandBufferBegin(VkCommandBuffer commandBuffer)
	{
		VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
	}
	
	void Backend::CommandBufferEnd(VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);
	}
	
	void Backend::CommandBufferExecuteSecondary(VkCommandBuffer commandBuffer, std::initializer_list<VkCommandBuffer> secondaryCommandBuffers)
	{
		vkCmdExecuteCommands((VkCommandBuffer)commandBuffer, (uint32_t)secondaryCommandBuffers.size(), secondaryCommandBuffers.begin());
	}

	void Backend::SubmitCommandBuffers(VkQueue queue, std::initializer_list<VkCommandBuffer> commandBuffers, VkFence fence, std::initializer_list<VkSemaphore> waitSemaphores, std::initializer_list<VkSemaphore> signalSemaphores, std::initializer_list<VkPipelineStageFlags> dstFlags)
	{
		VkSubmitInfo info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		info.pCommandBuffers = commandBuffers.begin();
		info.commandBufferCount = (uint32_t)commandBuffers.size();
		info.pWaitDstStageMask = dstFlags.begin();
		info.pWaitSemaphores = waitSemaphores.begin();
		info.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
		info.pSignalSemaphores = signalSemaphores.begin();
		info.signalSemaphoreCount = (uint32_t)signalSemaphores.size();

		vkQueueSubmit(queue, 1, &info, fence);
	}

	void Backend::DestroyCommandBuffer(VkCommandBuffer commandBuffer, In<CommandPoolInfo> commandPool)
	{
		vkFreeCommandBuffers(m_Device, commandPool.Pool, 1, &commandBuffer);
	}
	
	void Backend::SwapchainRelease(InOut<SwapchainInfo> swapchain)
	{
		for (auto& framebuffer : swapchain.Framebuffers) 
		{
			DestroyFramebuffer(framebuffer);
		}

		for (auto& imageView : swapchain.ImageViews) 
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}

		if (swapchain.Swapchain) 
		{
			vkDestroySwapchainKHR(m_Device, swapchain.Swapchain, nullptr);
		}

		swapchain = {};
	}

	SwapchainInfo Backend::CreateSwapchain(InOut<SurfaceInfo> surface, VkQueue presentQueue, In<CommandPoolInfo> mainCommandPool, uint32_t framebufferCount)
	{
		uint32_t formatCount = 0;
		std::vector<VkSurfaceFormatKHR> formats{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface.Surface, &formatCount, nullptr);
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface.Surface, &formatCount, formats.data());

		VkFormat format = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			format = VK_FORMAT_B8G8R8A8_UNORM;
			colorSpace = formats[0].colorSpace;
		}
		else
		{
			const VkFormat prefferedFormat = VK_FORMAT_B8G8R8A8_UNORM;
			for (uint32_t i = 0; i < formatCount; ++i)
			{
				if (formats[i].format == prefferedFormat)
				{
					format = formats[i].format;
				}
			}

			if (format == VK_FORMAT_UNDEFINED)
			{
				format = formats[0].format;
			}
		}

		VkSurfaceCapabilitiesKHR surfaceCap{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, surface.Surface, &surfaceCap);

		VkExtent2D extent{};

		if (surfaceCap.currentExtent.width == UINT32_MAX)
		{
			extent.width = std::max(std::min(surface.Width, surfaceCap.maxImageExtent.width), surfaceCap.minImageExtent.width);
			extent.height = std::max(std::min(surface.Height, surfaceCap.maxImageExtent.height), surfaceCap.minImageExtent.height);
		}
		else
		{
			extent = surfaceCap.currentExtent;
		}

		surface.Width = extent.width;
		surface.Height = extent.height;

		uint32_t presentModeCount = 0;
		std::vector<VkPresentModeKHR> presentModes{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface.Surface, &presentModeCount, nullptr);
		presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface.Surface, &presentModeCount, presentModes.data());

		VkPresentModeKHR presentMode = surface.PresentMode;

		if (std::find(presentModes.begin(), presentModes.end(), presentMode) == presentModes.end())
		{
			presentMode = VK_PRESENT_MODE_FIFO_KHR;
			surface.PresentMode = VK_PRESENT_MODE_FIFO_KHR;
		}

		uint32_t imageCount = std::max(framebufferCount, surfaceCap.minImageCount);
		if (surfaceCap.maxImageCount > 0)
		{
			imageCount = std::min(imageCount, surfaceCap.maxImageCount);
		}

		VkSurfaceTransformFlagBitsKHR surfaceTransform{};
		if (surfaceCap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			surfaceTransform = surfaceCap.currentTransform;
		}

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		VkSwapchainCreateInfoKHR swapchainInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		swapchainInfo.surface = surface.Surface;
		swapchainInfo.minImageCount = imageCount;
		swapchainInfo.imageFormat = format;
		swapchainInfo.imageColorSpace = colorSpace;
		swapchainInfo.imageExtent = extent;
		swapchainInfo.presentMode = presentMode;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.preTransform = surfaceTransform;
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.compositeAlpha = compositeAlpha;
		swapchainInfo.imageArrayLayers = 1;

		VkSwapchainKHR swapchain{};
		vkCreateSwapchainKHR(m_Device, &swapchainInfo, nullptr, &swapchain);

		uint32_t swapchainImageCount = 0;
		std::vector<VkImage> images{};
		vkGetSwapchainImagesKHR(m_Device, swapchain, &swapchainImageCount, nullptr);
		images.resize(swapchainImageCount);
		vkGetSwapchainImagesKHR(m_Device, swapchain, &swapchainImageCount, images.data());

		std::vector<VkImageView> imageViews{};

		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;

		for (uint32_t i = 0; i < swapchainImageCount; ++i)
		{
			viewInfo.image = images[i];
			VkImageView view{};
			vkCreateImageView(m_Device, &viewInfo, nullptr, &view);
			imageViews.push_back(view);
		}

		std::vector<SwapchainInfo::Frame> frames{};

		for (uint32_t i = 0; i < swapchainImageCount; ++i) 
		{
			SwapchainInfo::Frame frame{};
			frame.MainCommandBuffer = CreateCommandBuffer(mainCommandPool);
			frame.InFlightFence = CreateFence(VK_FENCE_CREATE_SIGNALED_BIT);
			frame.ImageAvailable = CreateSemaphore();
			frame.RenderFinished = CreateSemaphore();
			frames.push_back(frame);
		}

		VkAttachmentDescription attachment{};
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.format = format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorRef{};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkRenderPassCreateInfo passInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &attachment;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;

		VkRenderPass renderPass = {};
		vkCreateRenderPass(m_Device, &passInfo, nullptr, &renderPass);

		std::vector<VkFramebuffer> framebuffers{};

		VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		for (uint32_t i = 0; i < swapchainImageCount; ++i)
		{
			framebufferInfo.pAttachments = &imageViews[i];
			VkFramebuffer framebuffer{};
			vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &framebuffer);
			framebuffers.push_back(framebuffer);
		}
		
		SwapchainInfo info{};
		info.Colorspace = colorSpace;
		info.Extent = extent;
		info.Format = format;
		info.Framebuffers = framebuffers;
		info.Frames = frames;
		info.ImageCount = swapchainImageCount;
		info.ImageIndex = -1;
		info.Images = images;
		info.ImageViews = imageViews;
		info.PresentQueue = presentQueue;
		info.RenderPass = renderPass;
		info.Surface = surface;
		info.Swapchain = swapchain;
		return info;
	}

	void Backend::DestroySwapchain(InOut<SwapchainInfo> swapchain)
	{
		for (auto& framebuffer : swapchain.Framebuffers) 
		{
			DestroyFramebuffer(framebuffer);
		}

		for (auto& view : swapchain.ImageViews) {
			vkDestroyImageView(m_Device, view, nullptr);
		}

		swapchain.ImageIndex = -1;
		swapchain.Images.clear();
		swapchain.ImageViews.clear();
		swapchain.Framebuffers.clear();

		vkDestroySwapchainKHR(m_Device, swapchain.Swapchain, nullptr);
		swapchain.Swapchain = {};
	}
	
	TextureInfo Backend::CreateTexture(In<TextureInputInfo> info)
	{
		VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

		if (info.IsCube)
		{
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
		imageInfo.arrayLayers = info.Layers;
		imageInfo.extent = { info.Width,info.Height,info.Depth };
		imageInfo.format = info.Format;
		imageInfo.imageType = info.Type;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.mipLevels = info.Mips;
		imageInfo.samples = info.Samples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.tiling = info.IsCpuReadable ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = info.Usage;

		VmaAllocationCreateInfo allocationCreateInfo{};
		allocationCreateInfo.flags = info.IsCpuReadable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT : 0;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

		VkImage image{};
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};
		vmaCreateImage(m_Allocator, &imageInfo, &allocationCreateInfo, &image, &allocation, &allocationInfo);

		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.components.r = info.Swizzles[0];
		viewInfo.components.g = info.Swizzles[1];
		viewInfo.components.b = info.Swizzles[2];
		viewInfo.components.a = info.Swizzles[3];
		viewInfo.format = info.ViewFormat;
		viewInfo.image = image;
		viewInfo.subresourceRange.aspectMask = info.Aspects;
		viewInfo.subresourceRange.layerCount = info.Layers;
		viewInfo.subresourceRange.levelCount = info.Mips;
		viewInfo.viewType = info.ViewType;

		VkImageView view{};
		vkCreateImageView(m_Device, &viewInfo, nullptr, &view);


		TextureInfo outInfo{};
		outInfo.Allocation = allocation;
		outInfo.AllocationInfo = allocationInfo;
		outInfo.ImageInfo = imageInfo;
		outInfo.View = view;
		outInfo.ViewInfo = viewInfo;

		return outInfo;
	}
	
	uint8_t* Backend::TextureMap(In<TextureInfo> info, VkImageAspectFlagBits aspect, uint32_t layer, uint32_t mip)
	{
		VkImageSubresource subRes{};
		subRes.aspectMask = aspect;
		subRes.arrayLayer = layer;
		subRes.mipLevel = mip;

		VkSubresourceLayout layout = {};
		vkGetImageSubresourceLayout(m_Device,info.ViewInfo.image, &subRes, &layout);

		void* data = nullptr;
		vkMapMemory(m_Device, info.AllocationInfo.deviceMemory, info.AllocationInfo.offset + layout.offset, layout.size, 0 ,&data);

		vmaMapMemory(m_Allocator, info.Allocation, &data);
		return (uint8_t*)data;
	}
	
	void Backend::TextureUnmap(In<TextureInfo> info)
	{
		vkUnmapMemory(m_Device,info.AllocationInfo.deviceMemory);
	}
	
	TextureInfo Backend::CreateTextureFromExisting(void* nativeTexture, uint32_t layer, uint32_t layerCount, uint32_t level, uint32_t levelCount, VkFormat viewFormat,
		VkImageAspectFlags aspects, VkImageViewType viewType, VkComponentSwizzle swizzles[4])
	{
		VkImage image = (VkImage)nativeTexture;

		// We only need to create a view into the already existing natively-provided texture.

		VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = image;
		viewInfo.viewType = viewType;
		viewInfo.format = viewFormat;
		viewInfo.components.r = swizzles[0];
		viewInfo.components.g = swizzles[1];
		viewInfo.components.b = swizzles[2];
		viewInfo.components.a = swizzles[3];
		viewInfo.subresourceRange.baseArrayLayer = layer;
		viewInfo.subresourceRange.layerCount = layerCount;
		viewInfo.subresourceRange.baseMipLevel = level;
		viewInfo.subresourceRange.levelCount = levelCount;
		viewInfo.subresourceRange.aspectMask = aspects;

		VkImageView view = VK_NULL_HANDLE;
		vkCreateImageView(m_Device, &viewInfo, nullptr, &view);

		TextureInfo info{};
		info.View = view;
		info.ViewInfo = viewInfo;
		return info;
	}
	
	void Backend::DestroyTexture(InOut<TextureInfo> texture)
	{
		VK_CONTEXT.SubmitDeferredDelete([device = m_Device, view = texture.View]() {
			vkDestroyImageView(device, view, nullptr);
		});

		if (texture.Allocation) 
		{
			VkImage image = texture.ViewInfo.image;
			VmaAllocation allocation = texture.Allocation;
			VK_CONTEXT.SubmitDeferredDelete(
				[allocator=  m_Allocator, image, allocation]() {
				vmaDestroyImage(allocator, image, allocation);
			});
		}
	}

	VkSampler Backend::CreateSampler(In<SamplerState> state)
	{
		VkSamplerCreateInfo samplerInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = state.MagFilter;
		samplerInfo.minFilter = state.MinFilter;
		samplerInfo.mipmapMode = state.MipFilter;
		samplerInfo.addressModeU = state.AddressModeU;
		samplerInfo.addressModeV = state.AddressModeV;
		samplerInfo.addressModeW = state.AddressModeW;
		samplerInfo.mipLodBias = state.LodBias;
		samplerInfo.anisotropyEnable = state.Aniso;
		samplerInfo.maxAnisotropy = state.AnisoMax;
		samplerInfo.compareEnable = state.Compare;
		samplerInfo.compareOp = state.CompareOp;
		samplerInfo.minLod = state.MinLod;
		samplerInfo.maxLod = state.MaxLod;
		samplerInfo.borderColor = state.BorderColor;
		samplerInfo.unnormalizedCoordinates = state.Unnormalized;

		VkSampler sampler{};
		vkCreateSampler(m_Device, &samplerInfo, nullptr, &sampler);

		return sampler;
	}

	void Backend::DestroySampler(VkSampler sampler)
	{
		VK_CONTEXT.SubmitDeferredDelete([device = m_Device, sampler]() {
			vkDestroySampler(device, sampler, nullptr);
		});
	}
	
	VkFramebuffer Backend::CreateFramebuffer(VkRenderPass renderPass, const std::vector<const TextureInfo*>& attachments, uint32_t width, uint32_t height, uint32_t layers)
	{
		std::vector<VkImageView> views{};
		for (auto& attachment : attachments) {
			views.push_back(attachment->View);
		}

		VkFramebufferCreateInfo framebufferInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = (uint32_t)views.size();
		framebufferInfo.pAttachments = views.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = layers;

		VkFramebuffer framebuffer{};

		vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &framebuffer);

		return framebuffer;
	}

	void Backend::DestroyFramebuffer(VkFramebuffer framebuffer)
	{
		VK_CONTEXT.SubmitDeferredDelete([device = m_Device, framebuffer]() {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		});
	}

	void Backend::ReflectDescriptor(ShaderReflection& reflection, const spirv_cross::CompilerReflection& compilerReflection,
		const spirv_cross::SmallVector<spirv_cross::Resource>& resources, VkDescriptorType type, uint32_t stageFlags)
	{
		for (auto& resource : resources) {
			uint32_t set = compilerReflection.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compilerReflection.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t count = 1;
			const spirv_cross::SPIRType& spirvType = compilerReflection.get_type(resource.type_id);
			if (spirvType.array.size() > 0) {
				count = spirvType.array[0];
			}
			//reflection.AddDescriptor(type, count, set, binding);
		}
	}
	

	ShaderInfo Backend::CreateShader(const std::unordered_map<VkShaderStageFlagBits, std::initializer_list<uint32_t>>& spirvs)
	{
		std::unordered_map<VkShaderStageFlagBits, VkPipelineShaderStageCreateInfo> stages{};

		ShaderReflection shaderReflection{};
		VkShaderStageFlags pushConstantStages = 0;

		for (auto& [stage, spirv] : spirvs)
		{
			VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			moduleInfo.pCode = spirv.begin();
			moduleInfo.codeSize = (uint32_t)(spirv.size() * sizeof(uint32_t));

			VkShaderModule module{};
			vkCreateShaderModule(m_Device, &moduleInfo, nullptr, &module);

			//Reflect
			{
				SpvReflectShaderModule reflection{};
				spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.begin(), &reflection);

				if ((stage & VK_SHADER_STAGE_VERTEX_BIT) && reflection.input_variable_count)
				{
					for (uint32_t i = 0; i < reflection.input_variable_count; ++i)
					{
						const SpvReflectInterfaceVariable* variable = reflection.input_variables[i];
						
						if (variable->built_in != -1)
							continue;

						VertexShaderInputReflection input;
						input.Location = variable->location;
						input.Name = variable->name;
						shaderReflection.Inputs[variable->location] = input;
					}
				}

				uint32_t bindingCount = 0;
				spvReflectEnumerateDescriptorBindings(&reflection, &bindingCount, nullptr);

				if (bindingCount)
				{
					std::vector<SpvReflectDescriptorBinding*> bindings;
					bindings.resize(bindingCount);

					spvReflectEnumerateDescriptorBindings(&reflection, &bindingCount, bindings.data());

					for (uint32_t i = 0; i < bindingCount; ++i)
					{
						const SpvReflectDescriptorBinding& binding = *bindings[i];

						VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
						switch (binding.descriptor_type)
						{
						case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: type = VK_DESCRIPTOR_TYPE_SAMPLER; break;
						case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; break;
						case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
						case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
						case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER; break;
						case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER; break;
						case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
						case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
						}

						std::string name = binding.name;

						shaderReflection.AddDescriptor(name, type, binding.count, binding.set, binding.binding, stage);
					}
				}


				uint32_t pcCount = 0;
				spvReflectEnumeratePushConstantBlocks(&reflection, &pcCount, nullptr);

				if (pcCount)
				{
					KD_CORE_ASSERT(pcCount == 1);

					std::vector<SpvReflectBlockVariable*> pushConstants;
					pushConstants.resize(pcCount);

					spvReflectEnumeratePushConstantBlocks(&reflection, &pcCount, pushConstants.data());

					const SpvReflectBlockVariable& pushConstant = *pushConstants[0];
					KD_CORE_ASSERT(!shaderReflection.PushConstantSize || shaderReflection.PushConstantSize == pushConstant.size);

					shaderReflection.PushConstantSize = pushConstant.size;
					
					pushConstantStages |= stage;
				}

				spvReflectDestroyShaderModule(&reflection);
			}

			VkPipelineShaderStageCreateInfo stageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			stageInfo.module = module;
			stageInfo.pName = "main";
			stageInfo.stage = stage;
			
			stages[stage] = stageInfo;
		}

		std::vector<VkDescriptorSetLayout> setLayouts;
		setLayouts.resize(shaderReflection.Sets.size());

		for (auto& [setIndex, set] : shaderReflection.Sets)
		{
			std::vector<VkDescriptorSetLayoutBinding> setBindings;
			setBindings.resize(set.Bindings.size());

			for (auto& [bindingIndex, binding] : set.Bindings)
			{
				VkDescriptorSetLayoutBinding setBinding{};
				setBinding.binding = binding.Binding;
				setBinding.descriptorCount = binding.Count;
				setBinding.descriptorType = binding.Type;
				setBinding.stageFlags = VK_SHADER_STAGE_ALL;
				setBindings[bindingIndex] = setBinding;

			}


			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.pBindings = setBindings.data();
			layoutInfo.bindingCount = (uint32_t)setBindings.size();

			VkDescriptorSetLayout layout{};
			vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &layout);
			setLayouts[setIndex] = layout;
		}

		VkPushConstantRange range{};
		range.offset = 0;
		range.size = shaderReflection.PushConstantSize;
		range.stageFlags = pushConstantStages;

		VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		layoutInfo.pSetLayouts = setLayouts.data();
		layoutInfo.setLayoutCount = (uint32_t)setLayouts.size();
		layoutInfo.pPushConstantRanges = pushConstantStages != 0 ? &range : nullptr;
		layoutInfo.pushConstantRangeCount = pushConstantStages != 0;

		VkPipelineLayout layout{};

		vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &layout);

		ShaderInfo info{};
		info.DescriptorSetLayouts = setLayouts;
		info.Layout = layout;
		info.PushConstantStages = pushConstantStages;
		info.VkStageInfos = stages;
		info.Reflection = shaderReflection;
		return info;
	}

	void Backend::DestroyShader(InOut<ShaderInfo> shader)
	{
		for (uint32_t i = 0; i < shader.DescriptorSetLayouts.size(); ++i)
		{
			VK_CONTEXT.SubmitDeferredDelete([device = m_Device, layout = shader.DescriptorSetLayouts[i]]() {
				vkDestroyDescriptorSetLayout(device, layout , nullptr);
			});
		}
		
		VK_CONTEXT.SubmitDeferredDelete([device = m_Device, layout = shader.Layout]() {
			vkDestroyPipelineLayout(device, layout, nullptr);
		});

		for (auto& [stage, moduleInfo] : shader.VkStageInfos)
		{
			VK_CONTEXT.SubmitDeferredDelete([device = m_Device, module = moduleInfo.module]() {
				vkDestroyShaderModule(device, module, nullptr);
			});
		}
	}


	void Backend::UpdateShaderModules(
		const std::unordered_map<VkShaderStageFlagBits, std::initializer_list<uint32_t>>& spirvs,
		InOut<ShaderInfo> shader)
	{
		for (auto& [stage, moduleInfo] : shader.VkStageInfos)
		{
			VK_CONTEXT.SubmitDeferredDelete([device = m_Device, module = moduleInfo.module]() {
				vkDestroyShaderModule(device, module, nullptr);
			});
		}

		shader.VkStageInfos.clear();

		for (auto& [stage, spirv] : spirvs)
		{
			VkShaderModuleCreateInfo moduleInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			moduleInfo.pCode = spirv.begin();
			moduleInfo.codeSize = (uint32_t)(spirv.size() * sizeof(uint32_t));

			VkShaderModule module{};
			vkCreateShaderModule(m_Device, &moduleInfo, nullptr, &module);

			VkPipelineShaderStageCreateInfo stageInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			stageInfo.module = module;
			stageInfo.pName = "main";
			stageInfo.stage = stage;

			shader.VkStageInfos[stage] = stageInfo;
		}
	}

	VkPipeline Backend::CreateGraphicsPipeline(
		In<ShaderInfo> shaders,
		In<VkPipelineVertexInputStateCreateInfo> input,
		VkPrimitiveTopology primitive, 
		In<VkPipelineTessellationStateCreateInfo> tessellationState,
		In<VkPipelineRasterizationStateCreateInfo> rasterizationState,
		In<VkPipelineMultisampleStateCreateInfo> multisampleState,
		In<VkPipelineDepthStencilStateCreateInfo> depthStencilState,
		In<VkPipelineColorBlendStateCreateInfo> colorBlendState,
		In<VkPipelineDynamicStateCreateInfo> dynamicState,
		VkRenderPass renderPass, 
		uint32_t subpass)
	{
		//Input Assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
		inputAssemblyState.primitiveRestartEnable = false;
		inputAssemblyState.topology = primitive;

		

		//Viewport
		VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		//Shader Stages
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		
		for (auto& [stage, shaderInfo] : shaders.VkStageInfos)
		{
			shaderStages.push_back(shaderInfo);
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.layout = shaders.Layout;
		pipelineInfo.pColorBlendState = &colorBlendState;
		pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pMultisampleState = &multisampleState;
		pipelineInfo.pRasterizationState = &rasterizationState;
		pipelineInfo.pTessellationState = &tessellationState;
		pipelineInfo.pVertexInputState = &input;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = subpass;

		VkPipeline pipeline{};
		vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

		return pipeline;
	}

	void Backend::DestroyGraphicsPipeline(VkPipeline pipeline)
	{
		VK_CONTEXT.SubmitDeferredDelete([device = m_Device, pipeline]() {
			vkDestroyPipeline(device, pipeline, nullptr);
		});
	}

	VkPipeline Backend::CreateComputePipeline(In<ShaderInfo> shader) {
		VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
		pipelineInfo.layout = shader.Layout;
		pipelineInfo.stage = shader.VkStageInfos.at(VK_SHADER_STAGE_COMPUTE_BIT);
	
		VkPipeline pipeline{};
		vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

		return pipeline;
	}
	
	void Backend::DestroyComputePipeline(VkPipeline pipeline) {
		VK_CONTEXT.SubmitDeferredDelete([device = m_Device, pipeline]() {
			vkDestroyPipeline(device, pipeline, nullptr);
		});
	}

	VkAttachmentReference2 Backend::ToAttachmentReference(const AttachmentReference& ref)
	{
		VkAttachmentReference2 res{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 };
		res.attachment = ref.Attachment;
		res.layout = ref.Layout;
		res.aspectMask = ref.Aspects;
		return res;
	}

	BufferInfo Backend::CreateBuffer(uint64_t size, VkBufferUsageFlags usage, bool isGpu)
	{
		VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo{};
		if (isGpu)
		{
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		}
		else
		{
			bool isSrc = usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bool isDst = usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			if (isSrc && !isDst)
			{
				allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			}
			if (isDst && !isSrc)
			{
				allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
			}

			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.requiredFlags = (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		}

		VkBuffer buffer{};
		VmaAllocation allocation{};
		VmaAllocationInfo allocationInfo{};
		vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &buffer, &allocation, &allocationInfo);

		BufferInfo info{};
		info.Allocation = allocation;
		info.AllocationSize = allocationInfo.size;
		info.Buffer = buffer;
		info.BufferSize = size;
		return info;
	}	

	void Backend::DestroyBuffer(InOut<BufferInfo> buffer)
	{
		VK_CONTEXT.SubmitDeferredDelete([allocator = m_Allocator, buffer = buffer.Buffer, allocation = buffer.Allocation]() {
			vmaDestroyBuffer(allocator, buffer, allocation);
		});
	}

	uint8_t* Backend::BufferMap(In<BufferInfo> buffer)
	{
		void* data = nullptr;
		vmaMapMemory(m_Allocator, buffer.Allocation, &data);
		return (uint8_t*)data;
	}

	void Backend::BufferUnmap(In<BufferInfo> buffer)
	{
		vmaUnmapMemory(m_Allocator, buffer.Allocation);
	}

	void Backend::BufferFlush(In<BufferInfo> buffer, uint64_t offset, uint64_t size)
	{
		vmaFlushAllocation(m_Allocator, buffer.Allocation, offset, size);
	}

	VkRenderPass Backend::CreateRenderPass(std::initializer_list<RenderPassAttachment> attachments,
		std::initializer_list<Subpass> subpasses, std::initializer_list<SubpassDependency> dependencies)
	{
		std::vector<VkAttachmentDescription2> descs;

		std::vector<VkAttachmentReference2> inputRefs;
		std::vector<VkAttachmentReference2> colorRefs;
		std::vector<VkAttachmentReference2> resolveRefs;

		std::vector<VkAttachmentReference2> depthRefs;
		std::vector<VkAttachmentReference2KHR> vrsAttachmentRefs;
		std::vector<VkFragmentShadingRateAttachmentInfoKHR> vrsAttachmentInfos;


		{
			for (uint32_t i = 0; i < attachments.size(); ++i)
			{
				const RenderPassAttachment* attachment = attachments.begin() + i;

				VkAttachmentDescription2 desc{ VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2 };
				desc.format = attachment->Format;
				desc.samples = attachment->Samples;
				desc.loadOp = attachment->LoadOp;
				desc.storeOp = attachment->StoreOp;
				desc.stencilLoadOp = attachment->StencilLoadOp;
				desc.stencilStoreOp = attachment->StencilStoreOp;
				desc.initialLayout = attachment->InitialLayout;
				desc.finalLayout = attachment->FinalLayout;

				descs.push_back(desc);
			}

		}

		{
			for (uint32_t i = 0; i < subpasses.size(); ++i)
			{
				const Subpass* subpass = subpasses.begin() + i;

				bool usesDepth = subpass->DepthStencil.Attachment != -1;
				VkAttachmentReference2 depthStencilRef{};

				bool usesVRS = subpass->VRS.Attachment != -1;
				

				for (uint32_t j = 0; j < subpass->Inputs.size(); ++j) 
				{
					inputRefs.push_back(ToAttachmentReference(subpass->Inputs[j]));
				}

				for (uint32_t j = 0; j < subpass->Colors.size(); ++j)
				{
					colorRefs.push_back(ToAttachmentReference(subpass->Colors[j]));
				}
				
				for (uint32_t j = 0; j < subpass->Resolves.size(); ++j)
				{
					resolveRefs.push_back(ToAttachmentReference(subpass->Resolves[j]));
				}

				if (usesDepth)
				{
					depthStencilRef = ToAttachmentReference(subpass->DepthStencil);
					depthRefs.push_back(depthStencilRef);
				}

				

				if (usesVRS)
				{
					VkAttachmentReference2KHR vrsAttachmentRef{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR };
					vrsAttachmentRef.attachment = subpass->VRS.Attachment;
					vrsAttachmentRef.layout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;
					vrsAttachmentRefs.push_back(vrsAttachmentRef);

					VkFragmentShadingRateAttachmentInfoKHR vrsAttachmentInfo{ VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR };
					vrsAttachmentInfo.pFragmentShadingRateAttachment = &vrsAttachmentRefs.back();
					vrsAttachmentInfo.shadingRateAttachmentTexelSize.width = 2;
					vrsAttachmentInfo.shadingRateAttachmentTexelSize.height = 2;

					vrsAttachmentInfos.push_back(vrsAttachmentInfo);

				}
			}
		}

		std::vector<VkSubpassDescription2> subpassDescs;
		{
			uint32_t inputOffset = 0;
			uint32_t colorOffset = 0;
			uint32_t resolveOffset = 0;
			uint32_t depthOffset = 0;
			uint32_t vrsInfoOffset = 0;
			for (uint32_t i = 0; i < subpasses.size(); ++i)
			{
				const Subpass* subpass = subpasses.begin() + i;

				bool usesDepth = subpass->DepthStencil.Attachment != -1;
				bool usesVRS = subpass->VRS.Attachment != -1;

				const VkAttachmentReference2* inputs = inputRefs.data() + inputOffset;
				const VkAttachmentReference2* colors = colorRefs.data() + colorOffset;
				const VkAttachmentReference2* resolves = resolveRefs.data() + resolveOffset;
				const VkAttachmentReference2* depth = usesDepth ? depthRefs.data() + depthOffset : nullptr;
				const VkFragmentShadingRateAttachmentInfoKHR* vrsInfo = usesVRS ? vrsAttachmentInfos.data() + vrsInfoOffset : nullptr;

				VkSubpassDescription2 subpassDesc{ VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2 };
				subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subpassDesc.viewMask = 0;
				subpassDesc.pInputAttachments = inputs;
				subpassDesc.inputAttachmentCount = (uint32_t)subpass->Inputs.size();
				subpassDesc.pColorAttachments = colors;
				subpassDesc.colorAttachmentCount = (uint32_t)subpass->Colors.size();
				subpassDesc.pResolveAttachments= resolves;
				subpassDesc.pPreserveAttachments = subpass->Preserves.data();
				subpassDesc.preserveAttachmentCount = (uint32_t)subpass->Preserves.size();
				subpassDesc.pDepthStencilAttachment = depth;
				subpassDesc.pNext = vrsInfo;

				inputOffset += (uint32_t)subpass->Inputs.size();
				colorOffset += (uint32_t)subpass->Colors.size();
				resolves += (uint32_t)subpass->Resolves.size();
				
				if (usesDepth)
					++depthOffset;
				if (usesVRS)
					++vrsInfoOffset;
				subpassDescs.push_back(subpassDesc);
			}
		}

		std::vector<VkSubpassDependency2> subpassDependencies;
		for (uint32_t i = 0; i < dependencies.size(); ++i)
		{
			const SubpassDependency* dependency = dependencies.begin() + i;
			VkSubpassDependency2 dep{ VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2 };
			dep.srcSubpass = dependency->Src;
			dep.dstSubpass = dependency->Dst;
			dep.srcStageMask = dependency->SrcStages;
			dep.dstStageMask = dependency->DstStages;
			dep.srcAccessMask = dependency->SrcAccesses;
			dep.dstAccessMask = dependency->DstAccesses;
			dep.dependencyFlags = dependency->DependencyFlags;
			subpassDependencies.push_back(dep);
		}

		VkRenderPassCreateInfo2 renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };
		renderPassInfo.pAttachments = descs.data();
		renderPassInfo.attachmentCount = (uint32_t)descs.size();
		renderPassInfo.pSubpasses = subpassDescs.data();
		renderPassInfo.subpassCount = (uint32_t)subpassDescs.size();
		renderPassInfo.pDependencies = subpassDependencies.data();
		renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();

		VkRenderPass renderPass{};
		vkCreateRenderPass2(m_Device, &renderPassInfo, nullptr, &renderPass);

		return renderPass;
	}

	void Backend::DestroyRenderPass(VkRenderPass renderPass)
	{
		VK_CONTEXT.SubmitDeferredDelete([device = m_Device, renderPass]() {
			vkDestroyRenderPass(device, renderPass, nullptr);
		});
	}

	VkDescriptorPool Backend::CreateDescriptorPool(const std::unordered_map<VkDescriptorType, VkDescriptorPoolSize>& sizes, uint32_t maxSets)
	{
		std::vector<VkDescriptorPoolSize> actualSizes;

		for (auto& [type,size] : sizes)
		{
			if (size.descriptorCount)
			{
				VkDescriptorPoolSize poolSize = {};
				poolSize.type = type;
				poolSize.descriptorCount = size.descriptorCount * maxSets;
			}
		}
		
		VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = maxSets;
		poolInfo.pPoolSizes = actualSizes.data();
		poolInfo.poolSizeCount = (uint32_t)actualSizes.size();

		VkDescriptorPool pool{};
		vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &pool);
		
		return pool;
	}

	void Backend::DestroyDescriptorPool(VkDescriptorPool pool)
	{
		vkDestroyDescriptorPool(m_Device, pool, nullptr);
	}

	DescriptorSetInfo Backend::CreateDescriptorSet(std::initializer_list<std::pair<VkDescriptorType,VkShaderStageFlags>> types)
	{
		std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> sizes;

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto& [type, flags] : types) {
			sizes[type].type = type;
			++sizes[type].descriptorCount;

			VkDescriptorSetLayoutBinding binding{};
			binding.binding = (uint32_t)bindings.size();
			binding.descriptorCount = 1;
			binding.descriptorType = type;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
			bindings.push_back(binding);
		}

		auto& pool = FindOrCreatePool(sizes);

		VkDescriptorSetLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layoutInfo.bindingCount = (uint32_t)bindings.size();
		layoutInfo.pBindings = bindings.data();

		VkDescriptorSetLayout layout{};
		vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &layout);

		VkDescriptorSetAllocateInfo setInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		setInfo.descriptorPool = pool.Pool;
		setInfo.descriptorSetCount = 1;
		setInfo.pSetLayouts = &layout;

		VkDescriptorSet set{};
		vkAllocateDescriptorSets(m_Device, &setInfo, &set);

		DescriptorSetInfo info{};
		info.Layout = layout;
		info.Set = set;
		info.Pool = pool.Pool;
		return info;
	}

	DescriptorSetInfo Backend::CreateDescriptorSet(In<ShaderInfo> shader, uint32_t setIndex)
	{
		VkDescriptorSetLayout layout = shader.DescriptorSetLayouts[setIndex];

		std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> sizes;

		for (auto& [index, binding] : shader.Reflection.Sets.at(setIndex).Bindings) {
			sizes[binding.Type].type = binding.Type;
			++sizes[binding.Type].descriptorCount;
		}

		VkDescriptorPool pool = FindOrCreatePool(sizes).Pool;

		VkDescriptorSetAllocateInfo setInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		setInfo.descriptorPool = pool;
		setInfo.descriptorSetCount = 1;
		setInfo.pSetLayouts = &layout;

		VkDescriptorSet set{};
		vkAllocateDescriptorSets(m_Device, &setInfo, &set);

		DescriptorSetInfo info{};
		info.Set = set;
		info.Pool = pool;
		return info;
	}

	void Backend::UpdateDescriptorSet(In<DescriptorSetInfo> info, const std::vector<VkWriteDescriptorSet>& writes)
	{
		vkUpdateDescriptorSets(m_Device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
	}

	
	void Backend::DestroyDescriptorSet(InOut<DescriptorSetInfo> set)
	{
		if (set.Layout)
		{
			VK_CONTEXT.SubmitDeferredDelete([device = m_Device, layout = set.Layout, pool = set.Pool, set = set.Set]() {
				vkDestroyDescriptorSetLayout(device, layout, nullptr);
				vkFreeDescriptorSets(device, pool, 1, &set);
			});
		}
		else
		{
			VK_CONTEXT.SubmitDeferredDelete([device = m_Device, pool = set.Pool, set = set.Set]() {
				vkFreeDescriptorSets(device, pool, 1, &set);
			});
		}
	}

	
	void Backend::CommandBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer, VkSubpassContents contents,
		In<VkRect2D> renderArea, std::initializer_list<VkClearValue> clearValues)
	{
		VkRenderPassBeginInfo beginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		beginInfo.pClearValues = clearValues.begin();
		beginInfo.clearValueCount = (uint32_t)clearValues.size();
		beginInfo.framebuffer = framebuffer;
		beginInfo.renderArea = renderArea;
		beginInfo.renderPass = renderPass;

		vkCmdBeginRenderPass(commandBuffer, &beginInfo, contents);
	}
	void Backend::CommandEndRenderPass(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}
	void Backend::CommandNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
	{
		vkCmdNextSubpass(commandBuffer, contents);
	}

	void Backend::CommandBindVertexBuffers(VkCommandBuffer commandBuffer, std::initializer_list<const BufferInfo*> buffers, std::initializer_list<uint64_t> offsets)
	{
		VkBuffer vulkanBuffers[32] = {};
		for (uint32_t i = 0; i < buffers.size(); ++i)
		{
			const BufferInfo* buffer = *(buffers.begin() + i);
			vulkanBuffers[i] = buffer->Buffer;
		}


		vkCmdBindVertexBuffers(commandBuffer, 0, (uint32_t)buffers.size(), vulkanBuffers, offsets.begin());
	}
	void Backend::CommandBindIndexBuffer(VkCommandBuffer commandBuffer, In<BufferInfo> buffer, VkIndexType indexType, uint64_t offset)
	{
		vkCmdBindIndexBuffer(commandBuffer, buffer.Buffer, offset, indexType);
	}

	void Backend::CommandBindGraphicsPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
	void Backend::CommandBindComputePipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	}
	void Backend::CommandBindPushConstants(VkCommandBuffer commandBuffer, In<ShaderInfo> shader, uint32_t firstIndex, const uint8_t* values, uint64_t size)
	{
		vkCmdPushConstants(commandBuffer, shader.Layout, shader.PushConstantStages, firstIndex, (uint32_t)size, values);
	}
	void Backend::CommandBindDescriptorSet(VkCommandBuffer commandBuffer, In<ShaderInfo> shader, In<DescriptorSetInfo> set, uint32_t setIndex)
	{
		VkPipelineBindPoint point{};
		if (shader.VkStageInfos.find(VK_SHADER_STAGE_COMPUTE_BIT) != shader.VkStageInfos.end())
			point = VK_PIPELINE_BIND_POINT_COMPUTE;
		else
			point = VK_PIPELINE_BIND_POINT_GRAPHICS;
		vkCmdBindDescriptorSets(commandBuffer, point, shader.Layout, setIndex, 1, &set.Set, 0, nullptr);
	}

	void Backend::CommandSetViewport(VkCommandBuffer commandBuffer, std::initializer_list<VkRect2D> viewports)
	{
		VkViewport vulkanViewports[32] = {};
		for (uint32_t i = 0; i < viewports.size(); ++i)
		{
			const VkRect2D* viewport = viewports.begin() + i;

			vulkanViewports[i].x = (float)viewport->offset.x;
			vulkanViewports[i].y = (float)viewport->offset.y;
			vulkanViewports[i].width = (float)viewport->extent.width;
			vulkanViewports[i].height = (float)viewport->extent.height;
			vulkanViewports[i].minDepth = 0.0f;
			vulkanViewports[i].maxDepth = 1.0f;
		}

		vkCmdSetViewport(commandBuffer, 0, (uint32_t)viewports.size(), vulkanViewports);
	}
	void Backend::CommandSetScissor(VkCommandBuffer commandBuffer, std::initializer_list<VkRect2D> scissors)
	{
		vkCmdSetScissor(commandBuffer, 0, (uint32_t)scissors.size(),scissors.begin());
	}
	void Backend::CommandSetBlendConstants(VkCommandBuffer commandBuffer, const float color[4])
	{
		vkCmdSetBlendConstants(commandBuffer, color);
	}
	void Backend::CommandSetLineWidth(VkCommandBuffer commandBuffer, float width)
	{
		vkCmdSetLineWidth(commandBuffer, width);
	}

	void Backend::CommandClearAttachments(VkCommandBuffer commandBuffer, std::initializer_list<VkClearAttachment> clearValues, std::initializer_list<VkClearRect> clearRects)
	{
		vkCmdClearAttachments(commandBuffer, (uint32_t)clearValues.size(), clearValues.begin(), (uint32_t)clearRects.size(), clearRects.begin());
	}

	void Backend::CommandDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset)
	{
		vkCmdDraw(commandBuffer, vertexCount, instanceCount, vertexOffset, instanceOffset);
	}
	void Backend::CommandDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t vertexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset)
	{
		vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
	}

	void Backend::CommandDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z)
	{
		vkCmdDispatch(commandBuffer, x, y, z);
	}

	void Backend::CommandClearBuffer(VkCommandBuffer commandBuffer, In<BufferInfo> buffer, uint64_t offset, uint64_t size)
	{
		vkCmdFillBuffer(commandBuffer, buffer.Buffer, offset, size, 0);
	}
	void Backend::CommandCopyBuffer(VkCommandBuffer commandBuffer, In<BufferInfo> srcBuffer, In<BufferInfo> dstBuffer, std::initializer_list<VkBufferCopy> regions)
	{
		vkCmdCopyBuffer(commandBuffer, srcBuffer.Buffer, dstBuffer.Buffer, (uint32_t)regions.size(), regions.begin());
	}
	void Backend::CommandCopyTexture(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout,
		In<TextureInfo> dstTexture, VkImageLayout dstLayout, std::initializer_list<VkImageCopy> regions)
	{
		vkCmdCopyImage(commandBuffer, srcTexture.ViewInfo.image, srcLayout, dstTexture.ViewInfo.image, dstLayout, (uint32_t)regions.size(), regions.begin());
	}
	void Backend::CommandResolveTexture(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMip,
		In<TextureInfo> dstTexture, VkImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMip)
	{

		VkImageResolve resolve = {};
		resolve.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolve.srcSubresource.mipLevel = srcMip;
		resolve.srcSubresource.baseArrayLayer = srcLayer;
		resolve.srcSubresource.layerCount = 1;
		resolve.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		resolve.dstSubresource.mipLevel = dstMip;
		resolve.dstSubresource.baseArrayLayer = dstLayer;
		resolve.dstSubresource.layerCount = 1;
		resolve.extent.width = std::max(1u, srcTexture.ImageInfo.extent.width >> srcMip);
		resolve.extent.height = std::max(1u, srcTexture.ImageInfo.extent.height >> srcMip);
		resolve.extent.depth = std::max(1u, srcTexture.ImageInfo.extent.depth >> srcMip);

		vkCmdResolveImage(commandBuffer, srcTexture.ViewInfo.image, srcLayout, dstTexture.ViewInfo.image, dstLayout, 1, &resolve);
	}
	void Backend::CommandClearColorTexture(VkCommandBuffer commandBuffer, In<TextureInfo> texture, VkImageLayout layout,
		const VkClearColorValue& clear, const VkImageSubresourceRange& range)
	{
		vkCmdClearColorImage(commandBuffer, texture.ViewInfo.image, layout, &clear, 1, &range);
	}
	void Backend::CommandCopyBufferToTexture(VkCommandBuffer commandBuffer, In<BufferInfo> srcBuffer, In<TextureInfo> dstTexture, VkImageLayout dstLayout,
		std::initializer_list<VkBufferImageCopy> regions)
	{
		vkCmdCopyBufferToImage(commandBuffer, srcBuffer.Buffer, dstTexture.ViewInfo.image, dstLayout, (uint32_t)regions.size(), regions.begin());
	}
	void Backend::CommandCopyTextureToBuffer(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout, In<BufferInfo> dstBuffer,
		std::initializer_list<VkBufferImageCopy> regions)
	{
		vkCmdCopyImageToBuffer(commandBuffer, srcTexture.ViewInfo.image, srcLayout, dstBuffer.Buffer, (uint32_t)regions.size(), regions.begin());
	}

	void Backend::CommandBlitTexture(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout,
		In<TextureInfo> dstTexture, VkImageLayout dstLayout, In<VkImageBlit> blit)
	{
		vkCmdBlitImage(commandBuffer, srcTexture.ViewInfo.image, srcLayout, dstTexture.ViewInfo.image, dstLayout, 1, &blit, VK_FILTER_LINEAR);
	}

	void Backend::CommandPipelineBarrier(
		VkCommandBuffer commandBuffer,
		VkPipelineStageFlags srcStages,
		VkPipelineStageFlags dstStages,
		std::initializer_list<VkMemoryBarrier> memoryBarriers,
		std::initializer_list<VkBufferMemoryBarrier> bufferBarriers,
		std::initializer_list<VkImageMemoryBarrier> textureBarriers)
	{
		vkCmdPipelineBarrier(commandBuffer,
			srcStages, dstStages,
			0,
			(uint32_t)memoryBarriers.size(), memoryBarriers.begin(),
			(uint32_t)bufferBarriers.size(), bufferBarriers.begin(),
			(uint32_t)textureBarriers.size(), textureBarriers.begin());
	}

	void Backend::ValuesToSizes(const std::vector<VkWriteDescriptorSet>& values, std::unordered_map<VkDescriptorType, VkDescriptorPoolSize>& sizes)
	{
		
		sizes = {};

		for (uint32_t i = 0; i < values.size(); ++i)
		{
			const VkWriteDescriptorSet* value = values.data() + i;
			++sizes[value->descriptorType].descriptorCount;
			sizes[value->descriptorType].type = value->descriptorType;
		}
	}

	/*std::vector<VkWriteDescriptorSet> Backend::ValuesToWrites(std::initializer_list<DescriptorValues> values)
	{
		std::vector<VkWriteDescriptorSet> writes;
		for (uint32_t i = 0; i < values.size(); ++i)
		{
			const DescriptorValues* value = values.begin() + i;

			VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			write.descriptorCount = 1;
			write.descriptorType = value->Type;
			write.dstArrayElement = 0;
			write.dstBinding = value->Binding;

			switch (value->Type)
			{
			case DescriptorType::Sampler:
			case DescriptorType::SamplerWithTexture:
			case DescriptorType::Texture:
			{
				VkDescriptorImageInfo* info = new VkDescriptorImageInfo;
				info->imageLayout = value->ImageValues.Layout;
				info->imageView = value->ImageValues.ImageView;
				info->sampler = value->ImageValues.Sampler;
				write.pImageInfo = info;
			}
			break;
			case DescriptorType::UniformBuffer:
			case DescriptorType::StorageBuffer:
			{
				VkDescriptorBufferInfo* info = new VkDescriptorBufferInfo;
				info->buffer = value->BufferValues.Buffer;
				info->offset = 0;
				info->range = VK_WHOLE_SIZE;
				write.pBufferInfo = info;
			}
			break;
			}

			writes.push_back(write);
		}

		return writes;
	}*/

	uint32_t Backend::VkFormatSize(VkFormat format)
	{
		switch (format)
		{		case VK_FORMAT_R32_SFLOAT:return sizeof(float);		case VK_FORMAT_R32G32_SFLOAT:return sizeof(float) * 2;		case VK_FORMAT_R32G32B32_SFLOAT:return sizeof(float) * 3;		case VK_FORMAT_R32G32B32A32_SFLOAT:return sizeof(float) * 4;
		case VK_FORMAT_R32_SINT:return sizeof(int32_t);		case VK_FORMAT_R32G32_SINT:return sizeof(int32_t) * 2;		case VK_FORMAT_R32G32B32_SINT:return sizeof(int32_t) * 3;		case VK_FORMAT_R32G32B32A32_SINT:return sizeof(int32_t) * 4;		}

		return 0;
	}

	Backend::DescriptorPool& Backend::FindOrCreatePool(const std::unordered_map<VkDescriptorType, VkDescriptorPoolSize>& sizes)
	{
		for (uint32_t i = 0; i < m_Pools.size(); ++i)
		{
			if(m_Pools[i].Sizes == sizes)
				return m_Pools[i];
		}

		m_Pools.push_back({});

		DescriptorPool& pool = m_Pools.back();
		pool.Pool = CreateDescriptorPool(sizes, s_MaxSetsPerPool);
		pool.Sizes = sizes;

		return pool;
	}
	
}
