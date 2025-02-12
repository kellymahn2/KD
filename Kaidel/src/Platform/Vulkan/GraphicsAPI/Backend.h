#pragma once


#include <VMA.h>
#include <glad/vulkan.h>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_reflect.hpp>
#include <spirv-headers/spirv_reflect.h>

#include <vector>
#include <unordered_map>

namespace VulkanBackend 
{
	
	template<typename T>
	using In = const T&;

	template<typename T>
	using Out = T&;

	template<typename T>
	using InOut = T&;

	struct CommandPoolInfo 
	{
		VkCommandPool Pool;
		VkCommandBufferLevel BufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	};

	struct SurfaceInfo 
	{
		VkSurfaceKHR Surface;
		uint32_t Width = 0, Height = 0;
		VkPresentModeKHR PresentMode;
	};

	struct SwapchainInfo 
	{
		struct Frame {
			VkCommandBuffer MainCommandBuffer;
			VkFence InFlightFence;
			VkSemaphore ImageAvailable;
			VkSemaphore RenderFinished;
		};
		VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
		SurfaceInfo Surface;
		VkExtent2D Extent;
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR Colorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		std::vector<VkImage> Images;
		std::vector<VkImageView> ImageViews;
		std::vector<Frame> Frames;
		std::vector<VkFramebuffer> Framebuffers;
		VkRenderPass RenderPass;
		uint32_t ImageIndex = 0;
		uint32_t ImageCount = 0;
		VkQueue PresentQueue;
	};

	struct TextureInputInfo {
		VkFormat Format;
		uint32_t Width; 
		uint32_t Height; 
		uint32_t Depth; 
		uint32_t Layers; 
		uint32_t Mips;
		VkImageType Type; 
		VkSampleCountFlagBits Samples; 
		VkImageUsageFlags Usage;
		VkFormat ViewFormat; 
		VkImageAspectFlags Aspects; 
		VkImageViewType ViewType; 
		VkComponentSwizzle Swizzles[4];
		bool IsCube = false; 
		bool IsCpuReadable = false;
	};

	struct TextureInfo 
	{
		VkImageCreateInfo ImageInfo = {};
		VkImageViewCreateInfo ViewInfo = {};
		VmaAllocationInfo AllocationInfo = {};
		VkImageView View = {};
		VmaAllocation Allocation = {};
	};

	struct SamplerState {
		VkFilter MagFilter = VK_FILTER_NEAREST;
		VkFilter MinFilter = VK_FILTER_NEAREST;
		VkSamplerMipmapMode MipFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		VkSamplerAddressMode AddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode AddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode AddressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		float LodBias = 0.0f;
		bool Aniso = false;
		float AnisoMax = 1.0f;
		bool Compare = false;
		VkCompareOp CompareOp = VK_COMPARE_OP_ALWAYS;
		float MinLod = 0.0f;
		float MaxLod = 1e20;
		VkBorderColor BorderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		bool Unnormalized = false;
	};

	struct BufferInfo
	{
		VkBuffer Buffer = VK_NULL_HANDLE;
		VmaAllocation Allocation = nullptr;
		uint64_t AllocationSize = 0;
		uint64_t BufferSize = 0;
	};

	struct DescriptorSetBindingReflection 
	{
		uint32_t Binding = 0;
		uint32_t Count = 0;
		VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		VkShaderStageFlags ShaderStages = 0;
	};

	struct DescriptorSetReflection 
	{
		uint32_t Set;
		std::unordered_map<uint32_t, DescriptorSetBindingReflection> Bindings;
		std::unordered_map<std::string, uint32_t> NameToBinding;
	};

	struct ShaderReflection 
	{
		std::unordered_map<uint32_t, DescriptorSetReflection> Sets;
		uint32_t PushConstantSize;

		void AddDescriptor(const std::string& name,VkDescriptorType type, uint32_t count, uint32_t set, uint32_t binding, VkShaderStageFlagBits stage) {
			Sets[set].Set = set;
			Sets[set].NameToBinding[name] = binding;

			DescriptorSetBindingReflection& setBinding = Sets[set].Bindings[binding];
			setBinding.Binding = binding;
			setBinding.Type = type;
			setBinding.Count = count;
			setBinding.ShaderStages |= stage;
		}

