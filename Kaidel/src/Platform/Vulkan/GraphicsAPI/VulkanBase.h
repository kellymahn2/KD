#pragma once

#include "Kaidel/Core/Base.h"
#include <glad/vulkan.h>

#include <string>
#include <vector>
#include <optional>


#define VK_ASSERT(result) KD_CORE_ASSERT((result)==VK_SUCCESS,"Error while doing a vulkan operation")


#define VK_ALLOCATOR_PTR nullptr

#define VK_STRUCT(type,name,structType) type name = {};name.sType = structType;
