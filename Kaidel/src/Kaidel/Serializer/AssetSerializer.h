#pragma once
#include "Kaidel/Assets/AssetManager.h"
#include <yaml-cpp/yaml.h>

namespace Kaidel {

	class AssetSerializer {
	public:
		AssetSerializer(Ref<_Asset> asset)
			:m_Asset(std::move(asset))
		{
			KD_CORE_ASSERT(m_Asset);
		}

		bool Serialize(const Path& path);
		bool Deserialize(const Path& path);

		bool SerializeTo(YAML::Emitter& out);
		bool DeserializeFrom(YAML::Node node);



	private:
		Ref<_Asset> m_Asset;
	};

}
