#include "KDpch.h"
#include "RendererGlobals.h"

namespace Kaidel {



	struct ColorTexture {
		glm::vec4 Color;
		Ref<Texture2D> Texture;
	};

	struct GlobalsData {
		Ref<Sampler> LinearMipLinear;
		Ref<Sampler> LinearMipNearest;
		Ref<Sampler> NearestMipNearest;
		Ref<Sampler> NearestMipLinear;
		std::vector<ColorTexture> Textures;
	};

	static GlobalsData* s_Data = nullptr;;

	void RendererGlobals::Init()
	{
		s_Data = new GlobalsData;

		SamplerState state;
		state.AddressModeU = SamplerAddressMode::Repeat;
		state.AddressModeV = SamplerAddressMode::Repeat;
		state.AddressModeW = SamplerAddressMode::Repeat;

		state.MagFilter = SamplerFilter::Linear;
		state.MinFilter = SamplerFilter::Linear;
		state.MipFilter = SamplerMipMapMode::Linear;
		s_Data->LinearMipLinear = Sampler::Create(state);

		state.MipFilter = SamplerMipMapMode::Nearest;
		s_Data->LinearMipNearest = Sampler::Create(state);

		state.MagFilter = SamplerFilter::Nearest;
		state.MinFilter = SamplerFilter::Nearest;
		s_Data->NearestMipNearest = Sampler::Create(state);

		state.MipFilter = SamplerMipMapMode::Linear;
		s_Data->NearestMipLinear = Sampler::Create(state);

	}
	void RendererGlobals::Shutdown()
	{
		delete s_Data;
	}
	Ref<Sampler> RendererGlobals::GetSamler(SamplerFilter filter, SamplerMipMapMode mipMode)
	{
		if (filter == SamplerFilter::Linear && mipMode == SamplerMipMapMode::Linear)
			return s_Data->LinearMipLinear;
		else if (filter == SamplerFilter::Linear && mipMode == SamplerMipMapMode::Nearest)
			return s_Data->LinearMipNearest;
		else if (filter == SamplerFilter::Nearest && mipMode == SamplerMipMapMode::Nearest)
			return s_Data->NearestMipNearest;
		else if (filter == SamplerFilter::Nearest && mipMode == SamplerMipMapMode::Linear)
			return s_Data->NearestMipLinear;
		return {};
	}
	Ref<Texture2D> RendererGlobals::GetSingleColorTexture(const glm::vec4& color)
	{
		for (auto& texture : s_Data->Textures) {
			if (texture.Color == color)
				return texture.Texture;
		}

		uint32_t r = color.r * 255.0f;
		uint32_t g = color.g * 255.0f;
		uint32_t b = color.b * 255.0f;
		uint32_t a = color.a * 255.0f;

		r = std::clamp(r, 0U, 255U);
		g = std::clamp(g, 0U, 255U);
		b = std::clamp(b, 0U, 255U);
		a = std::clamp(a, 0U, 255U);

		uint8_t bytes[4] = { r,g,b,a };

		Texture2DSpecification specs;
		specs.Width = 1;
		specs.Height = 1;
		specs.Depth = 1;
		specs.Format = Format::RGBA8UN;
		specs.Layers = 1;
		specs.Mips = 1;
		specs.Samples = TextureSamples::x1;
		specs.Layout = ImageLayout::ShaderReadOnlyOptimal;

		TextureData d;
		d.Layer = 0;
		d.Data = bytes;
		specs.InitialDatas.push_back(d);

		ColorTexture t;
		t.Color = color;
		t.Texture = Texture2D::Create(specs);
		
		s_Data->Textures.push_back(t);

		return t.Texture;
	}
}
