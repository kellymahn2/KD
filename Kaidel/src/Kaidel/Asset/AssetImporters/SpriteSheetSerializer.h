#pragma once

#include "MetadataFile.h"
#include "Kaidel/Renderer/Sprite.h"

namespace Kaidel
{
	class SpriteSheetSerializer {
	public:
		void Serialize(Ref<SpriteSheet> sheet, MetadataFile* mtdFile);
		void Deserialize(Ref<SpriteSheet> sheet, Ref<Texture2D> texture, MetadataFile* mtdFile);

	};
}
