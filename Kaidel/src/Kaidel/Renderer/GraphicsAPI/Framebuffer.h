#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "RenderPass.h"
#include "Texture.h"

namespace Kaidel {
	
	struct FramebufferSpecification {
		Ref<RenderPass> RenderPass;
		uint32_t Width = 1, Height = 1;
	};


	class Framebuffer : public IRCCounter<false> {
	public:
		virtual ~Framebuffer() = default;
		
		virtual const FramebufferSpecification& GetSpecification()const = 0;
		virtual void Recreate(const FramebufferSpecification& newSpecs) = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual Ref<Texture2D> GetColorAttachment(uint32_t index)const = 0;
		virtual Ref<Texture2D> GetDepthAttachment()const = 0;
		virtual Ref<Texture2D> GetResolveAttachment(uint32_t index)const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& specs);
	};
}
