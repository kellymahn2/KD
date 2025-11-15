#pragma once
#include "Kaidel/Core/Base.h"

#include "Kaidel/Renderer/RendererDefinitions.h"
#include "ShaderReflection.h"

namespace Kaidel {

	using Spirv = std::vector<uint32_t>;
	
	class GraphicsPipeline;

	class Shader : public IRCCounter<false> {
	public:

		virtual ~Shader() = default;
		static Ref<Shader> Create(const std::unordered_map<ShaderType, Spirv>& spirvs);
		virtual uint32_t GetSetCount()const = 0;
		virtual void Update(const std::unordered_map<ShaderType, Spirv>& spirvs) = 0;
		virtual void BindPipeline(Ref<GraphicsPipeline> pipeline) = 0;

		virtual const ShaderReflection& GetReflection() const = 0;
	};
}

