#pragma once
#include "Kaidel/Core/Base.h"
#include "Shader.h"

namespace Kaidel {
	class ComputePipeline : public IRCCounter<false> {
	public:
		virtual ~ComputePipeline() = default;
		static Ref<ComputePipeline> Create(Ref<Shader> shader);
	};
}
