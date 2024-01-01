#pragma once
#include "UniformBuffer.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "Kaidel/Scene/Scene.h"

#include <glm/glm.hpp>
namespace Kaidel {

	struct CameraBufferData {
		glm::mat4 CameraViewProjection;
		glm::vec3 CameraPosition;
	};

	struct SharedPassDataConfig {
		Ref<Scene> Scene;
		CameraBufferData CameraData;
	};

	struct SharedPassData {
		SharedPassDataConfig Config;
		void SetData();
	};

}
