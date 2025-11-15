#pragma once

#include "Kaidel/Renderer/RendererDefinitions.h"

#include <unordered_map>
#include <string>

namespace Kaidel {

	struct ShaderUniformBlockReflection {
		std::string Name;
		uint32_t Size;
		std::unordered_map<std::string, uint32_t> Elements;
	};

	struct DescriptorReflection
	{
		ShaderUniformBlockReflection Uniform;
	};

	struct DescriptorSetBindingReflection
	{
		std::string Name;
		uint32_t Binding = 0;
		uint32_t Count = 0;
		DescriptorType Type = DescriptorType::Count;
		Kaidel::ShaderStages ShaderStages = 0;
		DescriptorReflection Descriptor;
	};

	struct DescriptorSetReflection
	{
		uint32_t Set;
		std::unordered_map<uint32_t, DescriptorSetBindingReflection> Bindings;
		std::unordered_map<std::string, uint32_t> NameToBinding;
	};

	struct VertexShaderInputReflection {
		std::string Name;
		uint32_t Location;
	};

	struct ShaderReflection
	{
		std::unordered_map<uint32_t, VertexShaderInputReflection> Inputs;
		std::unordered_map<uint32_t, DescriptorSetReflection> Sets;
		uint32_t PushConstantSize;

		void AddDescriptor(const std::string& name, const DescriptorReflection& descriptor, DescriptorType type, uint32_t count, uint32_t set, uint32_t binding, Kaidel::ShaderStages stage) {
			Sets[set].Set = set;
			Sets[set].NameToBinding[name] = binding;

			DescriptorSetBindingReflection& setBinding = Sets[set].Bindings[binding];
			setBinding.Binding = binding;
			setBinding.Type = type;
			setBinding.Count = count;
			setBinding.Name = name;
			setBinding.ShaderStages |= stage;
			setBinding.Descriptor = descriptor;
		}

		void Add(const ShaderReflection& reflection) {
			for (auto& [setIndex, set] : reflection.Sets) {
				Sets[setIndex].Set = setIndex;
				Add(Sets[setIndex], set);
			}
		}
		void Add(DescriptorSetReflection& dst, const DescriptorSetReflection& src) {
			for (auto& [bindingIndex, binding] : src.Bindings) {
				dst.Bindings[bindingIndex] = binding;
			}
		}
	};
}
