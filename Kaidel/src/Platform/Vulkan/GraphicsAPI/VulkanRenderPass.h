#pragma once

#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"

#include "Platform/Vulkan/VulkanDefinitions.h"

namespace Kaidel
{
	class VulkanRenderPass : public RenderPass 
	{
	public:

		VulkanRenderPass(const RenderPassSpecification& specification);
		~VulkanRenderPass();

		RendererID GetRendererID() const override { return (RendererID)m_RenderPass; }

		void SetClearValue(uint32_t attachmentIndex, const AttachmentClearValue& clearValue) override;

		const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

		AttachmentClearValue GetClearValue(uint32_t attachmentIndex) const override;

		const std::vector<VkClearValue>& GetClearValues()const { return m_ClearValues; }

	private:
		VkRenderPass m_RenderPass;
		std::vector<VkClearValue> m_ClearValues;
		RenderPassSpecification m_Specification;
	};

}
