#pragma once

#include "Shader.h"

namespace Kaidel {

	/*
		Cache style:
			TimeLastEdited: 8-byte unsigned integer
			ForEachShader:
				ShaderType: 1-byte unsigned char
				SpirvByteCount: 8-byte unsigned integer
				Spirv: SpirvByteCount bytes
	*/


	class ShaderLibrary {
	public:
		static void Init(const std::string& cacheFolder, const std::string& cacheFileExtension);

		static void Shutdown();

		static bool ShaderLoaded(const Path& path);
		static Ref<Shader> LoadShader(const Path& path);
		static Ref<Shader> LoadShader(const std::string& name, const Path& path);
		static Ref<Shader> GetShader(const Path& path);
		static Ref<Shader> GetNamedShader(const std::string& name);

		[[maybe_unused]] static Ref<Shader> UnloadShader(const Path& path);
		[[maybe_unused]] static Ref<Shader> UnloadNamedShader(const std::string& name);
	private:
		static std::string ReadFile(const Path& filepath);
		static void CreateCacheDirectoryIfNeeded();
		static Path GetCachePathForShader(const Path& name);

		static void CreateCache(const std::unordered_map<ShaderType, Spirv>& spirv, const Path& path);
		static bool IsCacheValid(const Path& path);
		static std::unordered_map<ShaderType, Spirv> ReadSPIRVsFromCache(const Path& path);
		static std::unordered_map<ShaderType, Spirv> ReadSPIRVsFromFile(const Path& path);
	};
}
