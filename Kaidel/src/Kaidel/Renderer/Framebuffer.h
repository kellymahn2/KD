#pragma once

#include "Kaidel/Core/Base.h"

namespace Kaidel {

	enum class FramebufferTextureFormat
	{
		None = 0,

		RGBA32F,//4-component 128-bits
		RGBA32UI,//4-component 128-bits
		RGBA32I,//4-component 128-bits

		RGB32F,//3-component 96-bits
		RGB32UI,//3-component 96-bits
		RGB32I,//3-component 96-bits

		RGBA16F,//4-component 64-bits
		RGBA16,//4-component 64-bits
		RGBA16UI	,//4-component 64-bits
		RGBA16NORM,//4-component 64-bits
		RGBA16I	,//4-component 64-bits

		RG32F,//2-component 64-bits
		RG32UI,//2-component 64-bits
		RG32I,//2-component 64-bits
		
		RGBA8,//4-component 32-bits
		RGBA8UI,//4-component 32-bits
		RGBA8NORM,//4-component 32-bits
		RGBA8I,//4-component 32-bits
		
		RG16F,//2-component 32-bits
		RG16,//2-component 32-bits
		RG16UI,//2-component 32-bits
		RG16NORM,//2-component 32-bits
		RG16I,//2-component 32-bits

		R32F,//1-component 32-bits
		R32UI,//1-component 32-bits
		R32I,//1-component 32-bits
		
		RG8,//2-component 16-bits
		RG8UI,//2-component 16-bits
		RG8NORM,//2-component 16-bits
		RG8I,//2-component 16-bits

		R16F,//1-component 16-bits
		R16,//1-component 16-bits
		R16UI,//1-component 16-bits
		R16NORM,//1-component 16-bits
		R16I,//1-component 16-bits

		R8,//1-component 8-bits
		R8UI,//1-component 8-bits
		R8NORM,//1-component 8-bits
		R8I,//1-component 8-bits
		

		// Depth/stencil
		DEPTH24STENCIL8,
		DEPTH32,
		DEPTH16,



		DepthStencil = DEPTH24STENCIL8,
		Depth = DEPTH32
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format,bool isReadable = false)
			: TextureFormat(format),Readable(isReadable) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		bool Readable;
		// TODO: filtering/wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	enum ImageBindigMode_ {
		ImageBindigMode_None = 0,ImageBindingMode_Write = 1<<0,ImageBindingMode_Read = 1<<1 
	};

	typedef int ImageBindingMode;
	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, const float* colors) = 0;
		virtual void ClearDepthAttachment(float value) = 0;

		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual void SetDepthAttachmentFromArray(uint32_t attachmentIndex, uint32_t arrayIndex) = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;
		virtual void BindColorAttachmentToSlot(uint32_t attachmentIndex, uint32_t slot)=0;
		virtual void BindColorAttachmentToImageSlot(uint32_t attachmnetIndex, uint32_t slot, ImageBindingMode bindingMode)=0;
		virtual void BindDepthAttachmentToSlot(uint32_t slot) = 0;

		virtual void CopyColorAttachment(uint32_t dstAttachmentIndex, uint32_t srcAttachmentIndex, Ref<Framebuffer> src) = 0;
		virtual void CopyDepthAttachment(Ref<Framebuffer> src) = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
		

	};


}
