#include "KDpch.h"
#include "GraphicsPipeline.h"

#include "Kaidel/Renderer/RendererAPI.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanGraphicsPipeline.h"
namespace Kaidel
{

	Ref<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineSpecification& spec)
	{
		
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::Vulkan:return CreateRef<VulkanGraphicsPipeline>(spec);
		}

		KD_CORE_ASSERT(false, "Unknown renderer api");

		return {};
	}
}

