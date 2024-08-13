#pragma once
#include "Texture.h"


namespace Kaidel {
	class TextureLibrary {
	public:
		static void Init(uint32_t width,uint32_t height,Format format);

		static void PushDefaultTextures();

		static void Shutdown();

		static bool PathedTextureLoaded(const Path& path);

		static ImageSubresource LoadTexture(const Path& path);
		static ImageSubresource LoadTexture(const std::string& name, const Path& path);

		static ImageSubresource GetTexture(const Path& path);
		static ImageSubresource GetNamedTexture(const std::string& name);
		static ImageSubresource GetTextureByLayer(uint32_t layerIndex);

		static Ref<TextureLayered2D> GetTextureArray();

		static Ref<Texture2D> LoadStaticTexture(const Path& path);

	private:
		
		struct ParsedFileContents {
			void* Pixels;
			uint32_t Width;
			uint32_t Height;
		};

	private:
		static ParsedFileContents ReadPixelsFromFile(const Path& path);
		static ImageSubresource CalculateSubresource(uint32_t layerIndex, uint32_t width,uint32_t height);
	};
}
