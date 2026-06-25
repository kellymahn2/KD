#pragma once

#include "Kaidel/Asset/AssetManager.h"
#include "Kaidel/Renderer/GraphicsAPI/Texture.h"

namespace Kaidel {

	class AssetPreview {
	public:
		static Ref<Texture2D> GetPreview(Asset asset);
	private:
		static bool TryGetPreview(AssetHandle handle, Ref<Texture2D>& texture);
	};

}
