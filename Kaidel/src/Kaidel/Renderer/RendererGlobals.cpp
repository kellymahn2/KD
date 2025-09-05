#include "KDpch.h"
#include "RendererGlobals.h"

#include "GraphicsAPI/TextureLibrary.h"

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
		
		Ref<EnvironmentMap> Environment;
		
		std::vector<ColorTexture> Textures;

		Ref<Sampler> NearestClamp;
		Ref<Sampler> LinearClamp;
		Ref<Sampler> NearestMippedClamp;
		Ref<Sampler> LinearMippedClamp;
		Ref<Sampler> NearestRepeat;
		Ref<Sampler> LinearRepeat;
		Ref<Sampler> NearestMippedRepeat;
		Ref<Sampler> LinearMippedRepeat;

		Ref<DescriptorSet> SamplerSet;

		Ref<Font> DefaultFont;
	};

	static GlobalsData* s_Data = nullptr;;

	void RendererGlobals::Init()
	{
		s_Data = new GlobalsData;

		{
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

		s_Data->DefaultFont = CreateRef<Font>("assets/fonts/opensans/OpenSans-Regular.ttf");


		Ref<Sampler> NearestCLAMP;
		Ref<Sampler> LinearCLAMP;
		Ref<Sampler> NearestMippedCLAMP;
		Ref<Sampler> LinearMippedCLAMP;
		Ref<Sampler> NearestRepeat;
		Ref<Sampler> LinearRepeat;
		Ref<Sampler> NearestMippedRepeat;
		Ref<Sampler> LinearMippedRepeat;

		SamplerState state;
		
		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::ClampToEdge;
		state.MinFilter = state.MagFilter = SamplerFilter::Nearest;
		state.MipFilter = SamplerMipMapMode::Nearest;
		s_Data->NearestClamp = Sampler::Create(state);

		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::ClampToEdge;
		state.MinFilter = state.MagFilter = SamplerFilter::Linear;
		state.MipFilter = SamplerMipMapMode::Nearest;
		s_Data->LinearClamp = Sampler::Create(state);

		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::ClampToEdge;
		state.MinFilter = state.MagFilter = SamplerFilter::Nearest;
		state.MipFilter = SamplerMipMapMode::Linear;
		s_Data->NearestMippedClamp = Sampler::Create(state);

		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::ClampToEdge;
		state.MinFilter = state.MagFilter = SamplerFilter::Linear;
		state.MipFilter = SamplerMipMapMode::Linear;
		s_Data->LinearMippedClamp = Sampler::Create(state);

		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::Repeat;
		state.MinFilter = state.MagFilter = SamplerFilter::Nearest;
		state.MipFilter = SamplerMipMapMode::Nearest;
		s_Data->NearestRepeat = Sampler::Create(state);

		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::Repeat;
		state.MinFilter = state.MagFilter = SamplerFilter::Linear;
		state.MipFilter = SamplerMipMapMode::Nearest;
		s_Data->LinearRepeat = Sampler::Create(state);

		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::Repeat;
		state.MinFilter = state.MagFilter = SamplerFilter::Nearest;
		state.MipFilter = SamplerMipMapMode::Linear;
		s_Data->NearestMippedRepeat = Sampler::Create(state);

		state.AddressModeU = state.AddressModeV = state.AddressModeW = SamplerAddressMode::Repeat;
		state.MinFilter = state.MagFilter = SamplerFilter::Linear;
		state.MipFilter = SamplerMipMapMode::Linear;
		s_Data->LinearMippedRepeat = Sampler::Create(state);

		DescriptorSetLayoutSpecification samplerSetSpecs(
			{
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
				{DescriptorType::Sampler, ShaderStage_VertexShader | ShaderStage_FragmentShader},
			}
		);

		s_Data->SamplerSet = DescriptorSet::Create(samplerSetSpecs);

		s_Data->SamplerSet->
			Update({}, s_Data->NearestClamp, {}, 0).
			Update({}, s_Data->LinearClamp, {}, 1).
			Update({}, s_Data->NearestMippedClamp, {}, 2).
			Update({}, s_Data->LinearMippedClamp, {}, 3).
			Update({}, s_Data->NearestRepeat, {}, 4).
			Update({}, s_Data->LinearRepeat, {}, 5).
			Update({}, s_Data->NearestMippedRepeat, {}, 6).
			Update({}, s_Data->LinearMippedRepeat, {}, 7);
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

	Ref<DescriptorSet> RendererGlobals::GetSamplerSet()
	{
		return s_Data->SamplerSet;
	}

	Ref<Font> RendererGlobals::GetDefaultFont()
	{
		return s_Data->DefaultFont;
	}

	Ref<EnvironmentMap> RendererGlobals::GetEnvironmentMap()
	{
		return s_Data->Environment;
	}

	Ref<EnvironmentMap> RendererGlobals::LoadEnvironmentMap(const Path& path)
	{
		Ref<EnvironmentMap> environment = TextureLibrary::LoadEquiRectHDR(path, 1024, 1024);
		if (environment)
			return s_Data->Environment = environment;

		return {};
	}

	Ref<Texture2D> RendererGlobals::GetSingleColorTexture(const glm::vec4& color)
	{
		for (auto& texture : s_Data->Textures) {
			if (texture.Color == color)
				return texture.Texture;
		}

		uint32_t r = (uint32_t)(color.r * 255.0f);
		uint32_t g = (uint32_t)(color.g * 255.0f);
		uint32_t b = (uint32_t)(color.b * 255.0f);
		uint32_t a = (uint32_t)(color.a * 255.0f);

		r = std::clamp(r, 0U, 255U);
		g = std::clamp(g, 0U, 255U);
		b = std::clamp(b, 0U, 255U);
		a = std::clamp(a, 0U, 255U);

		uint8_t bytes[4] = { (uint8_t)r,(uint8_t)g,(uint8_t)b,(uint8_t)a };

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