		void Add(const ShaderReflection& reflection) {
			for (auto& [setIndex, set] : reflection.Sets) {
				Sets[setIndex].Set = setIndex;
				Add(Sets[setIndex], set);
			}
		}
		void Add(DescriptorSetReflection& dst, const DescriptorSetReflection& src) {
			for (auto& [bindingIndex, binding] : src.Bindings) {
				dst.Bindings[bindingIndex] = binding;
			}
		}
	};

	struct ShaderModuleInfo
	{
		ShaderReflection Reflection;
		VkShaderModule Module{};
		VkShaderModuleCreateInfo ModuleInfo{};
	};

	struct ShaderInfo
	{
		std::unordered_map<VkShaderStageFlagBits,VkPipelineShaderStageCreateInfo> VkStageInfos;
		std::vector<VkDescriptorSetLayout> DescriptorSetLayouts{};
		ShaderReflection Reflection;
		VkShaderStageFlags PushConstantStages;
		VkPipelineLayout Layout{};
	};
	
	struct PipelineRasterization
	{
		bool DepthClamp = false;
		bool DiscardPrimitives = false;
		bool Wireframe = false;
		VkCullModeFlags CullMode = VK_CULL_MODE_NONE;
		VkFrontFace FrontFace = VK_FRONT_FACE_CLOCKWISE;
		bool DepthBias = false;
		float DepthBiasConstant = 0.0f;
		float DepthBiasClamp = 0.0f;
		float DepthBiasSlope = 0.0f;
		float LineWidth = 1.0f;
		uint32_t PatchControlPoints = 1;
	};
	
	struct PipelineMultisample
	{
		VkSampleCountFlagBits Samples;
		bool SampleShading = false;
		float SampleShadingMin = 0.0f;
		std::vector<VkSampleMask> SampleMask;
		bool AlphaToCoverage = false;
		bool AlphaToOne = false;
	};

	struct PipelineDepthStencil
	{
		bool DepthTest = false;
		bool DepthWrite = false;
		VkCompareOp DepthCompareOperator = VK_COMPARE_OP_ALWAYS;
		bool DepthRange = false;
		float DepthRangeMin = 0;
		float DepthRangeMax = 0;
		bool Stencil = false;

		struct StencilOperation {
			VkStencilOp Fail = VK_STENCIL_OP_ZERO;
			VkStencilOp Pass = VK_STENCIL_OP_ZERO;
			VkStencilOp DepthFail = VK_STENCIL_OP_ZERO;
			VkCompareOp Compare = VK_COMPARE_OP_ALWAYS;
			uint32_t CompareMask = 0;
			uint32_t WriteMask = 0;
			uint32_t Reference = 0;
		};

		StencilOperation StencilFrontOp;
		StencilOperation StencilBackOp;
	};

	struct PipelineColorBlend 
	{
		bool LogicOpEnable = false;
		VkLogicOp LogicOp = VK_LOGIC_OP_CLEAR;

		struct Attachment 
		{
			bool Blend = false;
			VkBlendFactor SrcColorBlend = VK_BLEND_FACTOR_ZERO;
			VkBlendFactor DstColorBlend = VK_BLEND_FACTOR_ZERO;
			VkBlendOp ColorBlendOp = VK_BLEND_OP_ADD;
			VkBlendFactor SrcAlphaBlend = VK_BLEND_FACTOR_ZERO;
			VkBlendFactor DstAlphaBlend = VK_BLEND_FACTOR_ZERO;
			VkBlendOp AlphaBlendOp = VK_BLEND_OP_ADD;
			bool WriteR = true;
			bool WriteG = true;
			bool WriteB = true;
			bool WriteA = true;
		};

		std::vector<Attachment> Attachments;
		float BlendConstant[4];
	};

	enum PipelineDynamicState_
	{
		PipelineDynamicState_LineWidth = (1 << 0),
		PipelineDynamicState_DepthBias = (1 << 1),
		PipelineDynamicState_BlendConstants = (1 << 2),
		PipelineDynamicState_DepthBounds = (1 << 3),
		PipelineDynamicState_StencilCompareMask = (1 << 4),
		PipelineDynamicState_StencilWriteMask = (1 << 5),
		PipelineDynamicState_StencilReference = (1 << 6),
	};

