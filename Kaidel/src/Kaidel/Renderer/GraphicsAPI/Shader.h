#pragma once
#include "Buffer.h"
#include "Kaidel/Renderer/RendererDefinitions.h"



#include <filesystem>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Kaidel {

	struct ShaderSpecification {
		std::string EntryPoint;
		std::vector<uint32_t> SPIRV;
		ShaderType Type;
	};


	struct DescriptorSetBindingReflection {
		uint32_t Binding;
		uint32_t Count;
		DescriptorType Type;
	};


	struct DescriptorSetReflection {
		uint32_t Set;
		std::unordered_map<uint32_t,DescriptorSetBindingReflection> Bindings;
	};


	struct PushConstantRangeReflection {
		uint32_t Size;
		ShaderType Type;
	};

	class ShaderReflection {
	public:

		const std::unordered_map<uint32_t,DescriptorSetReflection>& GetSets()const { return  m_Sets; }
		const std::vector<PushConstantRangeReflection>& GetPushConstants()const { return m_PushConstants; }


		void AddPushConstant(uint32_t size,ShaderType type) {
			m_PushConstants.push_back({ size, type});
		}

		void AddDescriptor(DescriptorType type, uint32_t count, uint32_t set, uint32_t binding) {
			m_Sets[set].Set = set;
			DescriptorSetBindingReflection setBinding;
			setBinding.Binding = binding;
			setBinding.Type = type;
			setBinding.Count = count;
			m_Sets[set].Bindings[binding] = setBinding;
		}

		void Add(const ShaderReflection& reflection) {
			for (auto& [setIndex, set] : reflection.m_Sets) {
				m_Sets[setIndex].Set = setIndex;
				Add(m_Sets[setIndex], set);
			}
			m_PushConstants.insert(m_PushConstants.end(), reflection.m_PushConstants.begin(), reflection.m_PushConstants.end());
		}

	private:
		void Add(DescriptorSetReflection& dst, const DescriptorSetReflection& src) {
			KD_CORE_ASSERT(dst.Set == src.Set);
			for (auto& [bindingIndex, binding] : src.Bindings) {
				dst.Bindings[bindingIndex] = binding;
			}
		}

	private:
		std::vector<PushConstantRangeReflection> m_PushConstants;
		std::unordered_map<uint32_t,DescriptorSetReflection> m_Sets;
	};



	class ShaderModule : public IRCCounter<false>
	{
	public:
		virtual ~ShaderModule() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual RendererID GetRendererID()const = 0;

		virtual const ShaderSpecification& GetSpecification()const = 0;
		
		static Ref<ShaderModule> Create(const ShaderSpecification& specification);

		virtual ShaderReflection& Reflect() = 0;


	};
}
