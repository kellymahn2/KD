#pragma once
#include "Font.h"

namespace Kaidel {
	class FontLibrary {
	public:
		static void Init();
		static void Shutdown();

		static void LoadInto(Ref<Font> font, const Path& path);

		static Ref<Font> Load(const Path& path);
		static Ref<Font> Get(const Path& path);

		static bool Exists(const Path& path);
	};
}
