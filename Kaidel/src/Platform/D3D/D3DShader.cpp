#include "KDpch.h"
#include "Platform/D3D/D3DShader.h"
#include "Platform/D3D/D3DContext.h"
#include <fstream>
#include "D3Dcompiler.h"

#include <glm/gtc/type_ptr.hpp>
namespace Kaidel {

	namespace Utils {
		static std::string ReadFile(const std::string& filePath) {
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


		static std::unordered_map<ShaderType, std::string> GetShaderSources(const ShaderSpecification& specification) {
			std::unordered_map<ShaderType, std::string> shaderSources;
			for (const auto& shaderDefinition : specification.Definitions) {
				if (shaderSources.find(shaderDefinition.ShaderType) != shaderSources.end())
					continue;
				shaderSources[shaderDefinition.ShaderType] = shaderDefinition.IsPath ? ReadFile(shaderDefinition.ControlString) : shaderDefinition.ControlString;
			}
			return shaderSources;
		}


		static const char* KaidelShaderTypeToD3DCompilerTarget(ShaderType type) {
			switch (type)
			{
			case Kaidel::ShaderType::VertexShader:return "vs_5_0";
			case Kaidel::ShaderType::PixelShader:return "ps_5_0";
			case Kaidel::ShaderType::GeometryShader:return "gs_5_0";
			case Kaidel::ShaderType::TessellationControlShader:return "hs_5_0";
			case Kaidel::ShaderType::TessellationEvaluationShader:return "ds_5_0";
			}
			return nullptr;
		}



		static ID3DBlob* CompileShader(ShaderType type, const std::string& shaderSource) {

			ID3DBlob* byteCode = nullptr;


			auto compilationFlags = 0;
#ifdef KD_DEBUG
			compilationFlags |= D3DCOMPILE_DEBUG;
#endif // KD_DEBUG

				D3DASSERT(D3DCompile(shaderSource.data(),shaderSource.length(),nullptr,nullptr,nullptr,"main",KaidelShaderTypeToD3DCompilerTarget(type),compilationFlags, 0,&byteCode, nullptr));
				return byteCode;
		}


		static void* CreateShader(ShaderType type, ID3DBlob* blob) {
			auto d3dContext = D3DContext::Get();
			switch (type)
			{
			case Kaidel::ShaderType::VertexShader: {
				ID3D11VertexShader* vertexShader = nullptr;
				D3DASSERT(d3dContext->GetDevice()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader));
				return vertexShader;
			}
			case Kaidel::ShaderType::PixelShader: {
				ID3D11PixelShader* pixelShader = nullptr;
				D3DASSERT(d3dContext->GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader));
				return pixelShader;
			}
			case Kaidel::ShaderType::GeometryShader: {
				ID3D11GeometryShader* geometryShader = nullptr;
				D3DASSERT(d3dContext->GetDevice()->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &geometryShader));
				return geometryShader;
			}
			case Kaidel::ShaderType::TessellationControlShader: {
				ID3D11HullShader* hullShader = nullptr;
				D3DASSERT(d3dContext->GetDevice()->CreateHullShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &hullShader));
				return hullShader;
			}
			case Kaidel::ShaderType::TessellationEvaluationShader: {
				ID3D11DomainShader* domainShader = nullptr;
				D3DASSERT(d3dContext->GetDevice()->CreateDomainShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &domainShader));
				return domainShader;
			}
			}
			return nullptr;
		}

		static std::unordered_map<ShaderType, void*> CreateProgram(const std::unordered_map<ShaderType, std::string>& shaderSources,ID3DBlob** outVertexShaderBlob) {
			std::unordered_map<ShaderType, void*> shaders;
			for (auto& [type, shaderSource] : shaderSources) {
				ID3DBlob* blob = CompileShader(type, shaderSource);
				shaders[type] = CreateShader(type, blob);
				if (type == ShaderType::VertexShader) {
					*outVertexShaderBlob = blob;
					continue;
				}

				ID3D11ShaderReflection* shaderReflection = nullptr;
				D3DASSERT(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&shaderReflection));
				D3D11_SHADER_DESC desc{};
				shaderReflection->GetDesc(&desc);

				blob->Release();
			}
			return shaders;
		}

		static void DeleteShader(ShaderType type, void* shader) {
			switch (type)
			{
			case Kaidel::ShaderType::VertexShader: {
				if (shader) {
					((ID3D11VertexShader*)(shader))->Release();
				}
				return;
			}
			case Kaidel::ShaderType::PixelShader: {
				if (shader) {
					((ID3D11PixelShader*)(shader))->Release();
				}
				return;
			}
			case Kaidel::ShaderType::GeometryShader: {
				if (shader) {
					((ID3D11GeometryShader*)(shader))->Release();
				}
				return;
			}
			case Kaidel::ShaderType::TessellationControlShader: {
				if (shader) {
					((ID3D11HullShader*)(shader))->Release();
				}
				return;
			}
			case Kaidel::ShaderType::TessellationEvaluationShader: {
				if (shader) {
					((ID3D11DomainShader*)(shader))->Release();
				}
				return;
			}
			}
		}

		

		static void BindShader(ShaderType type, void* shader) {
			auto d3dContext = D3DContext::Get();
			switch (type)
			{
			case Kaidel::ShaderType::VertexShader: {
				d3dContext->GetDeviceContext()->VSSetShader((ID3D11VertexShader*)shader, nullptr, 0);
				return;
			}
			case Kaidel::ShaderType::PixelShader: {
				d3dContext->GetDeviceContext()->PSSetShader((ID3D11PixelShader*)shader, nullptr, 0);
				return;
			}
			case Kaidel::ShaderType::GeometryShader: {
				d3dContext->GetDeviceContext()->GSSetShader((ID3D11GeometryShader*)shader, nullptr, 0);
				return;
			}
			case Kaidel::ShaderType::TessellationControlShader: {
				d3dContext->GetDeviceContext()->HSSetShader((ID3D11HullShader*)shader, nullptr, 0);
				return;
			}
			case Kaidel::ShaderType::TessellationEvaluationShader: {
				d3dContext->GetDeviceContext()->DSSetShader((ID3D11DomainShader*)shader, nullptr, 0);
				return;
			}
			}
		}

	}


	D3DShader::D3DShader(const ShaderSpecification& spec) {
		std::unordered_map<ShaderType, std::string> shaderSources;
		shaderSources = Utils::GetShaderSources(spec);
		m_Shaders = Utils::CreateProgram(shaderSources,&m_VSBlob);
	}


	D3DShader::~D3DShader()
	{
		for (auto& [type, shader] : m_Shaders) {
			Utils::DeleteShader(type, shader);
		}

		if (m_VSBlob) {
			m_VSBlob->Release();
		}
	}




	void D3DShader::Bind() const
	{
		for (auto& [type, shader] : m_Shaders) {
			Utils::BindShader(type, shader);
		}

	}

	void D3DShader::Unbind() const
	{
		for (auto& [type, shader] : m_Shaders) {
			Utils::BindShader(type, nullptr);
		}
	}

	void D3DShader::SetInt(const std::string& name, int value)
	{
	
	}

	void D3DShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		
	}

	void D3DShader::SetFloat(const std::string& name, float value)
	{
		
	}

	void D3DShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		
	}

	void D3DShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		
	}

	void D3DShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		
	}

	void D3DShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		
	}

	void D3DShader::UploadUniformInt(const std::string& name, int value)
	{
		
	}

	void D3DShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		
	}

	void D3DShader::UploadUniformFloat(const std::string& name, float value)
	{
		
	}

	void D3DShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		
	}

	void D3DShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		
	}

	void D3DShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		
	}

	void D3DShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
	}

	void D3DShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		
	}
	
}
