#pragma once

#include "Kaidel/Renderer/RendererDefinitions.h"

#include <glm/glm.hpp>

namespace Kaidel {

	struct AttachmentSpecification {
		Format AttachmentFormat = Format::None;
		ImageLayout InitialLayout = ImageLayout::None;
		ImageLayout FinalLayout = ImageLayout::ColorAttachmentOptimal;
		AttachmentLoadOp LoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp StoreOp = AttachmentStoreOp::DontCare;
		AttachmentLoadOp StencilLoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp StencilStoreOp = AttachmentStoreOp::DontCare;
	};

	struct RenderPassSpecification {
		std::vector<AttachmentSpecification> OutputColors;
		AttachmentSpecification OutputDepth;
	};


	class RenderPass : public IRCCounter<false> {
	public:

		virtual ~RenderPass() = default;

		virtual RendererID GetRendererID()const = 0;

		virtual const RenderPassSpecification& GetSpecification()const = 0;

		virtual AttachmentClearValue GetClearValue(uint32_t attachmentIndex)const = 0;
		virtual void SetClearValue(uint32_t attachmentIndex, const AttachmentClearValue& clearValue) = 0;

		static Ref<RenderPass> Create(const RenderPassSpecification& specification);

	private:

	};
}
