#pragma once
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanRenderPass : public RenderPass {
	public:
		VulkanRenderPass(const RenderPassSpecification& specs);
		~VulkanRenderPass();

		virtual const RenderPassSpecification& GetSpecification()const override { return m_Specification; }
		VkRenderPass GetRenderPass() { return m_RenderPass; }
	private:
		VkRenderPass m_RenderPass;
		RenderPassSpecification m_Specification;
	};
}
