#include "KDpch.h"
#include "Pipeline.h"
#include "Kaidel/Renderer/RendererAPI.h"

namespace Kaidel {
	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& specification)
	{
		//switch (RendererAPI::GetAPI())
		//{
		////case RendererAPI::API::Vulkan:return CreateRef<VulkanPipeline>(specification);
		//}

		KD_CORE_ASSERT(false, "Unknown renderer api");
		return Ref<Pipeline>();
	}
}
