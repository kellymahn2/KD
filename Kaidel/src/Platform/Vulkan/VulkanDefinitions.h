#pragma once



#include "Kaidel/Renderer/RendererDefinitions.h"

#include <glad/vulkan.h>
#include <VMA.h>


#define VK_ASSERT(pred) KD_ASSERT( ((pred) == VK_SUCCESS))





namespace Kaidel {

	struct VulkanBuffer {
		VkBuffer Buffer;
		VmaAllocation Allocation;
		uint64_t Size;
	};

	

	namespace Utils {

		static bool IsDepthFormat(Format format) {
			switch (format)
			{
			case Format::Depth24Stencil8:		return true;
			case Format::Depth32F:		return true;
			}
			return false;
		}

		static VkImageAspectFlags GetAspectFlags(Format format) {
			return IsDepthFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		}

		static VkFormat FormatToVulkanFormat(Format format) {

			switch (format)
			{
			case Format::None:       return VK_FORMAT_UNDEFINED;
			case Format::R8UN:       return VK_FORMAT_R8_UNORM;
			case Format::R8N:       return VK_FORMAT_R8_SNORM;
			case Format::R8UI:       return VK_FORMAT_R8_UINT;
			case Format::R8I:       return VK_FORMAT_R8_SINT;
			case Format::R8SRGB:	return VK_FORMAT_R8_SRGB;
			case Format::RG8UN:       return VK_FORMAT_R8G8_UNORM;
			case Format::RG8N:       return VK_FORMAT_R8G8_SNORM;
			case Format::RG8UI:       return VK_FORMAT_R8G8_UINT;
			case Format::RG8I:       return VK_FORMAT_R8G8_SINT;
			case Format::RG8SRGB:	return VK_FORMAT_R8G8_SRGB;
			case Format::RGB8UN:       return VK_FORMAT_R8G8B8_UNORM;
			case Format::RGB8N:       return VK_FORMAT_R8G8B8_SNORM;
			case Format::RGB8UI:		return VK_FORMAT_R8G8B8_UINT;
			case Format::RGB8I:		return VK_FORMAT_R8G8B8_SINT;
			case Format::RGB8SRGB:	return VK_FORMAT_R8G8B8_SRGB;
			case Format::RGBA8UN:		return VK_FORMAT_R8G8B8A8_UNORM;
			case Format::RGBA8N:		return VK_FORMAT_R8G8B8A8_SNORM;
			case Format::RGBA8UI:		return VK_FORMAT_R8G8B8A8_UINT;
			case Format::RGBA8I:		return VK_FORMAT_R8G8B8A8_SINT;
			case Format::RGBA8SRGB:	return VK_FORMAT_R8G8B8A8_SRGB;
			case Format::R16UN:		return VK_FORMAT_R16_UNORM;
			case Format::R16N:		return VK_FORMAT_R16_SNORM;
			case Format::R16UI:		return VK_FORMAT_R16_UINT;
			case Format::R16I:		return VK_FORMAT_R16_SINT;
			case Format::R16F:		return VK_FORMAT_R16_SFLOAT;
			case Format::RG16UN:		return VK_FORMAT_R16G16_UNORM;
			case Format::RG16N:		return VK_FORMAT_R16G16_SNORM;
			case Format::RG16UI:		return VK_FORMAT_R16G16_UINT;
			case Format::RG16I:		return VK_FORMAT_R16G16_SINT;
			case Format::RG16F:		return VK_FORMAT_R16G16_SFLOAT;
			case Format::RGB16UN:		return VK_FORMAT_R16G16B16_UNORM;
			case Format::RGB16N:		return VK_FORMAT_R16G16B16_SNORM;
			case Format::RGB16UI:		return VK_FORMAT_R16G16B16_UINT;
			case Format::RGB16I:		return VK_FORMAT_R16G16B16_SINT;
			case Format::RGB16F:		return VK_FORMAT_R16G16B16_SFLOAT;
			case Format::RGBA16UN:		return VK_FORMAT_R16G16B16A16_UNORM;
			case Format::RGBA16N:		return VK_FORMAT_R16G16B16A16_SNORM;
			case Format::RGBA16UI:		return VK_FORMAT_R16G16B16A16_UINT;
			case Format::RGBA16I:		return VK_FORMAT_R16G16B16A16_SINT;
			case Format::RGBA16F:		return VK_FORMAT_R16G16B16A16_SFLOAT;
			case Format::R32UI:		return VK_FORMAT_R32_UINT;
			case Format::R32I:		return VK_FORMAT_R32_SINT;
			case Format::R32F:		return VK_FORMAT_R32_SFLOAT;
			case Format::RG32UI:		return VK_FORMAT_R32G32_UINT;
			case Format::RG32I:		return VK_FORMAT_R32G32_SINT;
			case Format::RG32F:		return VK_FORMAT_R32G32_SFLOAT;
			case Format::RGB32UI:		return VK_FORMAT_R32G32B32_UINT;
			case Format::RGB32I:		return VK_FORMAT_R32G32B32_SINT;
			case Format::RGB32F:		return VK_FORMAT_R32G32B32_SFLOAT;
			case Format::RGBA32UI:		return VK_FORMAT_R32G32B32A32_UINT;
			case Format::RGBA32I:		return VK_FORMAT_R32G32B32A32_SINT;
			case Format::RGBA32F:		return VK_FORMAT_R32G32B32A32_SFLOAT;
			case Format::Depth24Stencil8:		return VK_FORMAT_D24_UNORM_S8_UINT;
			case Format::Depth32F:		return VK_FORMAT_D32_SFLOAT;
			}
			return VK_FORMAT_UNDEFINED;
		}

