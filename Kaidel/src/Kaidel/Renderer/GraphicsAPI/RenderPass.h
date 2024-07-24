#pragma once

#include "Kaidel/Renderer/RendererDefinitions.h"

#include <glm/glm.hpp>

namespace Kaidel {

	struct AttachmentSpecification {
		Format AttachmentFormat = Format::None;
		ImageLayout InitialLayout = ImageLayout::None;
		ImageLayout FinalLayout = ImageLayout::ColorAttachmentOptimal;
	};

	struct RenderPassSpecification {
		std::vector<AttachmentSpecification> OutputColors;
		AttachmentSpecification OutputDepth;
	};

	union AttachmentColorClearValue {
		glm::vec4 RGBAF;
		glm::ivec4 RGBAI;
		glm::uvec4 RGBAUI;
		AttachmentColorClearValue() = default;
		AttachmentColorClearValue(const glm::vec4& rgbaf)
			:RGBAF(rgbaf)
		{}
		AttachmentColorClearValue(const glm::ivec4& rgbai)
			:RGBAI(rgbai)
		{}
		AttachmentColorClearValue(const glm::uvec4& rgbaui)
			:RGBAUI(rgbaui)
		{}
	};

	struct AttachmentDepthStencilClearValue {
		float Depth;
		uint32_t Stencil;
		AttachmentDepthStencilClearValue() = default;
		AttachmentDepthStencilClearValue(float depth, uint32_t stencil)
			:Depth(depth),Stencil(stencil)
		{}
	};

	union AttachmentClearValue {
		AttachmentColorClearValue ColorClear;
		AttachmentDepthStencilClearValue DepthStencilClear;
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
