#include "KDpch.h"
#include "TextureLibrary.h"
#include "Kaidel/Renderer/RendererGlobals.h"
#include "Kaidel/Renderer/GraphicsAPI/ShaderLibrary.h"
#include "Kaidel/Renderer/RenderCommand.h"
#include "Kaidel/Renderer/RendererDefinitions.h"
#include "Kaidel/Renderer/GraphicsAPI/DescriptorSet.h"

#include "stb_image.h"

//Fucking windows.h
#undef LoadImage
namespace Kaidel {
	using LoadFunc = void*(*)(const char*, int*, int*, int*, int);
	using LoadBufferFunc = void*(*)(const stbi_uc*, int, int*, int*, int*, int);
	using ConvertFunc = void*(*)(const float*, int, int, int);

	namespace Utils {

		struct ImageLoadResult {
			const void* Data;
			uint32_t Width, Height;
		};

		static LoadBufferFunc GetloadBufferFunc( Format format)
		{
			switch (format)
			{
			case Kaidel::Format::R8UN:
			case Kaidel::Format::RG8UN:
			case Kaidel::Format::R8N:
			case Kaidel::Format::RG8N:
			case Kaidel::Format::RGB8N:
			case Kaidel::Format::RGBA8SRGB:
			case Kaidel::Format::RGBA8UN:
			case Kaidel::Format::RGBA8N:
			case Kaidel::Format::RGB8UN:
			{
				return (LoadBufferFunc)&stbi_load_from_memory;
			}
			break;
			case Kaidel::Format::R16UN:
			case Kaidel::Format::RG16UN:
			case Kaidel::Format::RG16N:
			case Kaidel::Format::RGB16UN:
			case Kaidel::Format::RGB16N:
			case Kaidel::Format::RGBA16UN:
			case Kaidel::Format::RGBA16N:
			case Kaidel::Format::R16N:
			{
				return (LoadBufferFunc)&stbi_load_16_from_memory;
			}
			break;
			case Kaidel::Format::RGB32F:
			case Kaidel::Format::RGBA32F:
			{
				return (LoadBufferFunc)&stbi_loadf_from_memory;
			}
			break;
			}

			return (LoadBufferFunc)nullptr;
		}

