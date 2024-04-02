#include "KDpch.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"

#include "Kaidel/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/OpenGL/OpenGLComputeShader.h"

namespace Kaidel {

	uint64_t UAVInput::s_UAVCount = 0;


	Ref<Shader> Shader::Create(const ShaderSpecification& specification) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLShader>(specification);
		//case RendererAPI::API::DirectX: return CreateRef<D3DShader>(specification);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}





	Ref<UAVInput> UAVInput::Create(uint32_t count,uint32_t sizeofElement, void* data) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLComputeShaderUAVInput>(count,sizeofElement,data);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<TypedBufferInput> TypedBufferInput::Create(TypedBufferInputDataType type, TypedBufferAccessMode accessMode, uint32_t width, uint32_t height, void* data) {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::OpenGL:  return CreateRef<OpenGLTypedBufferInput>(type,accessMode,width,height,data);
		}
		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}




	Ref<ComputeShader> ComputeShader::Create(const std::string& filepath) {
		switch (Renderer::GetAPI())
		{

		case RendererAPI::API::None:KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:return CreateRef<OpenGLComputeShader>(filepath);
		//case RendererAPI::API::DirectX:return CreateRef<D3DComputeShader>(filepath);

		}
		KD_CORE_ASSERT(false, "Unkown RendererAPI!");

		return nullptr;
	}

	namespace Utils {
		inline std::string GetFileContents(const FileSystem::path& filePath) {
			std::ifstream file(filePath, std::ios::binary | std::ios::in);
			std::string res;
			KD_CORE_ASSERT(file, "Could not read from file: {}", filePath);
			file.seekg(0, std::ios::end);
			uint64_t size = file.tellg();
			KD_CORE_ASSERT(size != -1, "Could not read from file: {}", filePath);

			res.resize(size);

			file.seekg(std::ios::beg);
			file.read(&res[0], size);
			return res;
		}

		static std::string PreprocessShaderSource(const FileSystem::path& path, const std::string& unprocessedSource) {
			std::string processedSourceStream;

			processedSourceStream.reserve(50000);
			std::stringstream unprocessedSourceStream(unprocessedSource);
			std::string line;

			while (std::getline(unprocessedSourceStream, line)) {
				if (line.find("#include") != std::string::npos) {
					std::string includeFileName;
					std::stringstream lineStream(line);
					std::string includeToken;
					lineStream >> includeToken >> includeFileName;

					includeFileName.erase(std::remove(includeFileName.begin(), includeFileName.end(), '"'), includeFileName.end());

					FileSystem::path includeFilePath = path.parent_path() / includeFileName;

					includeFilePath = FileSystem::canonical(includeFilePath);

					if (FileSystem::exists(includeFilePath)) {
						std::string includedSource = GetFileContents(includeFilePath);

						std::string processedIncludeSource = PreprocessShaderSource(includeFilePath, includedSource);
						processedSourceStream = processedSourceStream + processedIncludeSource + "\n";
					}
				}
				else {
					std::string s = line + "\n";
					processedSourceStream = processedSourceStream + s;
				}
			}
			return processedSourceStream;
		}

	}




	ShaderSource::ShaderSource(const Path& path) {

	}

}