		static VkIndexType IndexTypeToVulkanIndexType(IndexType type) {
			switch (type)
			{
			case Kaidel::IndexType::Uint16:return VK_INDEX_TYPE_UINT16;
			case Kaidel::IndexType::Uint32:return VK_INDEX_TYPE_UINT32;
			}
			return VK_INDEX_TYPE_MAX_ENUM;
		}

		

		static uint32_t VertexInputTypeSize(VertexInputType type) {
			switch (type)
			{
			case VertexInputType::Float:return sizeof(float);
			case VertexInputType::Float2:return sizeof(float) * 2;
			case VertexInputType::Float3:return sizeof(float) * 3;
			case VertexInputType::Float4:return sizeof(float) * 4;
			case VertexInputType::Int:return sizeof(int32_t);
			case VertexInputType::Int2:return sizeof(int32_t) * 2;
			case VertexInputType::Int3:return sizeof(int32_t) * 3;
			case VertexInputType::Int4:return sizeof(int32_t) * 4;
			}

			return 0;
		}

		static VkFormat VertexInputTypeToVulkanFormat(VertexInputType type) {
			switch (type)
			{
			case VertexInputType::Float:return VK_FORMAT_R32_SFLOAT;
			case VertexInputType::Float2:return VK_FORMAT_R32G32_SFLOAT;
			case VertexInputType::Float3:return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexInputType::Float4:return VK_FORMAT_R32G32B32A32_SFLOAT;
			case VertexInputType::Int:return VK_FORMAT_R32_SINT;
			case VertexInputType::Int2:return VK_FORMAT_R32G32_SINT;
			case VertexInputType::Int3:return VK_FORMAT_R32G32B32_SINT;
			case VertexInputType::Int4:return VK_FORMAT_R32G32B32A32_SINT;
			}

			return VK_FORMAT_UNDEFINED;
		}

