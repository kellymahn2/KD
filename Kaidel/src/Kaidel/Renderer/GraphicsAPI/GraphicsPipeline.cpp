#include "KDpch.h"
#include "GraphicsPipeline.h"

#include "temp.h"
#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanGraphicsPipeline.h"

namespace Kaidel {
	TMAKE(Ref<GraphicsPipeline>, GraphicsPipeline::Create, TPACK(const GraphicsPipelineSpecification& specs), CreateRef<VulkanGraphicsPipeline>, TPACK(specs));
}
