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
			case ShaderType::ComputeShader:return shaderc_compute_shader;
			}
			KD_CORE_ASSERT(false, "Unknown shader type");

			return shaderc_anyhit_shader;
		}

		static ShaderType ShaderTypeFromString(const std::string_view& view) {
			if (view == "vertex") {
				return ShaderType::VertexShader;
			}
		}

		std::string RemoveEmptyLines(const std::string& s) {
			std::stringstream input(s);
			std::string line;
			std::string result;
			bool first = true;

			while (std::getline(input, line)) {
				// Check if line has non-whitespace characters
				bool non_ws = false;
				for (char c : line) {
					if (!std::isspace(static_cast<unsigned char>(c))) {
						non_ws = true;
						break;
					}
				}
				if (non_ws) {
					if (!first) result += '\n';
					result += line;
					first = false;
				}
			}
			// Note: trailing newline behavior is preserved by not adding extra at end
			return result;
		}

		

	}

	struct ShaderLibraryData {
		std::unordered_map<Path, uint64_t> TimeAtLoad;
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
			SCOPED_TIMER(fmt::format("Including shader {}", requested_source));

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
			{"tcs",ShaderType::TessellationControlShader},
			{"tes",ShaderType::TessellationEvaluationShader},
			{"vertex",ShaderType::VertexShader},
			{"geometry",ShaderType::GeometryShader},
			{"fragment",ShaderType::FragmentShader},
			{"compute",ShaderType::ComputeShader}
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

		s_LibraryData->TimeAtLoad[path] = FileSystem::last_write_time(path).time_since_epoch().count();
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
			try {
				spirvs = ReadSPIRVsFromFile(path);
				CreateCache(spirvs, path);
			}
			catch(std::exception&)
			{ }
			
		}

		Ref<Shader> shader = Shader::Create(spirvs);

		s_LibraryData->TimeAtLoad[path] = FileSystem::last_write_time(path).time_since_epoch().count();

		s_LibraryData->NamedShaders[name] = shader;
		s_LibraryData->Shaders[path] = shader;

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

	Ref<Shader> ShaderLibrary::LoadOrGetShader(const Path& path)
	{
		auto it = s_LibraryData->Shaders.find(path);
		if (it != s_LibraryData->Shaders.end()) {
			return it->second;
		}
		return LoadShader(path);
	}

	Ref<Shader> ShaderLibrary::LoadOrGetNamedShader(const std::string& name, const Path& path)
	{
		auto it = s_LibraryData->NamedShaders.find(name);
		if (it != s_LibraryData->NamedShaders.end()) {
			return it->second;
		}
		return LoadShader(name, path);
	}
	
	Ref<Shader> ShaderLibrary::CompileFromSource(const std::string& source)
	{
		std::unordered_map<ShaderType, Spirv> spirvs;

		spirvs = ReadSPIRVsFromSource(source, Path("assets/_shaders/"));

		return Shader::Create(spirvs);
	}

	void ShaderLibrary::UpdateShader(const Path& path)
	{
		KD_CORE_ASSERT(ShaderLoaded(path));

		std::unordered_map<ShaderType, Spirv> spirvs = ReadSPIRVsFromFile(path);
		spirvs = ReadSPIRVsFromFile(path);
		CreateCache(spirvs, path);
		
		s_LibraryData->TimeAtLoad[path] = FileSystem::last_write_time(path).time_since_epoch().count();

		s_LibraryData->Shaders[path]->Update(spirvs);
	}

	std::unordered_map<Path, Ref<Shader>>& ShaderLibrary::GetAllShaders()
	{
		return s_LibraryData->Shaders;
	}

	uint64_t ShaderLibrary::GetTimeAtLoad(const Path& path)
	{
		return s_LibraryData->TimeAtLoad[path];
	}

	std::unordered_map<std::string, Ref<Shader>> ShaderLibrary::BatchLoad(const std::unordered_map<std::string, Path>& paths)
	{
		std::unordered_map<std::string, Ref<Shader>> results;

		for (auto& [name, path] : paths)
		{
			results[name] = {};
		}

		for (auto& [name, path] : paths)
		{
			auto task = [&name, &path, &results](const JobArgs&) {
				std::unordered_map<ShaderType, Spirv> spirvs;
				if (IsCacheValid(path)) {
					spirvs = ReadSPIRVsFromCache(path);
				}
				else {
					try {
						spirvs = ReadSPIRVsFromFile(path);
						CreateCache(spirvs, path);
					}
					catch (std::exception&)
					{
						KD_ASSERT(false);
					}

				}

				Ref<Shader> shader = Shader::Create(spirvs);

				results[name] = shader;
			};

			task({});
		}
		
		JobSystem::GetDefault().Wait();

		for (auto& [name, shader] : results)
		{
			s_LibraryData->Shaders[paths.at(name)] = shader;
			s_LibraryData->NamedShaders[name] = shader;
			s_LibraryData->TimeAtLoad[paths.at(name)] = FileSystem::last_write_time(paths.at(name)).time_since_epoch().count();
		}

		return results;
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
#ifdef KD_DEBUG
		return s_LibraryData->CachePath / (name.filename().string() + s_LibraryData->CachedFileExtension + "d");
#elif KD_RELEASE
		return s_LibraryData->CachePath / (name.filename().string() + s_LibraryData->CachedFileExtension);
#endif 

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

		KD_INFO("Compiling shader at: {}", path);

		return ReadSPIRVsFromSource(content, path);
	}

	std::unordered_map<ShaderType, Spirv> ShaderLibrary::ReadSPIRVsFromSource(const std::string& source, const Path& path)
	{
		SCOPED_TIMER(fmt::format("Compiling shader from source", path.string()));

		std::unordered_map<ShaderType, Spirv> spirvs;

		std::string pathStr = path.string();

		auto sources = PreprocessSource(source);

		for (auto& [type, src] : sources) {

			shaderc::Compiler compiler;

			std::string lines;
			shaderc::CompileOptions opt;

			{
				SCOPED_TIMER(fmt::format("Preprocessing shader"));

				opt.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
				opt.SetGenerateDebugInfo();
#ifdef  KD_RELEASE 
				opt.SetOptimizationLevel(shaderc_optimization_level_performance);
				opt.SetPreserveBindings(true);
#endif //  KD_RELEASE 

				opt.SetIncluder(CreateScope<ShaderIncluder>());

				auto c = compiler.PreprocessGlsl(src, Utils::KaidelShaderTypeToShaderCShaderKind(type), pathStr.c_str(), opt);

				if (c.GetCompilationStatus() != shaderc_compilation_status_success) {
					KD_ERROR(c.GetErrorMessage());

					KD_INFO("Shader Code");

					uint32_t lineNum = 0;

					std::string line;
					std::stringstream preprocessed(std::string(c.begin(), c.end()));
					while (std::getline(preprocessed, line))
					{
						KD_INFO("{} {}", lineNum++, line);
					}

					KD_ASSERT(false);
					throw std::exception("Error");
				}

				lines = std::string(c.begin(), c.end());
			}

			//lines = Utils::RemoveEmptyLines(lines);

			SCOPED_TIMER(fmt::format("Compiling shader"));

			shaderc::CompilationResult res =
				compiler.CompileGlslToSpv(lines.c_str(), lines.length(), Utils::KaidelShaderTypeToShaderCShaderKind(type), pathStr.c_str(), "main", opt);

			if (res.GetCompilationStatus() != shaderc_compilation_status_success) {
				KD_ERROR(res.GetErrorMessage());

				KD_INFO("Shader Code");

				uint32_t lineNum = 0;

				std::string line;
				std::stringstream preprocessed(lines);
				while (std::getline(preprocessed, line))
				{
					//if (std::any_of(line.begin(), line.end(), [](const char& c) { return std::isprint(c); }))
					KD_INFO("{} {}", lineNum, line);
					++lineNum;
				}

				KD_ASSERT(false);
			}

			spirvs[type] = std::vector<uint32_t>(res.cbegin(), res.cend());
		}

		return spirvs;
	}

	std::unordered_map<Kaidel::ShaderType, std::string> ShaderLibrary::PreprocessSource(const std::string& source)
	{

		std::unordered_map<ShaderType, std::string> res;

		std::string common;

		std::stringstream sourceStream(source);

		std::string line;
		std::string_view sub = source;

		while (std::getline(sourceStream, line))
		{
			if (line.find("#type") != std::string::npos)
			{
				break;
			}
			common += line;

			if (line.empty() || line.back() != '\n')
			{
				common += '\n';
			}

			sub = sub.substr(line.length() + 1);
		}
		
		while (!sub.empty())
		{
			if (sub[0] != '#')
			{
				std::size_t loc = sub.find('#');

				//no more shaders
				if (loc == std::string::npos)
				{
					break;
				}

				sub = sub.substr(loc);
			}

			std::size_t lineEnd = sub.find_first_of("\r\n");

			static const constexpr auto TYPE_LENGTH = 5;

			//skip #ifdef
			auto shaderType = sub.substr(TYPE_LENGTH, lineEnd - TYPE_LENGTH);

			std::size_t shaderTypeStart = shaderType.find_first_not_of(' ');
			shaderType = shaderType.substr(shaderTypeStart, shaderType.find_last_not_of(' ') - shaderTypeStart + 1);

			ShaderType type = s_LibraryData->TypeStringsToTypes.at(std::string(shaderType));

			KD_ASSERT(res.find(type) == res.end());

			sub = sub.substr(lineEnd + (sub[lineEnd] == '\r' ? 2 : 1));

			std::size_t shaderEnd = sub.find("#type");

			auto shaderCode = common + std::string(sub.substr(0, shaderEnd));

			res[type] = std::move(shaderCode);

			if (shaderEnd == std::string::npos)
			{
				break;
			}

			sub = sub.substr(shaderEnd);
		}

		return res;
	}

}
