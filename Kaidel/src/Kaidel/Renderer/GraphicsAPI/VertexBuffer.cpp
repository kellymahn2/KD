#include "KDpch.h"
#include "VertexBuffer.h"
#include "Kaidel/Renderer/Renderer.h"
#include "temp.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanVertexBuffer.h"

namespace Kaidel {
	TMAKE(Ref<VertexBuffer>, VertexBuffer::Create, TPACK(uint64_t size), CreateRef<VulkanVertexBuffer>, TPACK(size))
}

