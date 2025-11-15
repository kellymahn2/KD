#pragma once

#include "GraphicsAPI/Texture.h"
#include "GraphicsAPI/Sampler.h"
#include "Text/Font.h"
#include <glm/glm.hpp>

namespace Kaidel {

	class EnvironmentMap;

	class RendererGlobals {
	public:
		static void Init();
		static void Shutdown();

		static Ref<Sampler> GetSamler(SamplerFilter filter, SamplerMipMapMode mipMode);
		static Ref<DescriptorSet> GetSamplerSet();
		
		static Ref<Font> GetDefaultFont();

		static Ref<EnvironmentMap> GetEnvironmentMap();
		
		static Ref<EnvironmentMap> LoadEnvironmentMap(const Path& path);

		static Ref<Texture2D> GetSingleColorTexture(const glm::vec4& color);
	};
}
