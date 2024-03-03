#pragma once
#include "Material.h"
namespace YAML {
	class Emitter;
}
namespace Kaidel {
	class MaterialSerializer {
	public:
		MaterialSerializer() = default;
		MaterialSerializer(Ref<Material> material)
			:m_Material(material)
		{
		}

		bool Serialize(const FileSystem::path& path);
		bool Deserialize(const FileSystem::path& path);

		void Serialize(YAML::Emitter& emitter);

	private:
		Ref<Material> m_Material;
	};
}
