#pragma once

#include <Kaidel/Assets/AssetManager.h>
#include <yaml-cpp/yaml.h>
namespace Kaidel {

	class MetaDataSerializer {
	public:

		MetaDataSerializer() = default;

		bool Serialize(Ref<_Asset> asset , const Path& path);
		Ref<_Asset> Deserialize(const Path& mtdPath, const Path& assetPath);


	private:
		Ref<_Asset> m_Asset;
	};
}