	typedef int PipelineDynamicState;

	struct DescriptorSetBufferValues {
		VkBuffer Buffer;
	};

	struct DescriptorSetImageValues {
		VkSampler Sampler;
		VkImageView ImageView;
		VkImageLayout Layout;
	};

	struct DescriptorSetInfo {
		VkDescriptorSet Set = VK_NULL_HANDLE;
		VkDescriptorPool Pool = VK_NULL_HANDLE;
		//Only set if allocated
		VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
	};

	struct RenderPassAttachment 
	{
		VkFormat Format = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits Samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp LoadOp= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkAttachmentLoadOp StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkAttachmentStoreOp StencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout FinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	struct AttachmentReference 
	{
		uint32_t Attachment = -1;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageAspectFlags Aspects;
	};

	struct Subpass 
	{
		std::vector<AttachmentReference> Inputs;
		std::vector<AttachmentReference> Colors;
		AttachmentReference DepthStencil;
		std::vector<AttachmentReference> Resolves;
		std::vector<uint32_t> Preserves;
		AttachmentReference VRS;
	};

	struct SubpassDependency 
	{
		uint32_t Src = 0xffffffff;
		uint32_t Dst = 0xffffffff;
		VkPipelineStageFlags SrcStages;
		VkPipelineStageFlags DstStages;
		VkAccessFlags SrcAccesses;
		VkAccessFlags DstAccesses;
		VkDependencyFlags DependencyFlags;
	};

	class Backend 
	{
		struct DescriptorPool {
			std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> Sizes;
			VkDescriptorPool Pool = VK_NULL_HANDLE;
		};

	public:
		Backend(VkDevice device,VkPhysicalDevice physicalDevice, VmaAllocator allocator) 
			:m_Device(device), m_PhysicalDevice(physicalDevice), m_Allocator(allocator)
		{
		}
		~Backend()
		{
			for (auto& pool : m_Pools)
			{
				DestroyDescriptorPool(pool.Pool);
			}
		}

		//Fences
		VkFence CreateFence(uint32_t flags = 0);
		void FenceWait(VkFence fence);
		void DestroyFence(VkFence fence);
		
		//Semaphores
		VkSemaphore CreateSemaphore();
		void DestroySemaphore(VkSemaphore semaphore);

		//Commamd Pool
		CommandPoolInfo CreateCommandPool(uint32_t queueFamily, VkCommandBufferLevel level);
		void DestroyCommandPool(InOut<CommandPoolInfo> commandPool);

		//Command Buffer
		VkCommandBuffer CreateCommandBuffer(In<CommandPoolInfo> commandPool);
		void CommandBufferBegin(VkCommandBuffer commandBuffer);
		void CommandBufferEnd(VkCommandBuffer commandBuffer);
		void CommandBufferExecuteSecondary(VkCommandBuffer commandBuffer, std::initializer_list<VkCommandBuffer> secondaryCommandBuffers);
		void SubmitCommandBuffers(VkQueue queue, std::initializer_list<VkCommandBuffer> commandBuffers, VkFence fence = {},
			std::initializer_list<VkSemaphore> waitSemaphores = {}, std::initializer_list<VkSemaphore> signalSemaphores = {}, std::initializer_list<VkPipelineStageFlags> dstFlags = {});
		void DestroyCommandBuffer(VkCommandBuffer commandBuffer, In<CommandPoolInfo> commandPool);
		
		//Swapchain
		SwapchainInfo CreateSwapchain(InOut<SurfaceInfo> surface, VkQueue presentQueue, In<CommandPoolInfo> mainCommandPool, uint32_t framebufferCount);
		void DestroySwapchain(InOut<SwapchainInfo> swapchain);

		//Texture
		TextureInfo CreateTexture(In<TextureInputInfo> info);
		uint8_t* TextureMap(In<TextureInfo> info, VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t layer = 0, uint32_t mip = 0);
		void TextureUnmap(In<TextureInfo> info);
		TextureInfo CreateTextureFromExisting(void* nativeTexture, uint32_t layer, uint32_t layerCount, uint32_t level, uint32_t levelCount, VkFormat viewFormat,
			VkImageAspectFlags aspects, VkImageViewType viewType, VkComponentSwizzle swizzles[4]);
		void DestroyTexture(InOut<TextureInfo> texture);

