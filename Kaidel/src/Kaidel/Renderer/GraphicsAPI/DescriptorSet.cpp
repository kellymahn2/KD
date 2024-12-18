#include "KDpch.h"
#include "DescriptorSet.h"

#include "temp.h"
#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanDescriptorSet.h"

namespace Kaidel {
	TMAKE(Ref<DescriptorSet>, DescriptorSet::Create, TPACK(const DescriptorSetLayoutSpecification& specs), CreateRef<VulkanDescriptorSet>, TPACK(specs))
	TMAKE(Ref<DescriptorSet>, DescriptorSet::Create, TPACK(Ref<Shader> shader, uint32_t setIndex), CreateRef<VulkanDescriptorSet>, TPACK(shader, setIndex))
}
