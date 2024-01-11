#pragma once

#include "Kaidel/Core/Base.h"

namespace Kaidel {

	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		//Position/Normals
		RGBA32F,

		// Depth/stencil
		DEPTH24STENCIL8,
		//TODO: Implement
		DEPTH32,
		DEPTH16,



		// Defaults
		DepthStencil = DEPTH24STENCIL8,
		Depth = DEPTH16
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
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
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

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
		

	};


}