		static VkCullModeFlagBits PipelineCullModeToVulkanCullMode(PipelineCullMode cullMode) {
			switch (cullMode)
			{
			case PipelineCullMode::None: return VK_CULL_MODE_NONE;
			case PipelineCullMode::Front:return VK_CULL_MODE_FRONT_BIT;
			case PipelineCullMode::Back:return VK_CULL_MODE_BACK_BIT;
			case PipelineCullMode::FrontAndBack:return VK_CULL_MODE_FRONT_AND_BACK;
			}
			return VK_CULL_MODE_NONE;
		}

		static VkImageLayout ImageLayoutToVulkanImageLayout(ImageLayout layout) {
			switch (layout) {
			case ImageLayout::None: return VK_IMAGE_LAYOUT_UNDEFINED;
			case ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
			case ImageLayout::ColorAttachmentOptimal: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case ImageLayout::DepthStencilAttachmentOptimal: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case ImageLayout::DepthStencilReadOnlyOptimal: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			case ImageLayout::ShaderReadOnlyOptimal: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case ImageLayout::TransferSrcOptimal: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case ImageLayout::TransferDstOptimal: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case ImageLayout::DepthReadOnlyStencilAttachmentOptimal: return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
			case ImageLayout::DepthAttachmentStencilReadOnlyOptimal: return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
			case ImageLayout::DepthAttachmentOptimal: return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			case ImageLayout::DepthReadOnlyOptimal: return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			case ImageLayout::StencilAttachmentOptimal: return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
			case ImageLayout::StencilReadOnlyOptimal: return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
			case ImageLayout::ReadOnlyOptimal: return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
			case ImageLayout::AttachmentOptimal: return VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
			case ImageLayout::PresentSrcKhr: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}

		static VkPipelineStageFlags GetPipelineStageForLayout(ImageLayout layout) {
			switch (layout) {
			case ImageLayout::General: return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			case ImageLayout::ColorAttachmentOptimal: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case ImageLayout::DepthStencilAttachmentOptimal: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case ImageLayout::DepthStencilReadOnlyOptimal: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case ImageLayout::ShaderReadOnlyOptimal: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case ImageLayout::TransferSrcOptimal: return VK_PIPELINE_STAGE_TRANSFER_BIT;
			case ImageLayout::TransferDstOptimal: return VK_PIPELINE_STAGE_TRANSFER_BIT;
			case ImageLayout::DepthReadOnlyStencilAttachmentOptimal: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case ImageLayout::DepthAttachmentStencilReadOnlyOptimal: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case ImageLayout::DepthAttachmentOptimal: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case ImageLayout::DepthReadOnlyOptimal: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case ImageLayout::StencilAttachmentOptimal: return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			case ImageLayout::StencilReadOnlyOptimal: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case ImageLayout::ReadOnlyOptimal: return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			case ImageLayout::AttachmentOptimal: return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			case ImageLayout::PresentSrcKhr: return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			default: return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			}
		}

		static VkFilter SamplerFilterToVulkanFilter(SamplerFilter filter) {
			switch (filter)
			{
			case SamplerFilter::Nearest:return VK_FILTER_NEAREST;
			case SamplerFilter::Linear:return VK_FILTER_LINEAR;
			}
			return VK_FILTER_MAX_ENUM;
		}

		static VkSamplerMipmapMode SamplerMipMapModeToVulkanMipMapMode(SamplerMipMapMode mode) {
			switch (mode)
			{
			case SamplerMipMapMode::Nearest:return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			case SamplerMipMapMode::Linear:return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			}
			return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
		}

		static VkSamplerAddressMode SamplerAddressModeToVulkanAddressMode(SamplerAddressMode mode) {
			switch (mode)
			{
			case SamplerAddressMode::Repeat:return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case SamplerAddressMode::MirroredRepeat:return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case SamplerAddressMode::ClampToEdge:return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case SamplerAddressMode::ClampToBorder:return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case SamplerAddressMode::MirrorClampToEdge:return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			}
			return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
		}

		static VkBorderColor SamplerBorderColorToVulkanBorderColor(SamplerBorderColor color) {
			switch (color)
			{
			case SamplerBorderColor::None:return (VkBorderColor)0;
			case SamplerBorderColor::FloatTransparentBlack:return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case SamplerBorderColor::IntTransparentBlack:return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
			case SamplerBorderColor::FloatOpaqueBlack:return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case SamplerBorderColor::IntOpaqueBlack:return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			case SamplerBorderColor::FloatOpaqueWhite:return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			case SamplerBorderColor::IntOpaqueWhite:return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			}
			return VK_BORDER_COLOR_MAX_ENUM;
		}

		static VkDescriptorType DescriptorTypeToVulkanDescriptorType(DescriptorType type) {
			switch (type)
			{
			case DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
			case DescriptorType::SamplerWithTexture: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case DescriptorType::Texture: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case DescriptorType::Image: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case DescriptorType::TextureBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			case DescriptorType::SamplerBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			case DescriptorType::ImageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			}
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}

		static VkShaderStageFlags ShaderStagesToVulkanShaderStageFlags(ShaderStages stages) {
			VkShaderStageFlags flags = 0;

			if (stages & ShaderStage_VertexShader) {
				flags |= VK_SHADER_STAGE_VERTEX_BIT;
			}
			if (stages & ShaderStage_TessellationControlShader) {
				flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			}
			if (stages & ShaderStage_TessellationEvaluationShader) {
				flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			}
			if (stages & ShaderStage_GeometryShader) {
				flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
			}
			if (stages & ShaderStage_FragmentShader) {
				flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			if (stages & ShaderStage_ComputeShader) {
				flags |= VK_SHADER_STAGE_COMPUTE_BIT;
			}
			return flags;
		}

		static VkShaderStageFlagBits ShaderTypeToVulkanShaderStageFlag(ShaderType type) {
			switch (type)
			{
			case Kaidel::ShaderType::VertexShader:return VK_SHADER_STAGE_VERTEX_BIT;
			case Kaidel::ShaderType::FragmentShader:return VK_SHADER_STAGE_FRAGMENT_BIT;
			case Kaidel::ShaderType::GeometryShader:return VK_SHADER_STAGE_GEOMETRY_BIT;
			case Kaidel::ShaderType::TessellationControlShader:return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			case Kaidel::ShaderType::TessellationEvaluationShader:return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			case Kaidel::ShaderType::ComputeShader:return VK_SHADER_STAGE_COMPUTE_BIT;
			}
			return VK_SHADER_STAGE_ALL;
		}

		static VkImageUsageFlags TextureUsageToVulkanImageUsageFlags(TextureUsage usage) {

			VkImageUsageFlags flags = 0;

			if (usage & TextureUsage_Sampled)
				flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if(usage & TextureUsage_Updateable)
				flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			if(usage & TextureUsage_CopyFrom)
				flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			if (usage & TextureUsage_CopyTo)
				flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			return flags;
		}

		static VkComponentSwizzle TextureSwizzleToVulkanComponentSwizzle(TextureSwizzle swizzle) {
			switch (swizzle)
			{
			case TextureSwizzle::Identity: return VK_COMPONENT_SWIZZLE_IDENTITY;
			case TextureSwizzle::Zero: return VK_COMPONENT_SWIZZLE_ZERO;
			case TextureSwizzle::One: return VK_COMPONENT_SWIZZLE_ONE;
			case TextureSwizzle::Red: return VK_COMPONENT_SWIZZLE_R;
			case TextureSwizzle::Green: return VK_COMPONENT_SWIZZLE_G;
			case TextureSwizzle::Blue: return VK_COMPONENT_SWIZZLE_B;
			case TextureSwizzle::Alpha: return VK_COMPONENT_SWIZZLE_A;
			}
			return VK_COMPONENT_SWIZZLE_MAX_ENUM;
		}

		static VkSampleCountFlags TextureSamplesToVulkanSampleCountFlags(TextureSamples samples) {
			switch (samples)
			{
			case TextureSamples::x1: return VK_SAMPLE_COUNT_1_BIT;
			case TextureSamples::x2: return VK_SAMPLE_COUNT_2_BIT;
			case TextureSamples::x4: return VK_SAMPLE_COUNT_4_BIT;
			case TextureSamples::x8: return VK_SAMPLE_COUNT_8_BIT;
			case TextureSamples::x16: return VK_SAMPLE_COUNT_16_BIT;
			case TextureSamples::x32: return VK_SAMPLE_COUNT_32_BIT;
			}

			return 0;
		}

		static VkAttachmentLoadOp AttachmentLoadOpToVulkanAttachmentLoadOp(AttachmentLoadOp loadOp) {
			switch (loadOp)
			{
			case AttachmentLoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
			case AttachmentLoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case AttachmentLoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			}
			return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
		}

		static VkAttachmentStoreOp AttachmentStoreOpToVulkanAttachmentStoreOp(AttachmentStoreOp storeOp) {
			switch (storeOp)
			{
			case Kaidel::AttachmentStoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
			case Kaidel::AttachmentStoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			}
			return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
		}

		static VkPipelineStageFlags PipelineStagesToVulkanPipelineStageFlags(PipelineStages stages) {
			VkPipelineStageFlags ret = 0;

			if(stages & PipelineStages_TopOfPipe)
				ret |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			if(stages & PipelineStages_DrawIndirect)
				ret |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			if(stages & PipelineStages_VertexInput)
				ret |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			if(stages & PipelineStages_VertexShader)
				ret |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
			if(stages & PipelineStages_TesselationControlShader)
				ret |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
			if(stages & PipelineStages_TesselationEvaluationShader)
				ret |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
			if(stages & PipelineStages_GeometryShader)
				ret |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
			if(stages & PipelineStages_FragmentShader)
				ret |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			if(stages & PipelineStages_EarlyFragmentTests)
				ret |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			if(stages & PipelineStages_LateFragmentTests)
				ret |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			if(stages & PipelineStages_ColorAttachmentOutput)
				ret |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			if(stages & PipelineStages_ComputeShader)
				ret |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			if(stages & PipelineStages_Transfer)
				ret |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			if(stages & PipelineStages_BottomOfPipe)
				ret |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			if(stages & PipelineStages_Host)
				ret |= VK_PIPELINE_STAGE_HOST_BIT;
			if(stages & PipelineStages_AllGraphics)
				ret |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
			if (stages & PipelineStages_AllCommandsBit)
				ret |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			
			return ret;
		}

		static VkAccessFlags AccessFlagsToVulkanAccessFlags(AccessFlags flags) {
			VkAccessFlags ret = 0;

			if(flags & AccessFlags_IndexRead)
				ret |= VK_ACCESS_INDEX_READ_BIT;
			if(flags & AccessFlags_VertexAttribureRead)
				ret |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			if(flags & AccessFlags_UniformRead)
				ret |= VK_ACCESS_UNIFORM_READ_BIT;
			if(flags & AccessFlags_InputAttachmentRead)
				ret |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			if(flags & AccessFlags_ShaderRead)
				ret |= VK_ACCESS_SHADER_READ_BIT;
			if(flags & AccessFlags_ShaderWrite)
				ret |= VK_ACCESS_SHADER_WRITE_BIT;
			if(flags & AccessFlags_ColorAttachmentRead)
				ret |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			if(flags & AccessFlags_ColorAttachmentWrite)
				ret |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			if(flags & AccessFlags_DepthStencilRead)
				ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			if(flags & AccessFlags_DepthStencilWrite)
				ret |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			if(flags & AccessFlags_TransferRead)
				ret |= VK_ACCESS_TRANSFER_READ_BIT;
			if(flags & AccessFlags_TransferWrite)
				ret |= VK_ACCESS_TRANSFER_WRITE_BIT;
			if(flags & AccessFlags_HostRead)
				ret |= VK_ACCESS_HOST_READ_BIT;
			if(flags & AccessFlags_HostWrite)
				ret |= VK_ACCESS_HOST_WRITE_BIT;
			if(flags & AccessFlags_MemoryRead)
				ret |= VK_ACCESS_MEMORY_READ_BIT;
			if(flags & AccessFlags_MemoryWrite)
				ret |= VK_ACCESS_MEMORY_WRITE_BIT;


			return ret;
		}

		static VkDependencyFlags SubpassDependencyFlagsToVulkanDependencyFlags(DependencyFlags flags) {
			VkDependencyFlags ret = 0;

			if (flags & DependencyFlags_Region) {
				ret |= VK_DEPENDENCY_BY_REGION_BIT;
			}

			if (flags & DependencyFlags_Device) {
				ret |= VK_DEPENDENCY_DEVICE_GROUP_BIT;
			}

			if (flags & DependencyFlags_ViewLocal) {
				ret |= VK_DEPENDENCY_VIEW_LOCAL_BIT;
			}

			return ret;
		}

		static VkDeviceSize CalculateImageSize(uint32_t width, uint32_t height, uint32_t depth, uint32_t layerCount, uint32_t pixelSize) {
			VkDeviceSize imageSize = (VkDeviceSize)width
				* (VkDeviceSize)height
				* (VkDeviceSize)depth
				* (VkDeviceSize)layerCount
				* (VkDeviceSize)pixelSize;
			return imageSize;
		}


		static VkPrimitiveTopology PrimitiveTopologyToVulkanPrimitiveTopology(PrimitiveTopology toplogy) {
			switch (toplogy)
			{
			case PrimitiveTopology::PointList:  return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case PrimitiveTopology::TrinagleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			case PrimitiveTopology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			case PrimitiveTopology::LineListAdj: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
			case PrimitiveTopology::LineStripAdj: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
			case PrimitiveTopology::TriangleListAd: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
			case PrimitiveTopology::TriangleStripAdj: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
			case PrimitiveTopology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			}
			
			return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
		}

		static VkBlendFactor BlendFactorToVulkanBlendFactor(BlendFactor factor) {
			switch (factor)
			{
			case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
			case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
			case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
			case BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
			case BlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
			case BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
			case BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case BlendFactor::ConstantColor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
			case BlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			case BlendFactor::ConstantAlpha: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
			case BlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
			case BlendFactor::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			case BlendFactor::Src1_Color: return VK_BLEND_FACTOR_SRC1_COLOR;
			case BlendFactor::OneMinusSrc1Color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
			case BlendFactor::Src1Alpha: return VK_BLEND_FACTOR_SRC1_ALPHA;
			case BlendFactor::OneMinusSrc1Alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
			}

			return VK_BLEND_FACTOR_MAX_ENUM;
		}

		static VkBlendOp BlendOpToVulkanBlendOp(BlendOp op) {
			switch (op)
			{
			case BlendOp::Add: return VK_BLEND_OP_ADD;
			case BlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
			case BlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
			case BlendOp::Min: return VK_BLEND_OP_MIN;
			case BlendOp::Max: return VK_BLEND_OP_MAX;
			}
			return VK_BLEND_OP_MAX_ENUM;
		}

		static VkStencilOp StencilOpToVulkanStencilOp(StencilOp op) {
			switch(op)
			{
			case StencilOp::Keep: return VK_STENCIL_OP_KEEP;
			case StencilOp::Zero: return VK_STENCIL_OP_ZERO;
			case StencilOp::Replace: return VK_STENCIL_OP_REPLACE;
			case StencilOp::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			case StencilOp::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			case StencilOp::Invert: return VK_STENCIL_OP_INVERT;
			case StencilOp::IncrementAndWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
			case StencilOp::DecrementAndWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
			}
			return VK_STENCIL_OP_MAX_ENUM;
		}

		static VkLogicOp LogicOpToVulkanLogicOp(LogicOp op) {
			switch(op)
			{
			case LogicOp::None: return VK_LOGIC_OP_NOR;
			case LogicOp::Clear: return VK_LOGIC_OP_CLEAR;
			case LogicOp::And: return VK_LOGIC_OP_AND;
			case LogicOp::AndReverse: return VK_LOGIC_OP_AND_REVERSE;
			case LogicOp::Copy: return VK_LOGIC_OP_COPY;
			case LogicOp::AndInverted: return VK_LOGIC_OP_AND_INVERTED;
			case LogicOp::Xor: return VK_LOGIC_OP_XOR;
			case LogicOp::Or: return VK_LOGIC_OP_OR;
			case LogicOp::Nor: return VK_LOGIC_OP_NOR;
			case LogicOp::Equivalent: return VK_LOGIC_OP_EQUIVALENT;
			case LogicOp::Invert: return VK_LOGIC_OP_INVERT;
			case LogicOp::OrReverse: return VK_LOGIC_OP_OR_REVERSE;
			case LogicOp::CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
			case LogicOp::OrInverted: return VK_LOGIC_OP_OR_INVERTED;
			case LogicOp::Nand: return VK_LOGIC_OP_NAND;
			case LogicOp::Set: return VK_LOGIC_OP_SET;
			}

			return VK_LOGIC_OP_MAX_ENUM;
		}

		static VkCompareOp CompareOpToVulkanCompareOp(CompareOp op) {
			switch (op)
			{
			case CompareOp::Never:return VK_COMPARE_OP_NEVER;
			case CompareOp::Less:return VK_COMPARE_OP_LESS;
			case CompareOp::Equal:return VK_COMPARE_OP_EQUAL;
			case CompareOp::LessOrEqual:return VK_COMPARE_OP_LESS_OR_EQUAL;
			case CompareOp::Greater:return VK_COMPARE_OP_GREATER;
			case CompareOp::NotEqual:return VK_COMPARE_OP_NOT_EQUAL;
			case CompareOp::GreaterOrEqual:return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case CompareOp::Always:return VK_COMPARE_OP_ALWAYS;
			}
			return VK_COMPARE_OP_MAX_ENUM;
		}

		static VulkanBuffer CreateBuffer(
			VmaAllocator allocator,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VmaMemoryUsage memoryUsage) {

			VulkanBuffer result{};

			VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = size;
			bufferInfo.usage = usage;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo allocInfo = {};
			allocInfo.usage = memoryUsage;

			VK_ASSERT(vmaCreateBuffer(
				allocator,
				&bufferInfo,
				&allocInfo,
				&result.Buffer,
				&result.Allocation,
				nullptr
			));

			result.Size = size;

			return result;
		}

		/*static void Transition(VkCommandBuffer cmdBuffer, ImageSpecification& image, ImageLayout newLayout) {
			if (newLayout == image.Layout)
				return;

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = Utils::ImageLayoutToVulkanImageLayout(image.Layout);
			barrier.newLayout = Utils::ImageLayoutToVulkanImageLayout(newLayout);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = (VkImage)image._InternalImageID;

			if (Utils::IsDepthFormat(image.ImageFormat)) {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				if (image.ImageFormat == Format::Depth24Stencil8) {
					barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else {
				barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			}

			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = image.Levels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = image.Layers;

			VkPipelineStageFlags sourceStage = Utils::GetPipelineStageForLayout(image.Layout);
			VkPipelineStageFlags destinationStage = Utils::GetPipelineStageForLayout(newLayout);

			vkCmdPipelineBarrier(
				cmdBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			image.Layout = newLayout;
			image.IntendedLayout = newLayout;
		}*/

	}

}




