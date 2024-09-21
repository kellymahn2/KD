#pragma once
#include "Kaidel/Core/Base.h"

#include "Texture.h"

namespace Kaidel {


	class TextureLibrary {
	public:
		static void Init();
		static void Shutdown();

		static Ref<Texture2D> Load(const Path& path, ImageLayout layout, Format format);
		static Ref<Texture2D> Get(const Path& path);
		static bool Exists(const Path& path);
	};

}
