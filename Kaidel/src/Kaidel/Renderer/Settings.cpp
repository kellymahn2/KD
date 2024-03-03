#include "KDpch.h"
#include "Settings.h"
#include "GraphicsAPI/UniformBuffer.h"

namespace Kaidel {
	void RendererSettings::SetInShaders(uint32_t slot)const {
		static Ref<UniformBuffer> s_SettingsUniformBuffer = {};
		if (!s_SettingsUniformBuffer)
			s_SettingsUniformBuffer = UniformBuffer::Create(sizeof(RendererSettings), slot);
		s_SettingsUniformBuffer->SetData(this, sizeof(RendererSettings), 0);
		s_SettingsUniformBuffer->Bind(slot);
	}
}
