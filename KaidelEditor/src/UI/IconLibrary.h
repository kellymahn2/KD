#pragma once

#include "Kaidel/Renderer/GraphicsAPI/TextureLibrary.h"
#include "EditorIcon.h"

namespace Kaidel
{
	class IconLibrary {
	public:

		static void Init(const Path& path);

		static void Shutdown();

		static uint32_t GetIcon(const std::string& name);

		static Ref<TextureAtlas> GetAtlas();
		static Ref<DescriptorSet> GetDescriptorSet();
	};
}

