#include "KDpch.h"
#include "TextureLibrary.h"


#include <stb_image.h>

namespace Kaidel {

	struct TextureLibraryData {
		std::unordered_map<Path, ImageSubresource> Textures;
		std::unordered_map<std::string, ImageSubresource> NamedTextures;
		std::unordered_map<uint32_t, ImageSubresource> LayerToTexture;
		uint32_t Width, Height;
		Ref<TextureLayered2D> Texture;
	};

	static TextureLibraryData* s_LibraryData = nullptr;

	void TextureLibrary::Init(uint32_t width,uint32_t height,Format format)
	{
		s_LibraryData = new TextureLibraryData;
		s_LibraryData->Width = width;
		s_LibraryData->Height = height;
		{
			TextureLayered2DSpecification specs{};
			specs.InitialWidth = width;
			specs.InitialHeight = height;
			specs.Usage = TextureUsage_Updateable | TextureUsage_Sampled | TextureUsage_CopyFrom;
			specs.TextureFormat = format;
			s_LibraryData->Texture = TextureLayered2D::Create(specs);
		}
	}
	void TextureLibrary::PushDefaultTextures()
	{
		TextureLayered2DLayerSpecification spec{};
		spec.Width = s_LibraryData->Width;
		spec.Height = s_LibraryData->Height;
		
		uint64_t size = (uint64_t)s_LibraryData->Width * s_LibraryData->Height;
		
		glm::u8vec4* buffer = new glm::u8vec4[size];

		for (uint64_t i = 0; i < size; ++i) {
			buffer[i] = { 255,255,255,255 };
		}

		spec.InitialData = buffer;

		s_LibraryData->Texture->Push(spec);
		s_LibraryData->LayerToTexture[0] = CalculateSubresource(0,spec.Width,spec.Height);

		delete[] buffer;
	}
	void TextureLibrary::Shutdown()
	{
		delete s_LibraryData;
	}
	bool TextureLibrary::PathedTextureLoaded(const Path& path)
	{
		return s_LibraryData->Textures.find(path) != s_LibraryData->Textures.end();
	}
	ImageSubresource TextureLibrary::LoadTexture(const Path& path)
	{
		ParsedFileContents contents = ReadPixelsFromFile(path);

		KD_CORE_ASSERT(contents.Width > 0 && contents.Width <= s_LibraryData->Width);
		KD_CORE_ASSERT(contents.Height > 0 && contents.Height <= s_LibraryData->Height);

		TextureLayered2DLayerSpecification layerSpec{};
		layerSpec.Width = contents.Width;
		layerSpec.Height = contents.Height;
		layerSpec.InitialData = contents.Pixels;
		uint32_t layerIndex = s_LibraryData->Texture->Push(layerSpec);


		ImageSubresource resource = CalculateSubresource(layerIndex, contents.Width, contents.Height);

		s_LibraryData->Textures[path] = resource;
		s_LibraryData->LayerToTexture[layerIndex] = resource;
		
		return resource;
	}
	ImageSubresource TextureLibrary::LoadTexture(const std::string& name, const Path& path)
	{
		return s_LibraryData->NamedTextures[name] = LoadTexture(path);
	}
	ImageSubresource TextureLibrary::GetTexture(const Path& path)
	{
		return s_LibraryData->Textures.at(path);
	}
	ImageSubresource TextureLibrary::GetNamedTexture(const std::string& name)
	{
		return s_LibraryData->NamedTextures.at(name);
	}
	ImageSubresource TextureLibrary::GetTextureByLayer(uint32_t layerIndex)
	{
		return s_LibraryData->LayerToTexture.at(layerIndex);
	}
	Ref<TextureLayered2D> TextureLibrary::GetTextureArray()
	{
		return s_LibraryData->Texture;
	}

	Ref<Texture2D> TextureLibrary::LoadStaticTexture(const Path& path)
	{
		ParsedFileContents contents = ReadPixelsFromFile(path);
		
		Texture2DSpecification spec{};
		spec.Width = contents.Width;
		spec.Height = contents.Height;
		spec.InitialData = contents.Pixels;
		spec.Usage = TextureUsage_Sampled | TextureUsage_Updateable;
		spec.TextureFormat = Format::RGBA8UN;
		spec.MipMaps = 1;
		spec.Samples = TextureSamples::x1;

		return Texture2D::Create(spec);
	}


	TextureLibrary::ParsedFileContents TextureLibrary::ReadPixelsFromFile(const Path& path)
	{
		stbi_set_flip_vertically_on_load(true);
		char* bytes = nullptr;
		uint64_t size = 0;
		
		{
			std::ifstream file(path,std::ios::binary | std::ios::in | std::ios::ate);
			
			if (!file.is_open())
				return {};

			size = file.tellg();
			
			file.seekg(0,std::ios::beg);

			bytes = new char[size];

			file.read(bytes, size);
		}

		KD_CORE_ASSERT(bytes);
		KD_CORE_ASSERT(size);

		int width, height, channels;
		stbi_uc* pixels = stbi_load_from_memory((stbi_uc*)bytes, size, &width, &height, &channels, 4);

		delete[] bytes;

		ParsedFileContents contents{};
		contents.Width = width;
		contents.Height = height;
		contents.Pixels = pixels;

		return contents;
	}
	ImageSubresource TextureLibrary::CalculateSubresource(uint32_t layerIndex, uint32_t width, uint32_t height)
	{
		const Image& img = s_LibraryData->Texture->GetImage();
	
		glm::vec2 topLeftUV = { 0,(float)height / s_LibraryData->Height };
		glm::vec2 bottomRightUV = { (float)width / s_LibraryData->Width,0 };

		ImageSubresource resource{};
		resource.Layer = layerIndex;
		resource._InternalImageID = img._InternalImageID;
		resource._DeviceMemory = img._DeviceMemory;
		resource.ImageView = img.ImageView;
		resource.TopLeft = topLeftUV;
		resource.BottomRight = bottomRightUV;
		return resource;
	}
}
