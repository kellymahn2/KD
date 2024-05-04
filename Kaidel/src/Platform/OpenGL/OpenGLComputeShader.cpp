#include "KDpch.h"
#include "OpenGLComputeShader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
namespace Kaidel {
	
	namespace Utils {
		inline std::string GetFileContents(const FileSystem::path& filePath) {
			std::ifstream file(filePath, std::ios::binary | std::ios::in);
			std::string res;
			KD_CORE_ASSERT(file, "Could not read from file");
			file.seekg(0, std::ios::end);
			uint64_t size = file.tellg();
			KD_CORE_ASSERT(size != -1, "Could not read from file");

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


	OpenGLComputeShaderUAVInput::OpenGLComputeShaderUAVInput(uint32_t count,uint32_t sizeofElement, void* data) {
		m_SizeofElement = sizeofElement;
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, count*sizeofElement, data, GL_DYNAMIC_DRAW);
		++UAVInput::s_UAVCount;
	}
	void OpenGLComputeShaderUAVInput::SetBufferData(void* data, uint32_t count){
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, count*m_SizeofElement, data, GL_DYNAMIC_DRAW);
	}
	OpenGLComputeShaderUAVInput::~OpenGLComputeShaderUAVInput(){
		glDeleteBuffers(1, &m_RendererID);
		--UAVInput::s_UAVCount;
	}
	void OpenGLComputeShaderUAVInput::Bind(uint32_t slot)const {
		m_LastBoundSlot = slot;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_RendererID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	void OpenGLComputeShaderUAVInput::Unbind()const{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_LastBoundSlot, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}



	OpenGLTypedBufferInput::OpenGLTypedBufferInput(TypedBufferInputDataType type, TypedBufferAccessMode accessMode, uint32_t width, uint32_t height, void* data)
		:m_InputType(type),m_AccessMode(accessMode)
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		switch (type)
		{
		case Kaidel::TypedBufferInputDataType::None:
			break;
		case Kaidel::TypedBufferInputDataType::RGBA8:
			m_InternalFormat = GL_RGBA8;
			m_Format = GL_RGBA;
			break;
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0, m_Format,
			GL_FLOAT, data);
		
	}
	OpenGLTypedBufferInput::~OpenGLTypedBufferInput(){
		glDeleteTextures(1, &m_RendererID);
	}
	void OpenGLTypedBufferInput::SetBufferData(void* data, uint32_t width,uint32_t height) {
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, width, height, 0,m_Format , GL_FLOAT, data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	uint64_t OpenGLTypedBufferInput::GetTextureID()const {
		return m_RendererID;
	}
	void OpenGLTypedBufferInput::Bind(uint32_t slot)const {
		GLint accessmode = 0;
		switch (m_AccessMode)
		{
		case Kaidel::Read:
			accessmode = GL_READ_ONLY;
			break;
		case Kaidel::Write:
			accessmode = GL_WRITE_ONLY;
			break;
		case Kaidel::ReadWrite:
			accessmode = GL_READ_WRITE;
			break;
		}
			
		m_LastBoundSlot = slot;
		glBindImageTexture(slot, m_RendererID, 0, GL_FALSE, 0, accessmode, m_InternalFormat);
	}
	void OpenGLTypedBufferInput::Unbind() const {
	}
	OpenGLComputeShader::OpenGLComputeShader(const std::string& filepath)
	{
		std::string s = Utils::PreprocessShaderSource(filepath,Utils::GetFileContents(filepath));
		uint32_t csShader = 0;
		csShader = glCreateShader(GL_COMPUTE_SHADER);
		const char* src = s.c_str();
		int length = (int)s.capacity();
		glShaderSource(csShader, 1, &src, nullptr);
		glCompileShader(csShader);
		{
			GLint isCompiled = 0;
			glGetShaderiv(csShader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(csShader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(csShader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(csShader);

				KD_CORE_ERROR("{0}", infoLog.data());
				KD_CORE_ASSERT(false, "Shader compilation failure!");
			}
		}

		m_RendererID = glCreateProgram();
		glAttachShader(m_RendererID, csShader);
		glLinkProgram(m_RendererID);

		{
			GLint isLinked = 0;
			glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*)&isLinked);
			if (isLinked == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

				// We don't need the program anymore.
				glDeleteProgram(m_RendererID);
				glDeleteShader(csShader);
				KD_CORE_ERROR("{0}", infoLog.data());
				KD_CORE_ASSERT(false, "Shader link failure!");
				return;
			}
		}
		int32_t localSize[3];
		glGetProgramiv(m_RendererID, GL_COMPUTE_WORK_GROUP_SIZE, localSize);

		m_LocalX = localSize[0];
		m_LocalY = localSize[1];
		m_LocalZ = localSize[2];
		glDeleteShader(csShader);

	}
	OpenGLComputeShader::~OpenGLComputeShader()
	{
		glDeleteProgram(m_RendererID);
	}
	void OpenGLComputeShader::Bind() const 
	{
		glUseProgram(m_RendererID);
	}
	void OpenGLComputeShader::Unbind() const 
	{
		glUseProgram(0);
	}

	void OpenGLComputeShader::SetUAVInput(Ref<UAVInput> uav, uint32_t slot) {
		uav->Bind(slot);
	}
	void OpenGLComputeShader::SetTypedBufferInput(Ref<TypedBufferInput> tbi, uint32_t slot) {
		tbi->Bind(slot);
	}

	void OpenGLComputeShader::Wait()const {
		Bind();
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		Unbind();
	}

	void OpenGLComputeShader::Execute(uint64_t x, uint64_t y, uint64_t z) const
	{
		Bind();
		glDispatchCompute((GLuint)x, (GLuint)y, (GLuint)z);
		Unbind();	
	}


	void OpenGLComputeShader::SetInt(const std::string& name, int value) {
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);

	}
	void OpenGLComputeShader::SetIntArray(const std::string& name, int* values, uint32_t count) {
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count,values);
	}
	void OpenGLComputeShader::SetFloat(const std::string& name, float value) {
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location,value);
	}
	void OpenGLComputeShader::SetFloat2(const std::string& name, const glm::vec2& value) {
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}
	void OpenGLComputeShader::SetFloat3(const std::string& name, const glm::vec3& value) {
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}
	void OpenGLComputeShader::SetFloat4(const std::string& name, const glm::vec4& value) {
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}
	void OpenGLComputeShader::SetMat4(const std::string& name, const glm::mat4& value) {
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
	}

}
