#include "KDpch.h"

#include "ShaderLibrary.h"
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <sstream>

namespace Kaidel {

	namespace Utils {
		static shaderc_shader_kind KaidelShaderTypeToShaderCShaderKind(ShaderType type) {
			switch (type)
			{
			case ShaderType::VertexShader:return shaderc_vertex_shader;
			case ShaderType::FragmentShader:return shaderc_fragment_shader;
			case ShaderType::GeometryShader:return shaderc_geometry_shader;
			case ShaderType::TessellationControlShader:return shaderc_tess_control_shader;
			case ShaderType::TessellationEvaluationShader:return shaderc_tess_evaluation_shader;
			}
			KD_CORE_ASSERT(false, "Unknown shader type");

			return shaderc_anyhit_shader;
		}

		static ShaderType ShaderTypeFromString(const std::string_view& view) {
			if (view == "vertex") {
				return ShaderType::VertexShader;
			}
		}

	}

	struct ShaderLibraryData {
		std::unordered_map<Path, Ref<Shader>> Shaders;
		std::unordered_map<std::string, Ref<Shader>> NamedShaders;
		Path CachePath;
		std::string CachedFileExtension;
		std::unordered_map<std::string, ShaderType> TypeStringsToTypes;
	};

	class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface {
	public:
		shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override
		{
			std::string includeDir = "";
			if (requesting_source) {
				std::string requestingSource = requesting_source;
				size_t lastSlashIndex = requestingSource.find_last_of("/\\");
				if (lastSlashIndex != std::string::npos) {
					includeDir = requestingSource.substr(0, lastSlashIndex);
				}
			}

			std::string fullPath = includeDir + "/" + requested_source;

			std::ifstream file(fullPath);
			if (!file.is_open()) {
				return nullptr;
			}

			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();

			shaderc_include_result* result = new shaderc_include_result;
			result->content = strdup(content.c_str());
			result->content_length = content.size();
			result->source_name = strdup(fullPath.c_str());
			result->source_name_length = fullPath.size();
			result->user_data = nullptr;

			return result;
		}
		void ReleaseInclude(shaderc_include_result* data) override
		{
			delete[] data->content;
			delete[] data->source_name;
			delete data;
		}
	};

	static ShaderLibraryData* s_LibraryData;

	void ShaderLibrary::Init(const std::string& cacheFolder, const std::string& cacheFileExtension)
	{
		s_LibraryData = new ShaderLibraryData;
		s_LibraryData->CachePath = cacheFolder;
		s_LibraryData->CachedFileExtension = cacheFileExtension;
		CreateCacheDirectoryIfNeeded();
		s_LibraryData->TypeStringsToTypes = {
			{"vert",ShaderType::VertexShader},
			{"tcs",ShaderType::TessellationControlShader},
			{"tes",ShaderType::TessellationEvaluationShader},
			{"geo",ShaderType::GeometryShader},
			{"frag",ShaderType::FragmentShader},
			{"vertex",ShaderType::VertexShader},
			{"geometry",ShaderType::GeometryShader},
			{"fragment",ShaderType::FragmentShader},
		};
	}
	
	void ShaderLibrary::Shutdown()
	{
		delete s_LibraryData;
	}
	
	bool ShaderLibrary::ShaderLoaded(const Path& path)
	{
		return s_LibraryData->Shaders.find(path) != s_LibraryData->Shaders.end();
	}
	
	Ref<Shader> ShaderLibrary::LoadShader(const Path& path)
	{
		std::unordered_map<ShaderType, Spirv> spirvs;
		if (IsCacheValid(path)) {
			spirvs = ReadSPIRVsFromCache(path);
		}
		else {
			spirvs = ReadSPIRVsFromFile(path);
			CreateCache(spirvs, path);
		}

		Ref<Shader> shader = Shader::Create(spirvs);

		s_LibraryData->Shaders[path] = shader;

		return shader;
	}
	
	Ref<Shader> ShaderLibrary::LoadShader(const std::string& name, const Path& path)
	{
		std::unordered_map<ShaderType, Spirv> spirvs;
		if (IsCacheValid(path)) {
			spirvs = ReadSPIRVsFromCache(path);
		}
		else {
			spirvs = ReadSPIRVsFromFile(path);
			CreateCache(spirvs, path);
		}

		Ref<Shader> shader = Shader::Create(spirvs);

		s_LibraryData->NamedShaders[name] = shader;

		return shader;
	}
	
	Ref<Shader> ShaderLibrary::GetShader(const Path& path)
	{
		return s_LibraryData->Shaders.at(path);
	}
	
	Ref<Shader> ShaderLibrary::GetNamedShader(const std::string& name)
	{
		return s_LibraryData->NamedShaders.at(name);
	}
	
	Ref<Shader> ShaderLibrary::UnloadShader(const Path& path)
	{
		Ref<Shader> shader = s_LibraryData->Shaders.at(path);
		s_LibraryData->Shaders.erase(path);
		return shader;
	}
	
