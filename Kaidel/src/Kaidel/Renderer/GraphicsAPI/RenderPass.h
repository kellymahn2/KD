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

		RenderPassAttachment(
			Format format,
			ImageLayout initLayout,
			ImageLayout finalLayout,
			TextureSamples samples,
			AttachmentLoadOp loadOp,
			AttachmentStoreOp storeOp)
			: AttachmentFormat(format),
			Samples(samples),
			LoadOp(loadOp),
			StoreOp(storeOp),
			InitialLayout(initLayout),
			FinalLayout(finalLayout)
		{}

	};

	struct SubpassDependency {
		PipelineStages SrcStages;
		PipelineStages DstStages;
		AccessFlags SrcAccess;
		AccessFlags DstAccess;
		DependencyFlags Flags;
	};

	struct RenderPassSpecification {
		std::vector<RenderPassAttachment> Inputs;
		std::vector<RenderPassAttachment> Colors;
		RenderPassAttachment DepthStencil;
		RenderPassAttachment VRS;
		std::vector<SubpassDependency> Dependencies;
		bool AutoResolve = false;
	};

	class RenderPass : public IRCCounter<false> {
	public:
		virtual ~RenderPass() = default;
		
		virtual const RenderPassSpecification& GetSpecification()const = 0;
		
		virtual void SetColorClearValue(const AttachmentColorClearValue& clearValue, uint32_t index) = 0;
		virtual void SetDepthClearValue(const AttachmentDepthStencilClearValue& clearValue) = 0;
		
		static Ref<RenderPass> Create(const RenderPassSpecification& specs);



	};


}