		//Sampler
		VkSampler CreateSampler(In<SamplerState> state);
		void DestroySampler(VkSampler sampler);

		//Buffer
		BufferInfo CreateBuffer(uint64_t size, VkBufferUsageFlags usage, bool isGpu = true);
		uint8_t* BufferMap(In<BufferInfo> buffer);
		void BufferUnmap(In<BufferInfo> buffer); 
		void BufferFlush(In<BufferInfo> buffer,uint64_t offset,uint64_t size);
		void DestroyBuffer(InOut<BufferInfo> buffer);
		
		//Framebuffer
		VkFramebuffer CreateFramebuffer(VkRenderPass renderPass, const std::vector<const TextureInfo*>& attachments, uint32_t width, uint32_t height, uint32_t layers);
		void DestroyFramebuffer(VkFramebuffer framebuffer);

		//Shader
		ShaderInfo CreateShader(const std::unordered_map<VkShaderStageFlagBits, std::initializer_list<uint32_t>>& spirvs);
		void DestroyShader(InOut<ShaderInfo> shader);

		//Pipeline
		VkPipeline CreateGraphicsPipeline(In<ShaderInfo> shaders,
			In<VkPipelineVertexInputStateCreateInfo> input,
			VkPrimitiveTopology primitive,
			In<VkPipelineTessellationStateCreateInfo> tessellationState,
			In<VkPipelineRasterizationStateCreateInfo> rasterizationState,
			In<VkPipelineMultisampleStateCreateInfo> multisampleState,
			In<VkPipelineDepthStencilStateCreateInfo> depthStencilState,
			In<VkPipelineColorBlendStateCreateInfo> colorBlendState,
			In<VkPipelineDynamicStateCreateInfo> dynamicState,
			VkRenderPass renderPass,
			uint32_t subpass
		);
		void DestroyGraphicsPipeline(VkPipeline pipeline);
		VkPipeline CreateComputePipeline(In<ShaderInfo> shader);
		void DestroyComputePipeline(VkPipeline pipeline);


		//Render Pass
		VkRenderPass CreateRenderPass(std::initializer_list<RenderPassAttachment> attachments, std::initializer_list<Subpass> subpasses, std::initializer_list<SubpassDependency> dependencies);
		void DestroyRenderPass(VkRenderPass renderPass);

		//Descriptor Pool
		VkDescriptorPool CreateDescriptorPool(const std::unordered_map<VkDescriptorType,VkDescriptorPoolSize>& sizes, uint32_t maxSets);
		void DestroyDescriptorPool(VkDescriptorPool pool);

		//Descriptor Set
		DescriptorSetInfo CreateDescriptorSet(std::vector<VkWriteDescriptorSet>& values, In<ShaderInfo> shader, uint32_t setIndex);
		DescriptorSetInfo CreateDescriptorSet(std::vector<VkWriteDescriptorSet>& values, const std::vector<VkShaderStageFlags>& flags);
		DescriptorSetInfo CreateDescriptorSet(std::initializer_list<std::pair<VkDescriptorType, VkShaderStageFlags>> types);
		DescriptorSetInfo CreateDescriptorSet(In<ShaderInfo> shader, uint32_t setIndex);
		void UpdateDescriptorSet(In<DescriptorSetInfo> info, const std::vector<VkWriteDescriptorSet>& writes);
		void DestroyDescriptorSet(InOut<DescriptorSetInfo> set);

		//Command Recording
		void CommandBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer, VkSubpassContents contents,
			In<VkRect2D> renderArea, std::initializer_list<VkClearValue> clearValues);
		void CommandEndRenderPass(VkCommandBuffer commandBuffer);
		void CommandNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
		
		void CommandBindVertexBuffers(VkCommandBuffer commandBuffer, std::initializer_list<const BufferInfo*> buffers, std::initializer_list<uint64_t> offsets);
		void CommandBindIndexBuffer(VkCommandBuffer commandBuffer, In<BufferInfo> buffer, VkIndexType indexType, uint64_t offset);

