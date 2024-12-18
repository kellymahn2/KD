#include "KDpch.h"
#include "Sampler.h"

#include "temp.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/Vulkan/GraphicsAPI/VulkanSampler.h"

namespace Kaidel {
	TMAKE(Ref<Sampler>, Sampler::Create, TPACK(const SamplerState& state), CreateRef<VulkanSampler>, TPACK(state));
}
