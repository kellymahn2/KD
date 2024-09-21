#include "KDpch.h"
#include "ComputePipeline.h"

#include "temp.h"
#include "Kaidel/Renderer/Renderer.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanComputePipeline.h"

namespace Kaidel {
	TMAKE(Ref<ComputePipeline>, ComputePipeline::Create, TPACK(Ref<Shader> shader), CreateRef<VulkanComputePipeline>, TPACK(shader))
}
