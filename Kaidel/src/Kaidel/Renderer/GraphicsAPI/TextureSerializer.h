#pragma once

#include "Texture.h"

namespace Kaidel {
	class TextureSerializer {
	public:
		TextureSerializer(Ref<Texture2D> context) { m_Context = context; }

		bool Serialize(const FileSystem::path& path);
		bool Deserialize(const FileSystem::path& path);

	private:
		Ref<Texture2D> m_Context;
	};
}
