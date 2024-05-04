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
		private:
			RenderPassSpecification m_Specification;
			VkRenderPass m_RenderPass = VK_NULL_HANDLE;
			VkCommandBuffer m_RecordCommandBuffer = VK_NULL_HANDLE;
		};
	}
}