	Ref<Shader> ShaderLibrary::UnloadNamedShader(const std::string& name)
	{
		Ref<Shader> shader = s_LibraryData->NamedShaders.at(name);
		s_LibraryData->NamedShaders.erase(name);
		return shader;
	}
	
	std::string ShaderLibrary::ReadFile(const Path& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			}
		}

		return result;
	}
	
	void ShaderLibrary::CreateCacheDirectoryIfNeeded()
	{
		if (!std::filesystem::exists(s_LibraryData->CachePath))
			std::filesystem::create_directories(s_LibraryData->CachePath);
	}
	
	Path ShaderLibrary::GetCachePathForShader(const Path& name)
	{
		return s_LibraryData->CachePath / (name.filename().string() + s_LibraryData->CachedFileExtension);
	}

	void ShaderLibrary::CreateCache(const std::unordered_map<ShaderType, Spirv>& spirvs, const Path& path)
	{
		std::ofstream file(GetCachePathForShader(path), std::ios::binary | std::ios::out);
		if (!file.is_open())
			return;

		FileSystem::file_time_type lastWrite = FileSystem::last_write_time(path);

		uint64_t lastWriteTime = lastWrite.time_since_epoch().count();
		file.write((const char*)&lastWriteTime, sizeof(uint64_t));

		for (auto& [type, spirv] : spirvs) {
			char cType = (char)type;
			file.write(&cType, sizeof(char));
			
			uint64_t byteCount = spirv.size() * sizeof(uint32_t);
			
			file.write((const char*)&byteCount, sizeof(uint64_t));

			file.write((char*)spirv.data(), spirv.size() * sizeof(uint32_t));
		}
	}
	
	bool ShaderLibrary::IsCacheValid(const Path& path)
	{
		std::ifstream file(GetCachePathForShader(path), std::ios::binary | std::ios::in);

		if (!file.is_open())
			return false;

		uint64_t lastWriteTime = 0;
		file.read((char*)&lastWriteTime, sizeof(uint64_t));

		FileSystem::file_time_type lastWrite = FileSystem::last_write_time(path);

		uint64_t actualLastWriteTime = lastWrite.time_since_epoch().count();

		return actualLastWriteTime == lastWriteTime;
	}
	
	std::unordered_map<ShaderType, Spirv> ShaderLibrary::ReadSPIRVsFromCache(const Path& path)
	{
		std::ifstream file(GetCachePathForShader(path), std::ios::binary | std::ios::in);

		if (!file.is_open())
			return {};

		file.seekg(sizeof(uint64_t), std::ios::beg);

		std::unordered_map<ShaderType, Spirv> result;
		while (true) {
				
			char cType = 0;
			file.read(&cType, sizeof(char));
			if (file.eof())
				return result;
			ShaderType type = (ShaderType)cType;

			uint64_t byteCount = 0;
			file.read((char*)&byteCount, sizeof(uint64_t));

			KD_CORE_ASSERT(byteCount % 4 == 0);

			std::vector<uint32_t> spirv;
			spirv.resize(byteCount / 4);

			file.read((char*)spirv.data(), byteCount);
			result[type] = std::move(spirv);
		}

		return result;
	}
	
	std::unordered_map<ShaderType, Spirv> ShaderLibrary::ReadSPIRVsFromFile(const Path& path)
	{
		std::string content = ReadFile(path);
		shaderc::Compiler comp;

		std::string_view contentView = content;

		//Preprocess
		std::unordered_map<ShaderType, std::string_view> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = content.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = content.find_first_of("\r\n", pos);
			KD_CORE_ASSERT(eol != std::string::npos);

			size_t begin = pos + typeTokenLength + 1;
			
			std::string type = content.substr(begin, eol - begin);

			size_t nextLinePos = content.find_first_not_of("\r\n", eol);
			KD_CORE_ASSERT(nextLinePos != std::string::npos);
			
			pos = content.find(typeToken, nextLinePos);

			shaderSources[s_LibraryData->TypeStringsToTypes.at(type)] = 
				(pos == std::string::npos) ? contentView.substr(nextLinePos) : contentView.substr(nextLinePos, pos - nextLinePos);
		}

		std::unordered_map<ShaderType, Spirv> spirvs;

		shaderc::Compiler compiler;
		shaderc::CompileOptions opt;

		opt.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
#ifdef  KD_RELEASE 
		opt.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif //  KD_RELEASE 

		opt.SetIncluder(CreateScope<ShaderIncluder>());
		
		std::string pathStr = path.string();

		for (auto& [type, src] : shaderSources) {
			shaderc::CompilationResult res = 
				compiler.CompileGlslToSpv(src.data(), src.size(), Utils::KaidelShaderTypeToShaderCShaderKind(type), pathStr.c_str(), "main", opt);
			
			if (res.GetCompilationStatus() != shaderc_compilation_status_success) {
				KD_CORE_ERROR(res.GetErrorMessage());
				KD_CORE_ASSERT(false);
			}
			
			spirvs[type] = std::vector<uint32_t>(res.cbegin(), res.cend());
		}

		return spirvs;
	}
}
