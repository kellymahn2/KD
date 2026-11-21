#include "KDpch.h"
#include "UniformBuffer.h"

#include "Kaidel/Renderer/Renderer.h"
#include "temp.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanUniformBuffer.h"
namespace Kaidel {
	TMAKE(Ref<UniformBuffer>, UniformBuffer::Create, TPACK(uint64_t size, const void* initData), CreateRef<VulkanUniformBuffer>, TPACK(size, initData));
}
