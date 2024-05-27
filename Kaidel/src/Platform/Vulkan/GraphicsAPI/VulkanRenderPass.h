#pragma once



#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"

namespace Kaidel {
	namespace Vulkan {

		class VulkanRenderPass : public RenderPass {
		public:
			VulkanRenderPass(const RenderPassSpecification& specification);
			~VulkanRenderPass();

			void Begin() const override;
			void End() const override;

			const RenderPassSpecification& GetSpecification()const override { return m_Specification; }

			VkRenderPass GetRenderPass()const { return m_RenderPass; }

			void SetClearColor(const glm::vec4& color) override {
				for (auto& col : m_ClearColors) {
					SetClearColor(col, color);
				}
			}

			void SetClearColor(const glm::vec4& color, uint32_t attachment) override { SetClearColor(m_ClearColors[attachment], color); }

			const std::vector<VkClearValue>& GetClearColors()const { return m_ClearColors; }

		private:

			inline void SetClearColor(VkClearValue& value, const glm::vec4& color) {
				value.color.float32[0] = color.r;
				value.color.float32[1] = color.g;
				value.color.float32[2] = color.b;
				value.color.float32[3] = color.a;
			}

			RenderPassSpecification m_Specification;
			VkRenderPass m_RenderPass = VK_NULL_HANDLE;
			VkCommandBuffer m_RecordCommandBuffer = VK_NULL_HANDLE;

			std::vector<VkClearValue> m_ClearColors;

			// Inherited via RenderPass
		};
	}
}
