#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"

namespace Kaidel {

	struct RenderPassAttachment {
		Format AttachmentFormat = Format::None;
		TextureSamples Samples = TextureSamples::x1;
		AttachmentLoadOp LoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp StoreOp = AttachmentStoreOp::DontCare;
		AttachmentLoadOp StencilLoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp StencilStoreOp = AttachmentStoreOp::DontCare;
		ImageLayout InitialLayout = ImageLayout::None;
		ImageLayout FinalLayout = ImageLayout::None;

		RenderPassAttachment() = default;

		RenderPassAttachment(Format format,ImageLayout initLayout,ImageLayout finalLayout, TextureSamples samples)
			:AttachmentFormat(format), 
			Samples(samples),
			LoadOp(AttachmentLoadOp::Load), 
			StoreOp(AttachmentStoreOp::Store),
			InitialLayout(initLayout),
			FinalLayout(finalLayout)
		{}
	};

	struct RenderPassSpecification {
		std::vector<RenderPassAttachment> Inputs;
		std::vector<RenderPassAttachment> Colors;
		std::vector<RenderPassAttachment> Resolves;
		RenderPassAttachment DepthStencil;
		RenderPassAttachment VRS;
	};

	class RenderPass : public IRCCounter<false> {
	public:
		virtual ~RenderPass() = default;
		
		virtual const RenderPassSpecification& GetSpecification()const = 0;
		static Ref<RenderPass> Create(const RenderPassSpecification& specs);
	};


}
