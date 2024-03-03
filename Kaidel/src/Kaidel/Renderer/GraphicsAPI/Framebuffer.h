#pragma once

#include "Kaidel/Core/Base.h"
#include "Core.h"
namespace Kaidel {

	

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(TextureFormat format,bool isReadable = false)
			: Format(format),Readable(isReadable) {}

		TextureFormat Format = TextureFormat::None;
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

	struct FramebufferAttachmentHandle;

	class Framebuffer : public IRCCounter<false>
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Resample(uint32_t newSampleCount) = 0;


		virtual void ClearAttachment(uint32_t attachmentIndex, const void* colors) = 0;
		virtual void ClearDepthAttachment(float value) = 0;
		

		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual FramebufferAttachmentHandle GetAttachmentHandle(uint32_t index = 0)const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;


		virtual void BindColorAttachmentToSlot(uint32_t attachmentIndex, uint32_t slot)=0;
		virtual void BindColorAttachmentToImageSlot(uint32_t attachmnetIndex, uint32_t slot, ImageBindingMode bindingMode)=0;
		virtual void BindDepthAttachmentToSlot(uint32_t slot) = 0;


		virtual void CopyColorAttachment(uint32_t dstAttachmentIndex, uint32_t srcAttachmentIndex, Ref<Framebuffer> src) = 0;
		virtual void CopyDepthAttachment(Ref<Framebuffer> src) = 0;


		virtual void EnableColorAttachment(uint32_t attachmentIndex) = 0;
		virtual void DisableColorAttachment(uint32_t attachmentIndex) = 0;

		virtual void ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) = 0;

		virtual void SetAttachment(const TextureHandle& handle, uint32_t index = 0) = 0;
		virtual void SetAttachment(const TextureArrayHandle& handle, uint32_t index = 0) = 0;
		virtual void SetDepthAttachment(const TextureHandle& handle) = 0;
		virtual void SetDepthAttachment(const TextureArrayHandle& handle) = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};

	struct FramebufferAttachmentHandle {
		Ref<Framebuffer> Framebuffer;
		uint64_t AttachmentIndex;
	};

}
