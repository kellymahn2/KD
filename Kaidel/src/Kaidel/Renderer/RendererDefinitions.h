#pragma once
#include "Kaidel/Core/Base.h"

#include <glm/glm.hpp>

//Fucking windows.h
#undef MemoryBarrier

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
		R8SRGB,
		RG8UN,
		RG8N,
		RG8UI,
		RG8I,
		RG8SRGB,
		RGB8UN,
		RGB8N,
		RGB8UI,
		RGB8I,
		RGB8SRGB,
		RGBA8UN,
		RGBA8N,
		RGBA8UI,
		RGBA8I,
		RGBA8SRGB,
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

	enum class IndexType {
		None,
		Uint16,
		Uint32,
	};

	enum class PipelineCullMode {
		None,
		Front,
		Back,
		FrontAndBack
	};

	enum AccessFlags_{
		AccessFlags_None = 0,
		AccessFlags_IndexRead = BIT(0),
		AccessFlags_VertexAttribureRead = BIT(1),
		AccessFlags_UniformRead = BIT(2),
		AccessFlags_InputAttachmentRead = BIT(3),
		AccessFlags_ShaderRead = BIT(4),
		AccessFlags_ShaderWrite = BIT(5),
		AccessFlags_ColorAttachmentRead = BIT(6),
		AccessFlags_ColorAttachmentWrite = BIT(7),
		AccessFlags_DepthStencilRead = BIT(8),
		AccessFlags_DepthStencilWrite = BIT(9),
		AccessFlags_TransferRead = BIT(10),
		AccessFlags_TransferWrite = BIT(11),
		AccessFlags_HostRead = BIT(12),
		AccessFlags_HostWrite = BIT(13),
		AccessFlags_MemoryRead = BIT(14),
		AccessFlags_MemoryWrite = BIT(15),
	};

	typedef int AccessFlags;

	enum PipelineStages_ {
		PipelineStages_None = 0,
		PipelineStages_TopOfPipe = BIT(0),
		PipelineStages_DrawIndirect = BIT(1),
		PipelineStages_VertexInput = BIT(2),
		PipelineStages_VertexShader = BIT(3),
		PipelineStages_TesselationControlShader = BIT(4),
		PipelineStages_TesselationEvaluationShader = BIT(5),
		PipelineStages_GeometryShader = BIT(6),
		PipelineStages_FragmentShader = BIT(7),
		PipelineStages_EarlyFragmentTests = BIT(8),
		PipelineStages_LateFragmentTests = BIT(9),
		PipelineStages_ColorAttachmentOutput = BIT(10),
		PipelineStages_ComputeShader = BIT(11),
		PipelineStages_Transfer = BIT(12),
		PipelineStages_BottomOfPipe = BIT(13),
		PipelineStages_Host = BIT(14),
		PipelineStages_AllGraphics = BIT(15),
		PipelineStages_AllCommandsBit = BIT(16),
	};

	typedef int PipelineStages;

	enum AspectMask_ {
		AspectMask_None = 0,
		AspectMask_Color = BIT(0),
		AspectMask_Depth = BIT(1)
	};

	typedef int AspectMask;

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


	enum class PrimitiveTopology {
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TrinagleStrip,
		TriangleFan,
		LineListAdj,
		LineStripAdj,
		TriangleListAd,
		TriangleStripAdj,
		PatchList,
	};

	enum class BlendFactor {
		Zero = 0,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha,
		ConstantColor,
		OneMinusConstantColor,
		ConstantAlpha,
		OneMinusConstantAlpha,
		SrcAlphaSaturate,
		Src1_Color,
		OneMinusSrc1Color,
		Src1Alpha,
		OneMinusSrc1Alpha,
	};

	enum class BlendOp {
		Add = 0,
		Subtract,
		ReverseSubtract,
		Min,
		Max,
	};

	enum class StencilOp {
		Keep = 0,
		Zero,
		Replace,
		IncrementAndClamp,
		DecrementAndClamp,
		Invert,
		IncrementAndWrap,
		DecrementAndWrap,
	};

	enum DependencyFlags_ {
		DependencyFlags_Region = 1,
		DependencyFlags_ViewLocal = 2,
		DependencyFlags_Device = 4,
	};
	
	typedef int DependencyFlags;

	enum class LogicOp {
		None = 0,
		Clear,
		And,
		AndReverse,
		Copy,
		AndInverted,
		Xor,
		Or,
		Nor,
		Equivalent,
		Invert,
		OrReverse,
		CopyInverted,
		OrInverted,
		Nand,
		Set,
	};

	enum CompareOp {
		Never = 0,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always,
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
		Sampler, // sampler
		SamplerWithTexture, // samplerXX
		Texture, // textureXX
		Image, // imageXX
		TextureBuffer, // textureBuffer
		SamplerBuffer, // samplerBuffer
		ImageBuffer, // imageBuffer
		UniformBuffer, // uniform
		StorageBuffer, // buffer
		Count
	};

	enum class ImageType {
		None = 0,
		_1D,
		_2D,
		_3D,
		_1D_Array,
		_2D_Array,
		Reference
	};

	enum class ShaderType {
		None,
		VertexShader,
		FragmentShader,
		GeometryShader,
		TessellationControlShader,
		TessellationEvaluationShader,
		ComputeShader,
	};

	enum ShaderStage_ {
		ShaderStage_None = 0,
		ShaderStage_VertexShader = BIT(0),
		ShaderStage_FragmentShader = BIT(1),
		ShaderStage_GeometryShader = BIT(2),
		ShaderStage_TessellationControlShader = BIT(3),
		ShaderStage_TessellationEvaluationShader = BIT(4),
		ShaderStage_ComputeShader = BIT(5)
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
		x1 = 1,
		x2 = 2,
		x4 = 4,
		x8 = 8,
		x16 = 16,
		x32 = 32
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
		AttachmentClearValue(const AttachmentColorClearValue& color)
			:ColorClear(color)
		{}
		AttachmentClearValue(const AttachmentDepthStencilClearValue& depth)
			:DepthStencilClear(depth)
		{}
	};

	struct AttachmentClear {
		AttachmentClearValue Value;
		uint32_t ColorAttachment;
		AspectMask Aspect;
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

	struct Rect2D {
		glm::ivec2 Offset;
		glm::uvec2 Size;
		Rect2D(uint32_t width,uint32_t height, int32_t x = 0,int32_t y = 0)
			:Offset(x,y),Size(width,height)
		{}
	};

	struct ClearRect {
		Rect2D Rect;
		uint32_t StartLayer = 0;
		uint32_t LayerCount = 1;
	};

	namespace Utils {

		static uint32_t CalculateChannelSize(Format format) {
			switch (format)
			{
			case Format::R8UN:
			case Format::R8N:
			case Format::R8UI:
			case Format::R8I:
				return 1;
			case Format::RG8UN:
			case Format::RG8N:
			case Format::RG8UI:
			case Format::RG8I:
				return 1;
			case Format::RGB8UN:
			case Format::RGB8N:
			case Format::RGB8UI:
			case Format::RGB8I:
				return 1;
			case Format::RGBA8UN:
			case Format::RGBA8SRGB:
			case Format::RGBA8N:
			case Format::RGBA8UI:
			case Format::RGBA8I:
				return 1;
			case Format::R16UN:
			case Format::R16N:
			case Format::R16UI:
			case Format::R16I:
			case Format::R16F:
				return 2;
			case Format::RG16UN:
			case Format::RG16N:
			case Format::RG16UI:
			case Format::RG16I:
			case Format::RG16F:
				return 2;
			case Format::RGB16UN:
			case Format::RGB16N:
			case Format::RGB16UI:
			case Format::RGB16I:
			case Format::RGB16F:
				return 2;
			case Format::RGBA16UN:
			case Format::RGBA16N:
			case Format::RGBA16UI:
			case Format::RGBA16I:
			case Format::RGBA16F:
				return 2;
			case Format::R32UI:
			case Format::R32I:
			case Format::R32F:
				return 4;
			case Format::RG32UI:
			case Format::RG32I:
			case Format::RG32F:
				return 4;
			case Format::RGB32UI:
			case Format::RGB32I:
			case Format::RGB32F:
				return 4;
			case Format::RGBA32UI:
			case Format::RGBA32I:
			case Format::RGBA32F:
				return 4;
			case Format::Depth24Stencil8:
				return 4;
			case Format::Depth32F:
				return 4;
			}
			return 0;
		}

		static uint32_t CalculateChannelCount(Format format) {
			switch (format) {
			case Format::R8UN:
			case Format::R8N:
			case Format::R8UI:
			case Format::R8I:
			case Format::R16UN:
			case Format::R16N:
			case Format::R16UI:
			case Format::R16I:
			case Format::R16F:
			case Format::R32UI:
			case Format::R32I:
			case Format::R32F:
				return 1;
			case Format::RG8UN:
			case Format::RG8N:
			case Format::RG8UI:
			case Format::RG8I:
			case Format::RG16UN:
			case Format::RG16N:
			case Format::RG16UI:
			case Format::RG16I:
			case Format::RG16F:
			case Format::RG32UI:
			case Format::RG32I:
			case Format::RG32F:
				return 2;
			case Format::RGB8UN:
			case Format::RGB8N:
			case Format::RGB8UI:
			case Format::RGB8I:
			case Format::RGB16UN:
			case Format::RGB16N:
			case Format::RGB16UI:
			case Format::RGB16I:
			case Format::RGB16F:
			case Format::RGB32UI:
			case Format::RGB32I:
			case Format::RGB32F:
				return 3;

			case Format::RGBA8UN:
			case Format::RGBA8SRGB:
			case Format::RGBA8N:
			case Format::RGBA8UI:
			case Format::RGBA8I:
			case Format::RGBA16UN:
			case Format::RGBA16N:
			case Format::RGBA16UI:
			case Format::RGBA16I:
			case Format::RGBA16F:
			case Format::RGBA32UI:
			case Format::RGBA32I:
			case Format::RGBA32F:
				return 4;
			case Format::Depth24Stencil8:
			case Format::Depth32F:
				return 1;
			}
			return 0;
		}

		static uint32_t CalculatePixelSize(Format format) {
			return CalculateChannelSize(format) * CalculateChannelCount(format);
		}


	}

}
