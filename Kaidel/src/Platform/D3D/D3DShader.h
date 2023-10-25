#pragma once

#include "Kaidel/Renderer/Shader.h"
#include <glm/glm.hpp>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>

typedef unsigned int GLenum;
namespace Kaidel {

	class D3DShader : public Shader
	{
	public:
		D3DShader(const std::string& filepath);
		D3DShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
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

		virtual const std::string& GetName() const override { return m_Name; }

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values, uint32_t count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);
		ID3DBlob* GetVSBlob()const { return m_VSBlob; }
		ID3DBlob* GetPSBlob()const { return m_PSBlob; }



	


	private:
		std::string ReadFile(const std::string& filepath);
		std::unordered_map<ShaderType, std::string> PreProcess(const std::string& source);
		void Compile(const std::unordered_map<ShaderType, std::string>& shaderSources);
	private:
		uint32_t m_RendererID;
		ID3D11VertexShader* m_VertexShader = nullptr;
		ID3D11PixelShader* m_PixelShader = nullptr;
		ID3D11GeometryShader* m_GeometryShader = nullptr;
		ID3D11ComputeShader* m_ComputeShader = nullptr;
		ID3DBlob* m_VSBlob = nullptr;
		ID3DBlob* m_PSBlob = nullptr;
		ID3DBlob* m_GSBlob = nullptr;
		ID3DBlob* m_CSBlob = nullptr;
		std::string m_Name;
	};

}
