#pragma once

#include "Kaidel/Core/Base.h"
#include "Constants.h"
#include "Definitions.h"
#include "Image.h"

namespace Kaidel {

	class RenderPass;
	

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
		
		Kaidel::RenderPass* RenderPass;


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
		virtual Image* GetImage(uint32_t index = 0) = 0;

		virtual FramebufferAttachmentHandle GetAttachmentHandle(uint32_t index = 0)const = 0;
		virtual FramebufferImageHandle GetImageHandle(uint64_t index = 0) const = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;
		virtual uint32_t GetWidth()const = 0;
		virtual uint32_t GetHeight()const = 0;

		virtual void ReadValues(uint32_t attachemntIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h, float* output) = 0;

		virtual Ref<RenderPass> GetRenderPass()const = 0;
		
		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};

	struct FramebufferAttachmentHandle {
		Ref<Framebuffer> Framebuffer;
		uint64_t AttachmentIndex;
	};

}
