#pragma once
#include "Kaidel/Core/Base.h"
#include "Kaidel/Renderer/RendererDefinitions.h"

namespace Kaidel {

	struct DescriptorPoolSize {
		DescriptorType Type;
		uint32_t DescriptorCount;
	};

	class DescriptorPool : public IRCCounter<false> {
	public:
		virtual ~DescriptorPool() = default;
		
		static Ref<DescriptorPool> Create(const std::vector<DescriptorPoolSize>& sizes, uint32_t maxSetCount, uint32_t flags);
	};
}