		void CommandBindGraphicsPipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);
		void CommandBindComputePipeline(VkCommandBuffer commandBuffer, VkPipeline pipeline);
		void CommandBindPushConstants(VkCommandBuffer commandBuffer, In<ShaderInfo> shader, uint32_t firstIndex, const uint8_t* values, uint64_t size);
		void CommandBindDescriptorSet(VkCommandBuffer commandBuffer, In<ShaderInfo> shader, In<DescriptorSetInfo> set, uint32_t setIndex);

		void CommandSetViewport(VkCommandBuffer commandBuffer, std::initializer_list<VkRect2D> viewports);
		void CommandSetScissor(VkCommandBuffer commandBuffer, std::initializer_list<VkRect2D> scissors);
		void CommandSetBlendConstants(VkCommandBuffer commandBuffer, const float color[4]);
		void CommandSetLineWidth(VkCommandBuffer commandBuffer, float width); 
		
		void CommandClearAttachments(VkCommandBuffer commandBuffer, std::initializer_list<VkClearAttachment> clearValues, std::initializer_list<VkClearRect> clearRects);
		
		void CommandDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t vertexOffset, uint32_t instanceOffset);
		void CommandDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t vertexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset, uint32_t instanceOffset);
		
		void CommandDispatch(VkCommandBuffer commandBuffer, uint32_t x, uint32_t y, uint32_t z);
		
		void CommandClearBuffer(VkCommandBuffer commandBuffer, In<BufferInfo> buffer, uint64_t offset, uint64_t size);
		void CommandCopyBuffer(VkCommandBuffer commandBuffer, In<BufferInfo> srcBuffer, In<BufferInfo> dstBuffer, std::initializer_list<VkBufferCopy> regions);
		void CommandCopyTexture(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout, 
			In<TextureInfo> dstTexture, VkImageLayout dstLayout, std::initializer_list<VkImageCopy> regions);
		void CommandResolveTexture(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout, uint32_t srcLayer, uint32_t srcMip, 
			In<TextureInfo> dstTexture, VkImageLayout dstLayout, uint32_t dstLayer, uint32_t dstMip);
		void CommandClearColorTexture(VkCommandBuffer commandBuffer, In<TextureInfo> texture, VkImageLayout layout, 
			const VkClearColorValue& clear, const VkImageSubresourceRange& range);
		void CommandCopyBufferToTexture(VkCommandBuffer commandBuffer, In<BufferInfo> srcBuffer, In<TextureInfo> dstTexture, VkImageLayout dstLayout,
			std::initializer_list<VkBufferImageCopy> regions);
		void CommandCopyTextureToBuffer(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout, In<BufferInfo> dstBuffer,
			std::initializer_list<VkBufferImageCopy> regions);

		void CommandBlitTexture(VkCommandBuffer commandBuffer, In<TextureInfo> srcTexture, VkImageLayout srcLayout, 
			In<TextureInfo> dstTexture, VkImageLayout dstLayout, In<VkImageBlit> blit);

		void CommandPipelineBarrier(
			VkCommandBuffer commandBuffer,
			VkPipelineStageFlags srcStages,
			VkPipelineStageFlags dstStages,
			std::initializer_list<VkMemoryBarrier> memoryBarriers,
			std::initializer_list<VkBufferMemoryBarrier> bufferBarriers,
			std::initializer_list<VkImageMemoryBarrier> textureBarriers);

	private:
		void SwapchainRelease(InOut<SwapchainInfo> swapchain);
		void ReflectDescriptor(ShaderReflection& reflection, const spirv_cross::CompilerReflection& compilerReflection,
			const spirv_cross::SmallVector<spirv_cross::Resource>& resources, VkDescriptorType type, uint32_t stageFlags);
		uint32_t VkFormatSize(VkFormat format);
		VkAttachmentReference2 Backend::ToAttachmentReference(const AttachmentReference& ref);

		void ValuesToSizes(const std::vector<VkWriteDescriptorSet>& values, std::unordered_map<VkDescriptorType, VkDescriptorPoolSize>& sizes);

		DescriptorPool& FindOrCreatePool(const std::unordered_map<VkDescriptorType, VkDescriptorPoolSize>& sizes);
	private:
		static const uint32_t s_MaxSetsPerPool = 1000;

		VkDevice m_Device;
		VkPhysicalDevice m_PhysicalDevice;
		VmaAllocator m_Allocator;
		std::vector<DescriptorPool> m_Pools;
	};
}