		static LoadFunc GetLoadFunc(Format format)
		{
			switch (format)
			{
			case Kaidel::Format::R8UN:
			case Kaidel::Format::RG8UN:
			case Kaidel::Format::R8N:
			case Kaidel::Format::RG8N:
			case Kaidel::Format::RGB8N:
			case Kaidel::Format::RGBA8SRGB:
			case Kaidel::Format::RGBA8UN:
			case Kaidel::Format::RGBA8N:
			case Kaidel::Format::RGB8UN:
			{
				return (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::R16UN:
			case Kaidel::Format::RG16UN:
			case Kaidel::Format::RG16N:
			case Kaidel::Format::RGB16UN:
			case Kaidel::Format::RGB16N:
			case Kaidel::Format::RGBA16UN:
			case Kaidel::Format::RGBA16N:
			case Kaidel::Format::R16N:
			{
				return (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::RGB32F:
			case Kaidel::Format::RGBA32F:
			{
				return (LoadFunc)&stbi_loadf;
			}
			break;
			}

			return (LoadFunc)nullptr;
		}

		static ImageLoadResult LoadImage(const Path& path, Format format) {
			ImageLoadResult res{};
			const void* data;
			int width, height;
			std::string pathStr = path.string();

			LoadFunc func = GetLoadFunc(format);
			
			ConvertFunc convert = nullptr;

			uint32_t channelCount = Utils::CalculateChannelCount(format);
			KD_CORE_ASSERT(func);
			{
				int32_t count = 0;
				data = func(pathStr.c_str(), &width, &height, &count, channelCount);
			}
			KD_CORE_ASSERT(data);
			
			if (convert) {
				data = convert((const float*)data, width, height, channelCount);
				KD_CORE_ASSERT(data);
			}

			res.Data = data;
			res.Width = width;
			res.Height = height;
			return res;
		}
	}



	struct TextureLibraryData {
		std::unordered_map<Path, Ref<Texture2D>> LoadedTextures;
		std::unordered_map<Ref<Texture2D>, Path> LoadedTexturesInv;
		Ref<ComputePipeline> EquiToCubePipeline;
		Ref<DescriptorSet> EquiToCubeSet;
		
		Ref<ComputePipeline> IrradianceDiffusePipeline;
		Ref<DescriptorSet> IrradianceDiffuseSet;

		Ref<ComputePipeline> BRDFIntegrationPipeline;
		Ref<DescriptorSet> BRDFIntegrationSet;

		Ref<ComputePipeline> SpecularFilterPipeline;
	};

	static TextureLibraryData* s_LibraryData;

	void TextureLibrary::Init()
	{
		s_LibraryData = new TextureLibraryData;

		{
			DescriptorSetLayoutSpecification specs(
				{
					{DescriptorType::SamplerWithTexture, ShaderStage_ComputeShader},
					{DescriptorType::Image, ShaderStage_ComputeShader}
				}
			);
			s_LibraryData->EquiToCubeSet = DescriptorSet::Create(specs);
		}

		{
			s_LibraryData->EquiToCubePipeline = 
				ComputePipeline::Create(ShaderLibrary::LoadOrGetNamedShader("EquiToCube", "assets/_shaders/EquiRectToCubemap.glsl"));
		}

		{
			DescriptorSetLayoutSpecification specs(
				{
					{DescriptorType::SamplerWithTexture, ShaderStage_ComputeShader},
					{DescriptorType::Image, ShaderStage_ComputeShader}
				}
			);

			s_LibraryData->IrradianceDiffuseSet = DescriptorSet::Create(specs);
		}

		{
			s_LibraryData->IrradianceDiffusePipeline = 
				ComputePipeline::Create(ShaderLibrary::LoadOrGetNamedShader(
					"IrradianceDiffuseCalc", "assets/_shaders/IrradianceDiffuseCalc.glsl"));
		}

		{
			DescriptorSetLayoutSpecification specs(
				{
					{DescriptorType::Image, ShaderStage_ComputeShader}
				}
			);

			s_LibraryData->BRDFIntegrationSet = DescriptorSet::Create(specs);
		}

		{
			s_LibraryData->BRDFIntegrationPipeline =
				ComputePipeline::Create(ShaderLibrary::LoadOrGetNamedShader(
					"BRDFIntegrationLUT", "assets/_shaders/BRDFIntegrationLUT.glsl"));
		}

		{
			s_LibraryData->SpecularFilterPipeline =
				ComputePipeline::Create(ShaderLibrary::LoadOrGetNamedShader(
					"SpecularFilter", "assets/_shaders/SpecularFilter.glsl"));
		}
	}
	void TextureLibrary::Shutdown()
	{
		delete s_LibraryData;
	}
	Ref<Texture2D> TextureLibrary::Load(const Path& path, ImageLayout layout, Format format)
	{
		SCOPED_TIMER("Load");
		if (auto t = Get(path); t)
			return t;

		KD_INFO("Loading texture {}", path);

		Utils::ImageLoadResult res;
		
		res = Utils::LoadImage(path, format);
		
		Texture2DSpecification specs;
		specs.Format = format;
		specs.Width = res.Width;
		specs.Height = res.Height;
		specs.Depth = 1;
		specs.Layers = 1;
		specs.Mips = 1;
		specs.Samples = TextureSamples::x1;
		TextureData data;
		data.Data = res.Data;
		data.Layer = 0;
		specs.InitialDatas.push_back(data);
		specs.Layout = layout;
		specs.Swizzles[0] = TextureSwizzle::Red;
		specs.Swizzles[1] = TextureSwizzle::Green;
		specs.Swizzles[2] = TextureSwizzle::Blue;
		specs.Swizzles[3] = TextureSwizzle::Alpha;

		Ref<Texture2D> t = Texture2D::Create(specs);
		stbi_image_free((void*)res.Data);

		s_LibraryData->LoadedTextures[path] = t;
		s_LibraryData->LoadedTexturesInv[t] = path;
		return t;
	}

	Ref<TextureCube> TextureLibrary::LoadCube(const TextureCubePathParameters& pathParams, ImageLayout layout, Format format)
	{
		SCOPED_TIMER("Load");

		const Path* paths = (const Path*)&pathParams;

		std::unordered_map<uint32_t, Utils::ImageLoadResult> results;
		
		for (uint32_t i = 0; i < 6; ++i)
		{
			if(paths[i].empty())
				continue;
			KD_INFO("Loading texture {}", paths[i]);

			results[i] = Utils::LoadImage(paths[i], format);
		}

		KD_CORE_ASSERT(!results.empty());

		uint32_t width = 0, height = 0;

		for (auto& [i, res] :results)
		{
			width = std::max(width, res.Width);
			height = std::max(height, res.Height);
		}

		TextureCubeSpecification specs;
		specs.Format = format;
		specs.Width = width;
		specs.Height = height;
		specs.Depth = 1;
		specs.Layers = 1;
		specs.Mips = 1;
		specs.Samples = TextureSamples::x1;
		specs.Layout = layout;
		specs.Swizzles[0] = TextureSwizzle::Red;
		specs.Swizzles[1] = TextureSwizzle::Green;
		specs.Swizzles[2] = TextureSwizzle::Blue;
		specs.Swizzles[3] = TextureSwizzle::Alpha;

		for (auto& [i, res] : results)
		{
			TextureData data;
			data.Data = res.Data;
			data.Layer = i;
			specs.InitialDatas.push_back(data);
		}

		Ref<TextureCube> t = TextureCube::Create(specs);

		for (auto& [i, res] : results)
		{
			stbi_image_free((void*)res.Data);
		}

		return t;
	}

	Ref<EnvironmentMap> TextureLibrary::LoadEquiRectHDR(const Path& path, uint32_t width, uint32_t height)
	{
		KD_INFO("Load HDR Texture at {}", path.string());

		auto res = Utils::LoadImage(path, Format::RGBA32F);

		Texture2DSpecification specs;
		specs.Format = Format::RGBA32F;
		specs.Width = res.Width;
		specs.Height = res.Height;
		specs.Depth = 1;
		specs.Layers = 1;
		specs.Samples = TextureSamples::x1;
		TextureData data;
		data.Data = res.Data;
		data.Layer = 0;
		specs.InitialDatas.push_back(data);
		specs.Layout = ImageLayout::ShaderReadOnlyOptimal;
		specs.Swizzles[0] = TextureSwizzle::Red;
		specs.Swizzles[1] = TextureSwizzle::Green;
		specs.Swizzles[2] = TextureSwizzle::Blue;
		specs.Swizzles[3] = TextureSwizzle::Alpha;

		static Ref<Texture2D> t = Texture2D::Create(specs);
		stbi_image_free((void*)res.Data);

		TextureCubeSpecification cubeSpecs;
		cubeSpecs.Format = Format::RGBA16F;
		cubeSpecs.Width = width;
		cubeSpecs.Height = height;
		cubeSpecs.Layout = ImageLayout::General;
		Ref<TextureCube> cube = TextureCube::Create(cubeSpecs);

		s_LibraryData->EquiToCubeSet->Update(
			t, RendererGlobals::GetSamler(SamplerFilter::Linear, SamplerMipMapMode::Linear), 
			ImageLayout::ShaderReadOnlyOptimal, 0)
			.Update(cube, {}, ImageLayout::General, 1);


		//uint32_t specularWidth = 1024, specularHeight = 1024;
		TextureCubeSpecification specularSpecs;
		specularSpecs.Width = width;
		specularSpecs.Height = height;
		specularSpecs.Format = Format::RGBA16F;
		specularSpecs.Layout = ImageLayout::General;
		Ref<Texture2D> specular = TextureCube::Create(specularSpecs);

		std::vector<Ref<DescriptorSet>> sets;
		std::vector<Ref<TextureReference>> references;

		uint32_t envMipCount = specular->GetTextureSpecification().Mips;

		for (uint32_t level = 1; level < envMipCount; ++level)
		{
			TextureReferenceSpecification specs;
			specs.Reference = specular;
			specs.LayerCount = 6;
			specs.StartMip = level;
			Ref<TextureReference> reference = TextureReference::Create(specs);

			references.push_back(reference);

			DescriptorSetLayoutSpecification layoutSpecs(
				{
					{DescriptorType::SamplerWithTexture, ShaderStage_ComputeShader},
					{DescriptorType::Image, ShaderStage_ComputeShader}
				}
			);


			SamplerState state;
			state.MaxLod = 0.0f;
			state.MinLod = 0.0f;
			state.MinFilter = state.MagFilter = SamplerFilter::Linear;
			state.MipFilter = SamplerMipMapMode::Nearest;

			static Ref<Sampler> sampler = Sampler::Create(state);


			Ref<DescriptorSet> set = DescriptorSet::Create(layoutSpecs);

			set->Update(
				cube, sampler,
				ImageLayout::ShaderReadOnlyOptimal, 0)
				.Update(reference, {}, ImageLayout::General, 1);
			sets.push_back(set);
		}


		uint32_t irrWidth = 32, irrHeight = 32;
		TextureCubeSpecification irSpecs;
		irSpecs.Format = Format::RGBA16F;
		irSpecs.Width = irrWidth;
		irSpecs.Height = irrHeight;
		irSpecs.Layout = ImageLayout::General;
		irSpecs.Mipped = false;
		Ref<TextureCube> irradiance = TextureCube::Create(irSpecs);
		
		s_LibraryData->IrradianceDiffuseSet->Update(
			specular, RendererGlobals::GetSamler(SamplerFilter::Linear, SamplerMipMapMode::Linear),
			ImageLayout::ShaderReadOnlyOptimal, 0)
			.Update(irradiance, {}, ImageLayout::General, 1);

		uint32_t lutWidth = 256, lutHeight = 256;
		Texture2DSpecification lutSpecs;
		lutSpecs.Width = lutWidth;
		lutSpecs.Height = lutHeight;
		lutSpecs.Format = Format::RG16F;
		lutSpecs.Layout = ImageLayout::General;
		lutSpecs.Mipped = false;
		Ref<Texture2D> specularLUT = Texture2D::Create(lutSpecs);

		s_LibraryData->BRDFIntegrationSet->Update(specularLUT, {}, ImageLayout::General, 0);

		

		RenderCommand::Submit([&]() {
			RenderCommand::BindComputePipeline(s_LibraryData->EquiToCubePipeline);
			RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("EquiToCube"), s_LibraryData->EquiToCubeSet, 0);
			RenderCommand::Dispatch(std::ceil(width / 16.0f), std::ceil(height / 16.0f), 6);

			

			{
				ImageMemoryBarrier barrier(cube, ImageLayout::TransferSrcOptimal, AccessFlags_ShaderRead, AccessFlags_TransferRead);
				barrier.Subresource.LayerCount = 6;

				RenderCommand::PipelineBarrier(PipelineStages_ComputeShader, PipelineStages_Transfer, {}, {}, { barrier });
			}

			for (uint32_t i = 0; i < 6; ++i)
			{
				RenderCommand::CopyTexture(
					cube, i, 0,
					specular, i, 0
				);
			}

			{
				ImageMemoryBarrier barrier(
					specular, ImageLayout::ShaderReadOnlyOptimal, AccessFlags_TransferWrite, AccessFlags_None);
				barrier.Subresource.LayerCount = 6;
				barrier.Subresource.MipCount = 1;
				barrier.Subresource.StarMip = 0;

				RenderCommand::PipelineBarrier(PipelineStages_Transfer, PipelineStages_BottomOfPipe, {}, {}, { barrier });
			}

			RenderCommand::GenerateMips(cube);

			

			{
				ImageMemoryBarrier barrier(
					cube, ImageLayout::ShaderReadOnlyOptimal, AccessFlags_TransferRead, AccessFlags_ShaderRead);
				barrier.Subresource.LayerCount = 6;

				RenderCommand::PipelineBarrier(PipelineStages_Transfer, PipelineStages_ComputeShader, {}, {}, { barrier });
			}

			RenderCommand::BindComputePipeline(s_LibraryData->BRDFIntegrationPipeline);
			RenderCommand::BindDescriptorSet(
				ShaderLibrary::GetNamedShader("BRDFIntegrationLUT"), s_LibraryData->BRDFIntegrationSet, 0);
			RenderCommand::Dispatch(std::ceil(lutWidth / 32.0f), std::ceil(lutHeight / 32.0f), 1);

			{
				ImageMemoryBarrier barrier(
					specularLUT, ImageLayout::ShaderReadOnlyOptimal, AccessFlags_ShaderWrite, AccessFlags_None);

				RenderCommand::PipelineBarrier(PipelineStages_ComputeShader, PipelineStages_BottomOfPipe, {}, {}, { barrier });
			}

			const float deltaRoughness = 1.0f / std::max(float(envMipCount - 1), 1.0f);
			RenderCommand::BindComputePipeline(s_LibraryData->SpecularFilterPipeline);
			uint32_t width = specular->GetTextureSpecification().Width / 2;
			uint32_t height = specular->GetTextureSpecification().Height / 2;
			for (uint32_t level = 1; level < envMipCount; ++level) {
				auto& set = sets[level - 1];
				auto& reference = references[level - 1];


				RenderCommand::BindDescriptorSet(ShaderLibrary::GetNamedShader("SpecularFilter"), set, 0);
				RenderCommand::BindPushConstants(ShaderLibrary::GetNamedShader("SpecularFilter"), 0, deltaRoughness * level);
				RenderCommand::Dispatch(std::ceil(width / 32.0f), std::ceil(height / 32.0f), 6);

				if(width >  1)
					width = width / 2;
				if(height > 1)
					height = height / 2;
			}

			{
				ImageMemoryBarrier barrier(
					specular, ImageLayout::ShaderReadOnlyOptimal, AccessFlags_ShaderWrite, AccessFlags_ShaderRead);
				barrier.Subresource.LayerCount = 6;
				barrier.Subresource.MipCount = envMipCount - 1;
				barrier.Subresource.StarMip = 1;
				barrier.OldLayout = ImageLayout::General;

				RenderCommand::PipelineBarrier(PipelineStages_ComputeShader, PipelineStages_ComputeShader, {}, {}, { barrier });
			}

			RenderCommand::BindComputePipeline(s_LibraryData->IrradianceDiffusePipeline);
			RenderCommand::BindDescriptorSet(
				ShaderLibrary::GetNamedShader("IrradianceDiffuseCalc"), s_LibraryData->IrradianceDiffuseSet, 0);
			RenderCommand::Dispatch(std::ceil(irrWidth / 32.0f), std::ceil(irrHeight / 32.0f), 6);

			{
				ImageMemoryBarrier barrier(
					irradiance, ImageLayout::ShaderReadOnlyOptimal,
					AccessFlags_ShaderWrite, AccessFlags_None);
				barrier.Subresource.LayerCount = 6;

				RenderCommand::PipelineBarrier(PipelineStages_ComputeShader, PipelineStages_BottomOfPipe, {}, {}, { barrier });
			}
			
		});

		EnvironmentMapSpecification environmentSpecs;
		environmentSpecs.Environment = cube;
		environmentSpecs.Irradiance = irradiance;
		environmentSpecs.SpecularLUT = specularLUT;
		environmentSpecs.Specular = specular;
		return CreateRef<EnvironmentMap>(environmentSpecs);
	}

	Ref<Texture2D> TextureLibrary::Get(const Path& path)
	{
		auto it = s_LibraryData->LoadedTextures.find(path);
		if (it != s_LibraryData->LoadedTextures.end())
			return it->second;
		return {};
	}

	Ref<Texture2D> TextureLibrary::LoadFromBuffer(const void* buffer, uint64_t size, ImageLayout layout, Format format)
	{
		LoadBufferFunc func = Utils::GetloadBufferFunc(format);

		int width, height, numChannels;

		const void* res = 
			func((const stbi_uc*)buffer, (int)size, &width, &height, &numChannels, Utils::CalculateChannelCount(format));

		Texture2DSpecification specs;
		specs.Format = format;
		specs.Width = width;
		specs.Height = height;
		specs.Depth = 1;
		specs.Layers = 1;
		specs.Mips = 1;
		specs.Samples = TextureSamples::x1;
		TextureData data;
		data.Data = res;
		data.Layer = 0;
		specs.InitialDatas.push_back(data);
		specs.Layout = layout;
		specs.Swizzles[0] = TextureSwizzle::Red;
		specs.Swizzles[1] = TextureSwizzle::Green;
		specs.Swizzles[2] = TextureSwizzle::Blue;
		specs.Swizzles[3] = TextureSwizzle::Alpha;

		Ref<Texture2D> t = Texture2D::Create(specs);
		stbi_image_free((void*)res);

		return t;
	}

	const Path& TextureLibrary::GetPath(Ref<Texture2D> texture)
	{
		static Path empty = {};
		auto it = s_LibraryData->LoadedTexturesInv.find(texture);
		if (it != s_LibraryData->LoadedTexturesInv.end())
			return it->second;
		KD_CORE_ASSERT(false);
		return empty;
	}
	
	bool TextureLibrary::Exists(const Path& path)
	{
		return s_LibraryData->LoadedTextures.find(path) != s_LibraryData->LoadedTextures.end();
	}
}
