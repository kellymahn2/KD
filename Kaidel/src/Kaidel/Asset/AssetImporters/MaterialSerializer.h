#pragma once

#include "Kaidel/Scene/Material.h"
#include "MetadataFile.h"

namespace Kaidel {
	class MaterialSerializer {
	public:
		void Serialize(const Path& path, Ref<StandardMaterial> mat, MetadataFile* mtdFile);
		void Deserialize(Ref<StandardMaterial> mat, const Path& path, MetadataFile* mtdFile);

		void Serialize(const Path& path, Ref<ShaderMaterial> mat, MetadataFile* mtdFile);
		void Deserialize(Ref<ShaderMaterial> mat, const Path& path, MetadataFile* mtdFile);

		void Serialize(const Path& path, Ref<VisualShader> mat, MetadataFile* mtdFile);
		void Deserialize(Ref<VisualShader> mat, const Path& path, MetadataFile* mtdFile);
	};
}
