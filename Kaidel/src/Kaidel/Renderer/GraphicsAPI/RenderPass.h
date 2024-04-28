#pragma once
#include "Kaidel/Core/Base.h"
#include "Framebuffer.h"

namespace Kaidel {


	enum class RenderPassBindPoint {
		None =0,
		Graphics,
		Compute
	};

	struct SubpassSpecification {
		
	};

	struct RenderPassSpecification {
		RenderPassBindPoint BindingPoint;
		std::vector<ImageHandle> OutputImages;

	};

	class RenderPass : public IRCCounter<false> {
	public:
		virtual ~RenderPass() = default;
		static Ref<RenderPass> Create(const RenderPassSpecification& specification);

		virtual void Begin()const = 0;
		virtual void End() const = 0;


	};
}
