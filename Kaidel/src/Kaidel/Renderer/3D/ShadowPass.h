#pragma once
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Renderer/GraphicsAPI/FullAPI.h"

namespace Kaidel {

	struct ShadowPassInput {
		Ref<Texture2DArray> SpotLightDepthMaps;
		Ref<Shader> SpotlightShader;

		ShadowPassInput(Ref<Texture2DArray> spotlightDepthMaps)
			:SpotLightDepthMaps(std::move(spotlightDepthMaps))
		{}
	};


	class ShadowPass {
	public:

		void Render(Ref<Mesh> mesh);
	private:
		void FlushMesh(Ref<Mesh> mesh);
	};

}
