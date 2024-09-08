#include "KDpch.h"
#include "DescriptorSet.h"

#include "temp.h"
#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanDescriptorSet.h"

namespace Kaidel {
	TMAKE(Ref<DescriptorSet>, DescriptorSet::Create, TPACK(const DescriptorSetSpecification& specs), CreateRef<VulkanDescriptorSet>, TPACK(specs))
}
