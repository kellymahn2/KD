#pragma once


namespace Kaidel {

	namespace AssetExtensions {
		constexpr static const char* MaterialExtension = ".mat";
	}

	enum class AssetType {
		None = 0,
		Material,
		Material2D,
		Texture2D,
		MaterialTexture,
	};
}
