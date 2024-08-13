#include "KDpch.h"
#include "ShaderLibrary.h"


#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>


namespace Kaidel {
	
	namespace Utils {
		static shaderc_shader_kind KaidelShaderTypeToShaderCShaderKind(ShaderType type) {
			switch (type)
			{
			case Kaidel::ShaderType::VertexShader:return shaderc_vertex_shader;
			case Kaidel::ShaderType::FragmentShader:return shaderc_fragment_shader;
			case Kaidel::ShaderType::GeometryShader:return shaderc_geometry_shader;
			case Kaidel::ShaderType::TessellationControlShader:return shaderc_tess_control_shader;
			case Kaidel::ShaderType::TessellationEvaluationShader:return shaderc_tess_evaluation_shader;
			}
			KD_CORE_ASSERT(false,"Unknown shader type");

			return shaderc_anyhit_shader;
		}
	}

	struct ShaderLibraryData {
		std::unordered_map<Path, Ref<ShaderModule>> Shaders;
		std::unordered_map<std::string, Ref<ShaderModule>> NamedShaders;
		Path CachePath;
		std::string CachedFileExtension;
	};

	static ShaderLibraryData s_LibraryData;

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


	void ShaderLibrary::Init(const std::string& cacheFolder,const std::string& cacheFileExtension)
	{
		s_LibraryData.CachePath = cacheFolder;
		s_LibraryData.CachedFileExtension = cacheFileExtension;
		CreateCacheDirectoryIfNeeded();
	}
	void ShaderLibrary::Shutdown()
	{
		s_LibraryData.Shaders.clear();
		s_LibraryData.NamedShaders.clear();
	}
	bool ShaderLibrary::ShaderLoaded(const Path& path)
	{
		return s_LibraryData.Shaders.find(path) != s_LibraryData.Shaders.end();
	}
	Ref<ShaderModule> ShaderLibrary::LoadShader(const Path& path, ShaderType type)
	{
		std::vector<uint32_t> data;

		if (IsCacheValid(path)) {
			data = ReadSPIRVFromCache(path);
		}
		//Compile and cache if needed
		else {
			KD_CORE_INFO("Shader with file path {} being compiled", path);
			std::string source = ReadFile(path);

			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
#ifdef  KD_RELEASE 
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif //  KD_RELEASE 

			options.SetIncluder(CreateScope<ShaderIncluder>());

			shaderc::CompilationResult module = compiler.CompileGlslToSpv(source, Utils::KaidelShaderTypeToShaderCShaderKind(type), path.string().c_str(), options);

			if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
				KD_CORE_ERROR(module.GetErrorMessage());
				KD_ASSERT(false);
			}

			data = std::vector<uint32_t>(module.cbegin(), module.cend());
			CreateCache(data, path);
		}

		ShaderSpecification spec{};
		spec.EntryPoint = "main";
		spec.SPIRV = data;
		spec.Type = type;
		Ref<ShaderModule> shader = ShaderModule::Create(spec);

		s_LibraryData.Shaders[path] = shader;

		//Reflect(shader, type, data);

		return shader;
	}
	Ref<ShaderModule> ShaderLibrary::LoadShader(const std::string& name, const Path& path, ShaderType type)
	{
		return s_LibraryData.NamedShaders[name] = LoadShader(path, type);
	}
	Ref<ShaderModule> ShaderLibrary::GetShader(const Path& path)
	{
		return s_LibraryData.Shaders.at(path);
	}
	Ref<ShaderModule> ShaderLibrary::GetNamedShader(const std::string& name)
	{
		return s_LibraryData.NamedShaders.at(name);
	}
	Ref<ShaderModule> ShaderLibrary::UnloadShader(const std::string& name)
	{
		Ref<ShaderModule> shader = s_LibraryData.NamedShaders.at(name);
		s_LibraryData.NamedShaders.erase(name);
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
		if (!std::filesystem::exists(s_LibraryData.CachePath))
			std::filesystem::create_directories(s_LibraryData.CachePath);
	}
	Path ShaderLibrary::GetCachePathForShader(const Path& name)
	{
		return s_LibraryData.CachePath / (name.filename().string() + s_LibraryData.CachedFileExtension);
	}
	void ShaderLibrary::CreateCache(const std::vector<uint32_t>& spirv, const Path& path)
	{

		std::ofstream file(GetCachePathForShader(path),std::ios::binary| std::ios::out);
		if (!file.is_open())
			return;

		FileSystem::file_time_type lastWrite = FileSystem::last_write_time(path);

		uint64_t lastWriteTime = lastWrite.time_since_epoch().count();

		//TODO: should be written and read as BE.

		file.write((const char*)&lastWriteTime, sizeof(uint64_t));

		file.write((char*)spirv.data(), spirv.size() * sizeof(uint32_t));
	}
	bool ShaderLibrary::IsCacheValid(const Path& path)
	{
		std::ifstream file(GetCachePathForShader(path),std::ios::binary | std::ios::in);
		
		if (!file.is_open())
			return false;

		uint64_t lastWriteTime = 0;
		file.read((char*)&lastWriteTime, sizeof(uint64_t));

		FileSystem::file_time_type lastWrite = FileSystem::last_write_time(path);

		uint64_t actualLastWriteTime = lastWrite.time_since_epoch().count();

		return actualLastWriteTime == lastWriteTime;
	}
	std::vector<uint32_t> ShaderLibrary::ReadSPIRVFromCache(const Path& path)
	{
		std::ifstream file(GetCachePathForShader(path), std::ios::binary | std::ios::in);

		if (!file.is_open())
			return {};

		uint64_t binarySize = 0;

		file.seekg(0, std::ios::end);

		binarySize = file.tellg();

		binarySize -= sizeof(uint64_t);

		file.seekg(sizeof(uint64_t), std::ios::beg);

		std::vector<uint32_t> spirv(binarySize / sizeof(uint32_t), 0);

		file.read((char*)spirv.data(), binarySize);

		uint64_t read = file.gcount();
		if (!file.good())
			return {};
		return spirv;
	}
}
