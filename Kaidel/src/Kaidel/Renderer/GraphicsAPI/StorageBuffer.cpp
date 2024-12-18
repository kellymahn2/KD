#include "KDpch.h"
#include "StorageBuffer.h"

#include "Kaidel/Renderer/Renderer.h"
#include "temp.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanStorageBuffer.h"

namespace Kaidel{
    TMAKE(Ref<StorageBuffer>, StorageBuffer::Create,TPACK(uint64_t size),CreateRef<VulkanStorageBuffer>,TPACK(size));
}
