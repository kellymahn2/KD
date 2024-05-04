#pragma once
#include "Kaidel/Core/Base.h"
#include "Framebuffer.h"

namespace Kaidel {


	enum class RenderPassBindPoint {
		None =0,
		Graphics,
		Compute
	};

	enum class RenderPassImageLoadOp {
		Clear = 0,
		Load,
		DontCare,
	};

	enum class RenderPassImageStoreOp {
		None = 0,
		Store,
		DontCare,
	};


	enum class RenderPassImageLayout {
		None = 0,
		Undefined,
		Color,
		Depth,
		DepthStencil,
		ReadOnly,
		Present,
	};


	struct RenderPassAttachmentSpecification {
		TextureFormat ImageFormat = TextureFormat::None;
		RenderPassImageLayout InitialLayout = RenderPassImageLayout::Undefined;
		RenderPassImageLayout Layout = RenderPassImageLayout::Color;
		RenderPassImageLayout FinalLayout = RenderPassImageLayout::Color;
		RenderPassImageLoadOp LoadOp = RenderPassImageLoadOp::Load;
		RenderPassImageStoreOp StoreOp = RenderPassImageStoreOp::Store;

		RenderPassAttachmentSpecification() = default;
		RenderPassAttachmentSpecification(const RenderPassAttachmentSpecification&) = default;
		RenderPassAttachmentSpecification(TextureFormat format,
											RenderPassImageLoadOp loadOp = RenderPassImageLoadOp::Load, RenderPassImageStoreOp storeOp = RenderPassImageStoreOp::Store, 
											RenderPassImageLayout initialLayout = RenderPassImageLayout::Undefined,RenderPassImageLayout layout = RenderPassImageLayout::Color,
											RenderPassImageLayout finalLayout = RenderPassImageLayout::Color)
			:ImageFormat(format),InitialLayout(initialLayout),Layout(layout),FinalLayout(finalLayout),LoadOp(loadOp),StoreOp(storeOp)
		{}
	};


	struct RenderPassSpecification {
		RenderPassBindPoint BindingPoint;
		std::vector<RenderPassAttachmentSpecification> InputImages;
		std::vector<RenderPassAttachmentSpecification> OutputImages;
		RenderPassAttachmentSpecification OutputDepthAttachment = {};
	};

	class RenderPass : public IRCCounter<false> {
	public:
		virtual ~RenderPass() = default;
		static Ref<RenderPass> Create(const RenderPassSpecification& specification);


		virtual const RenderPassSpecification& GetSpecification()const = 0;

		virtual void Begin()const = 0;
		virtual void End() const = 0;


	};
}
