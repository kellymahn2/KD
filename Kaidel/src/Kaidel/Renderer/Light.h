#pragma once

#include "Kaidel/Core/Base.h"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

namespace Kaidel {
	struct DirectionalLight {
		glm::vec4 Color;

		//A value between 0 and 1
		float SplitDistances[4];
	};
}
