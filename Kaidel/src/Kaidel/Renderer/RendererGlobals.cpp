#include "KDpch.h"
#include "RendererGlobals.h"
#include "Kaidel/Asset/AssetManager.h"

#include "GraphicsAPI/TextureLibrary.h"

namespace Kaidel {



	struct ColorTexture {
		glm::vec4 Color;
		Ref<Texture2D> Texture;
	};

	struct GlobalsData {
		
		Ref<EnvironmentMap> Environment;
		Ref<EnvironmentMap> SingleColorEnvironment;
		
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

		Ref<StandardMaterialInstance> DefaultMaterial;

		Ref<Texture2D> SpecularLUT;

		Ref<Font> DefaultFont;
	};

	static GlobalsData* s_Data = nullptr;;

	void RendererGlobals::Init()
	{
		s_Data = new GlobalsData;

		//s_Data->DefaultFont = CreateRef<Font>("assets/fonts/opensans/OpenSans-Regular.ttf");

		{
			uint32_t lutWidth = 256, lutHeight = 256;
			Texture2DSpecification lutSpecs;
			lutSpecs.Width = lutWidth;
			lutSpecs.Height = lutHeight;
			lutSpecs.Format = Format::RG16F;
			lutSpecs.Layout = ImageLayout::General;
			lutSpecs.Mipped = false;
			Ref<Texture2D> specularLUT = Texture2D::Create(lutSpecs);

			Ref<ComputePipeline> integPipeline;
			Ref<DescriptorSet> integSet;

			{
				DescriptorSetLayoutSpecification specs(
					{
						{DescriptorType::Image, ShaderStage_ComputeShader}
					}
				);

				integSet = DescriptorSet::Create(specs);
			}

			{
				integPipeline =
					ComputePipeline::Create(ShaderLibrary::LoadOrGetNamedShader(
						"BRDFIntegrationLUT", "assets/_shaders/BRDFIntegrationLUT.glsl"));
			}

			integSet->Update(specularLUT, {}, ImageLayout::General, 0);

			RenderCommand::Submit([&]() {
				RenderCommand::BeginMarker("BRDFIntegrationLUT");
				RenderCommand::BindComputePipeline(integPipeline);
				RenderCommand::BindDescriptorSet(
					ShaderLibrary::GetNamedShader("BRDFIntegrationLUT"), integSet, 0);
				RenderCommand::Dispatch(std::ceil(lutWidth / 32.0f), std::ceil(lutHeight / 32.0f), 1);

				{
					ImageMemoryBarrier barrier(
						specularLUT, ImageLayout::ShaderReadOnlyOptimal, AccessFlags_ShaderWrite, AccessFlags_None);

					RenderCommand::PipelineBarrier(PipelineStages_ComputeShader, PipelineStages_BottomOfPipe, {}, {}, { barrier });
				}
				RenderCommand::EndMarker();
			});

			s_Data->SpecularLUT = std::move(specularLUT);
		}

		{
			TextureCubeSpecification cubeSpecs;
			cubeSpecs.Width = 1;
			cubeSpecs.Height = 1;
			cubeSpecs.Layers = 1;
			cubeSpecs.Mipped = false;
			cubeSpecs.Layout = ImageLayout::ShaderReadOnlyOptimal;
			cubeSpecs.InitialDatas.reserve(6);

			glm::u8vec4 col = glm::u8vec4(127U, 127U, 127U, 255U);

			for (uint32_t i = 0; i < 6; ++i)
			{
				TextureData td;
				td.Layer = i;
				td.Data = &col;
				cubeSpecs.InitialDatas.emplace_back(td);
			}

			Ref<TextureCube> cube = TextureCube::Create(cubeSpecs);

			EnvironmentMapSpecification environmentSpecs;
			environmentSpecs.Environment = cube;
			environmentSpecs.Irradiance = cube;
			environmentSpecs.SpecularLUT = s_Data->SpecularLUT;
			environmentSpecs.Specular = cube;
			s_Data->SingleColorEnvironment = CreateRef<EnvironmentMap>(environmentSpecs);
		}


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

	void RendererGlobals::CreateDefaults()
	{
		{
			StandardMaterialInstance::Initializer matInitializer;

			s_Data->DefaultMaterial = CreateRef<StandardMaterialInstance>(matInitializer);

			AssetManager::RegisterBuiltIn(s_Data->DefaultMaterial, "Default material");
		}
	}

	void RendererGlobals::Shutdown()
	{
		delete s_Data;
	}
	Ref<Sampler> RendererGlobals::GetSampler(SamplerFilter filter, SamplerMipMapMode mipMode)
	{
		if (filter == SamplerFilter::Linear && mipMode == SamplerMipMapMode::Linear)
			return s_Data->LinearMippedRepeat;
		else if (filter == SamplerFilter::Linear && mipMode == SamplerMipMapMode::Nearest)
			return s_Data->LinearRepeat;
		else if (filter == SamplerFilter::Nearest && mipMode == SamplerMipMapMode::Nearest)
			return s_Data->NearestRepeat;
		else if (filter == SamplerFilter::Nearest && mipMode == SamplerMipMapMode::Linear)
			return s_Data->NearestMippedRepeat;
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

	Kaidel::Ref<Kaidel::EnvironmentMap> RendererGlobals::GetSingleColorEnvironmentMap()
	{
		return s_Data->SingleColorEnvironment;
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

	Kaidel::Ref<Kaidel::Texture2D> RendererGlobals::GetSpecularLUT()
	{
		return s_Data->SpecularLUT;
	}

	Kaidel::Ref<Kaidel::StandardMaterialInstance> RendererGlobals::GetDefaultMaterial()
	{
		return s_Data->DefaultMaterial;
	}

}
