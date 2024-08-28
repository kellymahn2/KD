#pragma once

#include "Kaidel/Core/Base.h"

#include <glm/glm.hpp>

namespace Kaidel {
	enum class VertexInputType {
		None,
		Dummy,
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		Mat3, Mat4,
	};

	enum class VertexInputRate {
		None,
		Vertex,
		Instance
	};

	enum class Format {
		None,
		R8UN,
		R8N,
		R8UI,
		R8I,
		RG8UN,
		RG8N,
		RG8UI,
		RG8I,
		RGB8UN,
		RGB8N,
		RGB8UI,
		RGB8I,
		RGBA8UN,
		RGBA8N,
		RGBA8UI,
		RGBA8I,
		R16UN,
		R16N,
		R16UI,
		R16I,
		R16F,
		RG16UN,
		RG16N,
		RG16UI,
		RG16I,
		RG16F,
		RGB16UN,
		RGB16N,
		RGB16UI,
		RGB16I,
		RGB16F,
		RGBA16UN,
		RGBA16N,
		RGBA16UI,
		RGBA16I,
		RGBA16F,
		R32UI,
		R32I,
		R32F,
		RG32UI,
		RG32I,
		RG32F,
		RGB32UI,
		RGB32I,
		RGB32F,
		RGBA32UI,
		RGBA32I,
		RGBA32F,
		Depth24Stencil8,
		Depth32F,
	};

	enum class PipelineCullMode {
		None,
		Front,
		Back,
		FrontAndBack
	};

	enum class ImageLayout {
		None = 0,
		General,
		ColorAttachmentOptimal,
		DepthStencilAttachmentOptimal,
		DepthStencilReadOnlyOptimal,
		ShaderReadOnlyOptimal,
		TransferSrcOptimal,
		TransferDstOptimal,
		DepthReadOnlyStencilAttachmentOptimal,
		DepthAttachmentStencilReadOnlyOptimal,
		DepthAttachmentOptimal,
		DepthReadOnlyOptimal,
		StencilAttachmentOptimal,
		StencilReadOnlyOptimal,
		ReadOnlyOptimal,
		AttachmentOptimal,
		PresentSrcKhr
	};


	enum class SamplerFilter {
		Nearest,
		Linear
	};

	enum class SamplerMipMapMode {
		Nearest,
		Linear
	};

	enum class SamplerAddressMode {
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge
	};


	enum class SamplerBorderColor {
		None = 0,
		FloatTransparentBlack,	// (0.0f,0.0f,0.0f,0.0f)
		IntTransparentBlack,	// (0,0,0,0)
		FloatOpaqueBlack,	// (0.0f,0.0f,0.0f,1.0f)
		IntOpaqueBlack,	// (0,0,0,1)
		FloatOpaqueWhite,	// (1.0f,1.0f,1.0f,1.0f)
		IntOpaqueWhite,	// (1,1,1,1)
	};

	enum class DeviceMemoryType {
		None = 0,
		Static,
		Dynamic,
		Immutable = Static,
		Mutable = Dynamic,
		Constant = Static,
		Variable = Dynamic
	};

	enum class DescriptorType {
		None,
		Sampler,
		CombinedSampler,
		Texture,
		ImageBuffer,
		UniformBuffer,
		StorageBuffer,
	};

	enum class ShaderType {
		None,
		VertexShader,
		FragmentShader,
		GeometryShader,
		TessellationControlShader,
		TessellationEvaluationShader
	};

	enum ShaderStage_ {
		ShaderStage_None = 0,
		ShaderStage_VertexShader = BIT(0),
		ShaderStage_FragmentShader = BIT(1),
		ShaderStage_GeometryShader = BIT(2),
		ShaderStage_TessellationControlShader = BIT(3),
		ShaderStage_TessellationEvaluationShader = BIT(4),
	};

	typedef int ShaderStages;

	enum TextureUsage_ {
		TextureUsage_None = 0,
		TextureUsage_Sampled = BIT(0),
		TextureUsage_CPUReadable = BIT(1),
		TextureUsage_Updateable = BIT(2),
		TextureUsage_CopyFrom = BIT(3),
		TextureUsage_CopyTo = BIT(4),
	};

	typedef int TextureUsage;

	enum class TextureSwizzle {
		Identity,
		Zero,
		One,
		Red,
		Green,
		Blue,
		Alpha,
	};

	enum class TextureSamples {
		x1,
		x2,
		x4,
		x8,
		x16,
		x32
	};

	enum class AttachmentLoadOp {
		Load = 1,
		Clear,
		DontCare,
	};

	enum class AttachmentStoreOp {
		Store = 1,
		DontCare,
	};

	union AttachmentColorClearValue {
		glm::vec4 RGBAF;
		glm::ivec4 RGBAI;
		glm::uvec4 RGBAUI;
		AttachmentColorClearValue() = default;
		AttachmentColorClearValue(const glm::vec4& rgbaf)
			:RGBAF(rgbaf)
		{}
		AttachmentColorClearValue(const glm::ivec4& rgbai)
			:RGBAI(rgbai)
		{}
		AttachmentColorClearValue(const glm::uvec4& rgbaui)
			:RGBAUI(rgbaui)
		{}
	};

	struct AttachmentDepthStencilClearValue {
		float Depth;
		uint32_t Stencil;
		AttachmentDepthStencilClearValue() = default;
		AttachmentDepthStencilClearValue(float depth, uint32_t stencil)
			:Depth(depth), Stencil(stencil)
		{}
	};

	union AttachmentClearValue {
		AttachmentColorClearValue ColorClear;
		AttachmentDepthStencilClearValue DepthStencilClear;
	};

	struct TextureSubresourceRegion {
		uint32_t StarMip = 0;
		uint32_t MipCount = 1;
		uint32_t StartLayer = 0;
		uint32_t LayerCount = 1;
	};

	struct BufferToTextureCopyRegion {
		uint64_t BufferOffset = 0;
		uint64_t Mipmap = 0;
		uint64_t StartLayer = 0;
		uint64_t LayerCount = 1;
		glm::ivec3 TextureOffset = {0,0,0};
		glm::ivec3 TextureRegionSize = { 0,0,0 };
	};
}
