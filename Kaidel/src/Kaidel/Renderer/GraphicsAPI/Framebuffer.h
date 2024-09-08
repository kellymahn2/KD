#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "RenderPass.h"
#include "Texture.h"

namespace Kaidel {
	
	struct FramebufferAttachment {
		Format AttachmentFormat;
		FramebufferAttachment(Format format)
			:AttachmentFormat(format)
		{}
	};

	struct FramebufferSpecification {
		uint32_t Width = 1, Height = 1;
		TextureSamples Samples = TextureSamples::x1;
		std::vector<FramebufferAttachment> Attachments;

		//Optional
		Ref<RenderPass> OptRenderPass;
	};


	class Framebuffer : public IRCCounter<false> {
	public:
		virtual ~Framebuffer() = default;
		
		virtual const FramebufferSpecification& GetSpecification()const = 0;
		virtual void Recreate(const FramebufferSpecification& newSpecs) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual Ref<Texture2D> GetColorAttachment(uint32_t index)const = 0;


		static Ref<Framebuffer> Create(const FramebufferSpecification& specs);
	};
}
