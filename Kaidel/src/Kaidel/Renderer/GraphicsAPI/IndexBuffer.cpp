#include "KDpch.h"
#include "IndexBuffer.h"
#include "Kaidel/Renderer/Renderer.h"
#include "temp.h"

#include "Platform/Vulkan/GraphicsAPI/VulkanIndexBuffer.h"

namespace Kaidel{
    TMAKE(Ref<IndexBuffer>, IndexBuffer::Create, TPACK(const void* indices, uint64_t size, IndexType indexType),CreateRef<VulkanIndexBuffer>,TPACK(indices,size,indexType))
}
