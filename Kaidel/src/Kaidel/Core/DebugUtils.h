#pragma once


#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

namespace Kaidel {
	class DebugUtils
	{
	public:

		static void AddDebugTexture(const std::string& name, Ref<Texture> texture)
		{
			m_DebugTextures[name] = texture;
		}

		static std::unordered_map<std::string, Ref<Texture>>& GetDebugTextures()
		{
			return m_DebugTextures;
		}

		static void Reset()
		{
			m_DebugTextures.clear();
		}

	private:
		static inline std::unordered_map<std::string, Ref<Texture>> m_DebugTextures;
	};
}
