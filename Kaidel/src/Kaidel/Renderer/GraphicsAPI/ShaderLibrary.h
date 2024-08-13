#pragma once
#include "Shader.h"


namespace Kaidel {
	class ShaderLibrary {
	public:
		static void Init(const std::string& cacheFolder, const std::string& cacheFileExtension);

		static void Shutdown();

		static bool ShaderLoaded(const Path& path);
		static Ref<ShaderModule> LoadShader(const Path& path, ShaderType type);
		static Ref<ShaderModule> LoadShader(const std::string& name, const Path& path, ShaderType type);
		static Ref<ShaderModule> GetShader(const Path& path);
		static Ref<ShaderModule> GetNamedShader(const std::string& name);
		[[maybe_unused]] static Ref<ShaderModule> UnloadShader(const std::string& name);
	private:
		static std::string ReadFile(const Path& filepath);
		static void CreateCacheDirectoryIfNeeded();
		static Path GetCachePathForShader(const Path& name);

		static void CreateCache(const std::vector<uint32_t>& spirv, const Path& path);
		static bool IsCacheValid(const Path& path);
		static std::vector<uint32_t> ReadSPIRVFromCache(const Path& path);

	};
}
