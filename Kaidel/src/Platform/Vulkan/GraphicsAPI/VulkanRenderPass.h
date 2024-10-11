#pragma once
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"
#include "Backend.h"

namespace Kaidel {
	class VulkanRenderPass : public RenderPass {
	public:
		VulkanRenderPass(const RenderPassSpecification& specs);
		~VulkanRenderPass();

		virtual void SetColorClearValue(const AttachmentColorClearValue& clearValue, uint32_t index)override {
			m_ClearValues[index] = (const VkClearValue&)clearValue;

			if (m_Specification.AutoResolve) {
				m_ClearValues[m_ResolvesOffset + index] = (const VkClearValue&)clearValue;
			}
		}
		virtual void SetDepthClearValue(const AttachmentDepthStencilClearValue& clearValue)override {
			m_ClearValues.back() = (const VkClearValue&)clearValue;
		}

		virtual const RenderPassSpecification& GetSpecification()const override { return m_Specification; }
		VkRenderPass GetRenderPass()const { return m_RenderPass; }

		const auto& GetVkClearValues()const { return m_ClearValues; }

	private:
		VkRenderPass m_RenderPass;
		RenderPassSpecification m_Specification;

		std::vector<VkClearValue> m_ClearValues;
		uint32_t m_ResolvesOffset = 0;
	};
}
