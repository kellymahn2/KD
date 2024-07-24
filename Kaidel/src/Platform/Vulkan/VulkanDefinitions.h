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

		static VkFormat FormatToVulkanFormat(Format format) {

			switch (format)
			{
			case Format::None:       return VK_FORMAT_UNDEFINED;
			case Format::R8UN:       return VK_FORMAT_R8_UNORM;
			case Format::R8N:       return VK_FORMAT_R8_SNORM;
			case Format::R8UI:       return VK_FORMAT_R8_UINT;
			case Format::R8I:       return VK_FORMAT_R8_SINT;
			case Format::RG8UN:       return VK_FORMAT_R8G8_UNORM;
			case Format::RG8N:       return VK_FORMAT_R8G8_SNORM;
			case Format::RG8UI:       return VK_FORMAT_R8G8_UINT;
			case Format::RG8I:       return VK_FORMAT_R8G8_SINT;
			case Format::RGB8UN:       return VK_FORMAT_R8G8B8_UNORM;
			case Format::RGB8N:       return VK_FORMAT_R8G8B8_SNORM;
			case Format::RGB8UI:		return VK_FORMAT_R8G8B8_UINT;
			case Format::RGB8I:		return VK_FORMAT_R8G8B8_SINT;
			case Format::RGBA8UN:		return VK_FORMAT_R8G8B8A8_UNORM;
			case Format::RGBA8N:		return VK_FORMAT_R8G8B8A8_SNORM;
			case Format::RGBA8UI:		return VK_FORMAT_R8G8B8A8_UINT;
			case Format::RGBA8I:		return VK_FORMAT_R8G8B8A8_SINT;
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
	}

}




