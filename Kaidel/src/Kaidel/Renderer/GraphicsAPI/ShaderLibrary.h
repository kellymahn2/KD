#pragma once
#include "Shader.h"


namespace Kaidel {
	class ShaderLibrary {
	public:
		static void Init(const std::string& cacheFolder, const std::string& cacheFileExtension);

		static void Shutdown();

		static bool ShaderLoaded(const Path& path);
		static Ref<Shader> LoadShader(const Path& path, ShaderType type, bool cache = true);
		static Ref<Shader> LoadShader(const std::string& name, const Path& path, ShaderType type, bool cache = true);
		static Ref<Shader> GetShader(const Path& path);
		static Ref<Shader> GetNamedShader(const std::string& name);
		[[maybe_unused]] static Ref<Shader> UnloadShader(const std::string& name);
	private:
		static std::string ReadFile(const Path& filepath);
		static void CreateCacheDirectoryIfNeeded();
		static Path GetCachePathForShader(const Path& name);
	};
}
