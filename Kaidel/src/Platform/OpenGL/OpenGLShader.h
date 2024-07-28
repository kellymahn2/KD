#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include <glm/glm.hpp>

// TODO: REMOVE!
typedef unsigned int GLenum;

namespace Kaidel {

	class OpenGLShader : public ShaderModule
	{
	public:
		OpenGLShader(const ShaderSpecification& specification);

		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value);
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count);
		virtual void SetFloat(const std::string& name, float value);
		virtual void SetFloat2(const std::string& name, const glm::vec2& value);
		virtual void SetFloat3(const std::string& name, const glm::vec3& value);
		virtual void SetFloat4(const std::string& name, const glm::vec4& value);
		virtual void SetMat4(const std::string& name, const glm::mat4& value);

		virtual const ShaderSpecification& GetSpecification()const  override { return m_Specification; }

	private:
		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		uint32_t m_RendererID;
		ShaderSpecification m_Specification;
	};

}
