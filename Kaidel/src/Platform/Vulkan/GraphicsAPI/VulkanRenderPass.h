#pragma once



#include "VulkanBase.h"
#include "Kaidel/Renderer/GraphicsAPI/RenderPass.h"

namespace Kaidel {
	namespace Vulkan {

		class VulkanRenderPass : public RenderPass {
		public:
			VulkanRenderPass(const RenderPassSpecification& specification);
			~VulkanRenderPass();


		private:
			VkRenderPass m_RenderPass = VK_NULL_HANDLE;
			VkCommandBuffer m_RecordCommandBuffer = VK_NULL_HANDLE;
			// Inherited via RenderPass
			void Begin() const override;
			void End() const override;
		};
	}
}
