#pragma once
#include "PerFrameResource.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"
#include <glad/vulkan.h>

namespace Kaidel {
	class VulkanDescriptorSet : public DescriptorSet {
	public:
		VulkanDescriptorSet(Ref<GraphicsPipeline> pipeline, uint32_t setBinding);
		VulkanDescriptorSet(DescriptorType type, ShaderStages flags);
		~VulkanDescriptorSet();
		RendererID GetSetID() const override { return (RendererID)*m_Sets; }

		void Update(const DescriptorSetUpdate& update) override;
	private:
		PerFrameResource<VkDescriptorSet> m_Sets;
	};
}
