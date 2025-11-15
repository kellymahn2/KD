#pragma once
#include "Kaidel/Core/Base.h"

#include "Texture.h"
#include "Kaidel/Renderer/EnvironmentMap.h"

namespace Kaidel {
	
	struct TextureCubePathParameters {
		Path Right, Left, Top, Bottom, Front, Back;
		TextureCubePathParameters(
			const Path& right, const Path& left,
			const Path& top, const Path& bottom,
			const Path& front, const Path& back)
			: Right(right), Left(left), Top(top), Bottom(bottom), Front(front), Back(back)
		{
		}
	};

	class TextureLibrary {
	public:
		static void Init();
		static void Shutdown();

		static Ref<Texture2D> Load(const Path& path, ImageLayout layout, Format format);
		static Ref<TextureCube> LoadCube(const TextureCubePathParameters& pathParams, ImageLayout layout, Format format);
		static Ref<EnvironmentMap> LoadEquiRectHDR(const Path& path, uint32_t width, uint32_t height);
		static Ref<Texture2D> Get(const Path& path);
		static Ref<Texture2D> LoadFromBuffer(const void* buffer, uint64_t size, ImageLayout layout, Format format);
		static const Path& GetPath(Ref<Texture2D> texture);
		
		static bool Exists(const Path& path);
	};

}
