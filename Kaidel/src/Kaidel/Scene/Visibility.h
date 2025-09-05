#pragma once

#include "Kaidel/Core/Base.h"
#include "Kaidel/Math/FrustumCulling.h"
#include "Kaidel/Renderer/Renderer3D.h"

#include <EnTT.hpp>

namespace Kaidel {
	struct InstanceData
	{
		glm::mat4 Transform;
	};

	struct ShadowVisibilityCascade {
		FrustumCuller Culler;
		std::vector<InstanceData> Instances;
		std::vector<entt::entity> VisibleEntities;
		std::atomic<uint32_t> Count;
	};

	struct ShadowVisibility {
		ShadowVisibilityCascade Cascades[4];
	};

	struct Visibility {
		FrustumCuller Culler;
		
		std::vector<InstanceData> Instances;
		std::vector<entt::entity> VisibleEntities;
		std::atomic<uint32_t> Count;

		ShadowVisibility Directional;
	};
}
