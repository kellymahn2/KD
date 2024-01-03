#pragma once
#include "Kaidel/Scene/Scene.h"
namespace Kaidel {
	struct ShadowPassConfig {
		Ref<Scene> Scene;
	};

	struct ShadowPass {

		ShadowPassConfig Config;
		ShadowPass() = default;
		void Render();
	};
}
