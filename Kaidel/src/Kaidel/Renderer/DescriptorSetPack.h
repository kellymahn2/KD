#pragma once
#include "Kaidel/Core/Base.h"
#include "GraphicsAPI/DescriptorSet.h"
#include "GraphicsAPI/Shader.h"
#include "RenderCommand.h"

namespace Kaidel {
	
	class DescriptorSetPack : public IRCCounter<false> {
	public:
		DescriptorSetPack() = default;
		DescriptorSetPack(Ref<Shader> shader, 
			const std::unordered_map<uint32_t, Ref<DescriptorSet>>& sets)
			:m_Shader(shader)
		{
			m_Sets.resize(shader->GetSetCount());

			for (uint32_t i = 0; i < shader->GetSetCount(); ++i) {
				auto it = sets.find(i);
				if (it != sets.end()) {
					m_Sets[i] = it->second;
				}
				else {
					m_Sets[i] = DescriptorSet::Create(shader, i);
				}
			}
		}
		void Bind()const {
			for (uint32_t i = 0; i < m_Sets.size(); ++i) {
				RenderCommand::BindDescriptorSet(m_Shader, m_Sets[i], i);
			}
		}

		const std::vector<Ref<DescriptorSet>>& GetSets()const { return m_Sets; }
		Ref<DescriptorSet> GetSet(uint32_t i)const { return m_Sets[i]; }

		Ref<DescriptorSet> operator[](uint32_t index)const { return m_Sets[index]; }

	private:
		std::vector<Ref<DescriptorSet>> m_Sets;
		Ref<Shader> m_Shader;
	};
}
