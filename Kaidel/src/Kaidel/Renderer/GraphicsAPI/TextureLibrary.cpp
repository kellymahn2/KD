#include "KDpch.h"
#include "TextureLibrary.h"
#include "stb_image.h"

#include "Kaidel/Renderer/RendererDefinitions.h"
//Fucking windows.h
#undef LoadImage
namespace Kaidel {

	namespace Utils {

		static void* ConvertToI32(const float* data, int width, int height, int channelCount) {
			int32_t* res = new int32_t[width * height * channelCount];

			for (uint32_t i = 0; i < height; ++i) {
				for (uint32_t j = 0; j < width; ++j) {
					for (uint32_t k = 0; k < channelCount; ++k) {
						uint64_t offset = ((uint64_t)i * width + j) * channelCount + k;
						float val = data[offset];
						res[offset] = (int32_t)val;
					}
				}
			}

			return res;
		}

		static void* ConvertToU32(const float* data, int width, int height, int channelCount) {
			uint32_t* res = new uint32_t[width * height * channelCount];

			for (uint32_t i = 0; i < height; ++i) {
				for (uint32_t j = 0; j < width; ++j) {
					for (uint32_t k = 0; k < channelCount; ++k) {
						uint64_t offset = ((uint64_t)i * width + j) * channelCount + k;
						float val = data[offset];
						res[offset] = (int32_t)val;
					}
				}
			}

			return res;
		}


		struct ImageLoadResult {
			const void* Data;
			uint32_t Width, Height;
		};

		static ImageLoadResult LoadImage(const Path& path, Format format) {
			ImageLoadResult res{};
			const void* data;
			int width, height;
			std::string pathStr = path.string();

			using LoadFunc = void* (*)(const char*, int*, int*, int*,int);
			LoadFunc func = nullptr;
			
			using ConvertFunc = void* (*)(const float*, int, int, int);
			ConvertFunc convert = nullptr;

			switch (format)
			{
			case Kaidel::Format::R8UN:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::R8N:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::R8UI:
			{
			}
			break;
			case Kaidel::Format::R8I:
			{
			}
			break;
			case Kaidel::Format::RG8UN:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::RG8N:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::RG8UI:
			{
			}
			break;
			case Kaidel::Format::RG8I:
			{
			}
			break;
			case Kaidel::Format::RGB8UN:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::RGB8N:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::RGB8UI:
			{
			}
			break;
			case Kaidel::Format::RGB8I:
			{
			}
			break;
			case Kaidel::Format::RGBA8UN:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::RGBA8N:
			{
				func = (LoadFunc)&stbi_load;
			}
			break;
			case Kaidel::Format::RGBA8UI:
			{
			}
			break;
			case Kaidel::Format::RGBA8I:
			{
			}
			break;
			case Kaidel::Format::R16UN:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::R16N:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::R16UI:
			{
			}
			break;
			case Kaidel::Format::R16I:
			{
			}
			break;
			case Kaidel::Format::R16F:
			{
			}
			break;
			case Kaidel::Format::RG16UN:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::RG16N:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::RG16UI:
			{
			}
			break;
			case Kaidel::Format::RG16I:
			{
			}
			break;
			case Kaidel::Format::RG16F:
			{
			}
			break;
			case Kaidel::Format::RGB16UN:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::RGB16N:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::RGB16UI:
			{
			}
			break;
			case Kaidel::Format::RGB16I:
			{
			}
			break;
			case Kaidel::Format::RGB16F:
			{
			}
			break;
			case Kaidel::Format::RGBA16UN:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::RGBA16N:
			{
				func = (LoadFunc)&stbi_load_16;
			}
			break;
			case Kaidel::Format::RGBA16UI:
			{
			}
			break;
			case Kaidel::Format::RGBA16I:
			{
			}
			break;
			case Kaidel::Format::RGBA16F:
			{
			}
			break;
			case Kaidel::Format::R32UI:
			{
			}
			break;
			case Kaidel::Format::R32I:
			{
			}
			break;
			case Kaidel::Format::R32F:
			{
			}
			break;
			case Kaidel::Format::RG32UI:
			{
			}
			break;
			case Kaidel::Format::RG32I:
			{
			}
			break;
			case Kaidel::Format::RG32F:
			{
			}
			break;
			case Kaidel::Format::RGB32UI:
			{
			}
			break;
			case Kaidel::Format::RGB32I:
			{
			}
			break;
			case Kaidel::Format::RGB32F:
			{
				func = (LoadFunc)stbi_loadf;
			}
			break;
			case Kaidel::Format::RGBA32UI:
			{
			}
			break;
			case Kaidel::Format::RGBA32I:
			{
			}
			break;
			case Kaidel::Format::RGBA32F:
			{
				func = (LoadFunc)stbi_loadf;
			}
			break;
			case Kaidel::Format::Depth24Stencil8:
			{
			}
			break;
			case Kaidel::Format::Depth32F:
			{
			}
			break;
			}

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
	};

	static TextureLibraryData* s_LibraryData;

	void TextureLibrary::Init()
	{
		s_LibraryData = new TextureLibraryData;
	}
	void TextureLibrary::Shutdown()
	{
		delete s_LibraryData;
	}
	Ref<Texture2D> TextureLibrary::Load(const Path& path, ImageLayout layout, Format format)
	{
		if (auto t = Get(path); t)
			return t;
		Utils::ImageLoadResult res = Utils::LoadImage(path,format);

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

		s_LibraryData->LoadedTextures[path] = t;
		return t;
	}
	Ref<Texture2D> TextureLibrary::Get(const Path& path)
	{
		auto it = s_LibraryData->LoadedTextures.find(path);
		if (it != s_LibraryData->LoadedTextures.end())
			return it->second;
		return {};
	}
	
	bool TextureLibrary::Exists(const Path& path)
	{
		return s_LibraryData->LoadedTextures.find(path) != s_LibraryData->LoadedTextures.end();
	}
}
