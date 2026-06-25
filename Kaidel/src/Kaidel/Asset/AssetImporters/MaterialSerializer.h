#pragma once

#include "Kaidel/Scene/Material.h"

namespace Kaidel {
	class MaterialSerializer {
	public:
		void Serialize(const Path& path, Ref<StandardMaterialInstance> mat);
		void Deserialize(Ref<StandardMaterialInstance> mat, const Path& path);
	};
}
