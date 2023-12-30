#pragma once
#include <glm/glm.hpp>
namespace Kaidel {
	struct CubeVertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		int MaterialIndex;
		int EntityID;
	};

}
