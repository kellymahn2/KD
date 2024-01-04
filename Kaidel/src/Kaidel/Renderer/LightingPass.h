#pragma once
#include "Kaidel/Scene/Scene.h"
#include "SharedPassData.h"
namespace Kaidel {


	struct LightingPassConfig {
		Ref<Scene> Scene;
		uint32_t Width;
		uint32_t Height;
	};

	struct LightingPass {

		LightingPassConfig Config;
		LightingPass() = default;
		void Render();
		static uint64_t GetColorAttachment();
	};

}
