#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"

namespace Kaidel {

	struct RenderPassAttachment {
		Format Format = Format::None;
		TextureSamples Samples = TextureSamples::x1;
		AttachmentLoadOp LoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp StoreOp = AttachmentStoreOp::DontCare;
		AttachmentLoadOp StencilLoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp StencilStoreOp = AttachmentStoreOp::DontCare;
		ImageLayout InitialLayout = ImageLayout::None;
		ImageLayout FinalLayout = ImageLayout::None;
	};

	struct AttachmentReference {
		uint32_t Attachment = -1;
		ImageLayout Layout = ImageLayout::None;
		AspectMask Aspects;
	};

	struct Subpass {
		std::vector<AttachmentReference> Inputs;
		std::vector<AttachmentReference> Colors;
		AttachmentReference DepthStencil;
		std::vector<AttachmentReference> Resolves;
		std::vector<uint32_t> Preserves;
		AttachmentReference VRS;
	};

	struct SubpassDependency {
		uint32_t Src = -1;
		uint32_t Dst = -1;
		PipelineStages SrcStages;
		PipelineStages DstStages;
		AccessFlags SrcAccesses;
		AccessFlags DstAccesses;
	};

	struct RenderPassSpecification {
		std::vector<RenderPassAttachment> Attachments;
		std::vector<Subpass> Subpasses;
		std::vector<SubpassDependency> Dependencies;
	};

	class RenderPass : public IRCCounter<false> {
	public:
		virtual ~RenderPass() = default;
		
		virtual const RenderPassSpecification& GetSpecification()const = 0;
		static Ref<RenderPass> Create(const RenderPassSpecification& specs);
	};


}
