#pragma once

#include "GraphicsAPI/Texture.h"
#include "GraphicsAPI/Sampler.h"
#include "Text/Font.h"
#include "Kaidel/Scene/Material.h"
#include <glm/glm.hpp>

#define GLOBAL_SAMPLER_LINEAR_MIPPED RendererGlobals::GetSampler(SamplerFilter::Linear, SamplerMipMapMode::Linear)
#define GLOBAL_SAMPLER_LINEAR RendererGlobals::GetSampler(SamplerFilter::Linear, SamplerMipMapMode::Nearest)
#define GLOBAL_SAMPLER_NEAREST_MIPPED RendererGlobals::GetSampler(SamplerFilter::Nearest, SamplerMipMapMode::Linear)
#define GLOBAL_SAMPLER_NEAREST RendererGlobals::GetSampler(SamplerFilter::Nearest, SamplerMipMapMode::Nearest)


namespace Kaidel {

	class EnvironmentMap;

	class RendererGlobals {
	public:
		static void Init();
		static void CreateDefaults();
		static void Shutdown();

		static Ref<Sampler> GetSampler(SamplerFilter filter, SamplerMipMapMode mipMode, SamplerAddressMode addressMode = SamplerAddressMode::Repeat);
		static Ref<DescriptorSet> GetSamplerSet();
		
		static Ref<Font> GetDefaultFont();

		static Ref<EnvironmentMap> GetEnvironmentMap();

		static Ref<EnvironmentMap> GetSingleColorEnvironmentMap();
		
		static Ref<EnvironmentMap> LoadEnvironmentMap(const Path& path);

		static Ref<SpriteSheet> GetSingleColorTexture(const glm::vec4& color);

		static Ref<Texture2D> GetSpecularLUT();

		static Ref<StandardMaterial> GetDefaultMaterial();
	};
}
