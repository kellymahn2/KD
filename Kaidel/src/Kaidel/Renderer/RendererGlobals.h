#pragma once

#include "GraphicsAPI/Texture.h"
#include "GraphicsAPI/Sampler.h"
#include <glm/glm.hpp>

namespace Kaidel {
	class RendererGlobals {
	public:
		static void Init();
		static void Shutdown();

		static Ref<Sampler> GetSamler(SamplerFilter filter, SamplerMipMapMode mipMode);

		static Ref<Texture2D> GetSingleColorTexture(const glm::vec4& color);
	};
}
