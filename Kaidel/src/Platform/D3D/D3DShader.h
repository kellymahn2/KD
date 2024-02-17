#pragma once

#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include "Kaidel/Renderer/GraphicsAPI/UniformBuffer.h"
#include <glm/glm.hpp>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
typedef unsigned int GLenum;

namespace Kaidel {

	class D3DShader : public Shader
	{
	public:
		
		D3DShader(const ShaderSpecification& spec);
		
		virtual ~D3DShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const std::string& GetName() const override { return m_Specification.ShaderName; }

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		ID3DBlob* GetVSBlob()const { return m_VSBlob; }
	private:
		std::unordered_map<ShaderType, void*> m_Shaders;
		ID3DBlob* m_VSBlob;
		ShaderSpecification m_Specification;
		std::vector<byte> m_UniformBytes;
	};

}
