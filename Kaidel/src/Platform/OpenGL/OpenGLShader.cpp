#include "KDpch.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace Kaidel {
	namespace Utils {

		static std::string ReadFile(const std::string& filePath) {
			std::ifstream file(filePath, std::ios::binary | std::ios::in);
			std::string res;
			KD_CORE_ASSERT(file, "Could not read from file: {}", filePath);
			file.seekg(0,std::ios::end);
			uint64_t size = file.tellg();
			KD_CORE_ASSERT(size != -1, "Could not read from file: {}", filePath);

			res.resize(size);

			file.seekg(std::ios::beg);
			file.read(&res[0], size);
			return res;
		}


		static std::unordered_map<ShaderType, std::string> GetShaderSources(const ShaderSpecification& specification) {
			std::unordered_map<ShaderType, std::string> shaderSources;
			for (const auto& shaderDefinition : specification.Definitions) {
				if (shaderSources.find(shaderDefinition.ShaderType) != shaderSources.end())
					continue;
				shaderSources[shaderDefinition.ShaderType] = shaderDefinition.IsPath ? ReadFile(shaderDefinition.ControlString) : shaderDefinition.ControlString; 
			}
			return shaderSources;
		}


		static GLenum KaidelShaderTypeToGLShaderType(ShaderType type) {
			switch (type)
			{
			case ShaderType::VertexShader:return GL_VERTEX_SHADER;
			case ShaderType::FragmentShader:return GL_FRAGMENT_SHADER;
			case ShaderType::GeometryShader:return GL_GEOMETRY_SHADER;
			case ShaderType::TessellationControlShader:return GL_TESS_CONTROL_SHADER;
			case ShaderType::TessellationEvaluationShader:return GL_TESS_EVALUATION_SHADER;
			}
			return 0;
		}

		static GLuint CreateShader(ShaderType type, const std::string& shaderSource) {
			GLuint shader = glCreateShader(KaidelShaderTypeToGLShaderType(type));
			const char* shaderCStringSource = shaderSource.c_str();
			glShaderSource(shader, 1, &shaderCStringSource, nullptr);

			glCompileShader(shader);
			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE) {
				GLint errorLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorLength);
				std::vector<GLchar> error(errorLength);
				glGetShaderInfoLog(shader, errorLength, &errorLength, error.data());
				glDeleteShader(shader);

				KD_CORE_ERROR("{0}", error.data());
				KD_CORE_ASSERT(false, "Shader compilation failure!");
				return -1;
			}
			return shader;
		}

		static void AttachShader(GLuint program, GLuint shader) {
			glAttachShader(program, shader);
		}


		static bool LinkProgram(GLuint program) {
			glLinkProgram(program);

			GLint isLinked = 0;
			glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
			if (isLinked == GL_FALSE) {
				GLint errorLength = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &errorLength);
				std::vector<GLchar> error(errorLength);
				glGetProgramInfoLog(program, errorLength, &errorLength, error.data());

				KD_CORE_ERROR("{0}", error.data());
				KD_CORE_ASSERT(false, "Program linkage failure!");
				return false;
			}
			return true;
		}

		static GLuint CreateProgram(const std::unordered_map<ShaderType, std::string>& shaderSources) {
			GLuint program = glCreateProgram();
			
			std::vector<GLuint> shaderIDs;

			for (const auto& [shaderType, shaderSource] : shaderSources) {
				GLuint shaderID = CreateShader(shaderType, shaderSource);
				if (shaderID != -1) {
					AttachShader(program, shaderID);
					shaderIDs.push_back(shaderID);
				}
			}

			if (!LinkProgram(program)) {
				glDeleteProgram(program);
				for (const auto& id : shaderIDs) {
					glDeleteShader(id);
				}
				return -1;
			}

			for (const auto& id : shaderIDs) {
				glDetachShader(program, id);
				glDeleteShader(id);
			}
			return program;
		}

	}

	OpenGLShader::OpenGLShader(const ShaderSpecification& specification) 
		:m_Specification(specification)
	{
		std::unordered_map<ShaderType, std::string> shaderSources;
		shaderSources = Utils::GetShaderSources(specification);
		m_RendererID = Utils::CreateProgram(shaderSources);
	}
	OpenGLShader::~OpenGLShader()
	{
		KD_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}



	void OpenGLShader::Bind() const
	{
		KD_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		KD_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		KD_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		KD_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		KD_PROFILE_FUNCTION();

		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		KD_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		KD_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		KD_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		glUseProgram(m_RendererID);
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}


	

}
