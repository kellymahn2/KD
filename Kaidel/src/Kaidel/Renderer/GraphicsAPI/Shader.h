#pragma once
#include "Kaidel/Core/Base.h"

#include "Kaidel/Renderer/RendererDefinitions.h"

namespace Kaidel {

	using Spirv = std::vector<uint32_t>;

	class Shader : public IRCCounter<false> {
	public:

		virtual ~Shader() = default;
		static Ref<Shader> Create(const std::unordered_map<ShaderType, Spirv>& spirvs);
	};
}

