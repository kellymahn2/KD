#include "KDpch.h"
#include "Platform/D3D/D3DShader.h"
#include "Platform/D3D/D3DContext.h"
#include <fstream>
#include <glad/glad.h>
#include "D3Dcompiler.h"

#include <glm/gtc/type_ptr.hpp>
namespace Kaidel {

	static GLenum ShaderTypeFromString(const std::string& type)
	{
		return type == "vertex"?0:1;
	}

	D3DShader::D3DShader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);
		std::unordered_map<GLenum,std::string> shaderSources = PreProcess(source);
		Compile(shaderSources);
	}

	D3DShader::D3DShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		Compile({ {0,vertexSrc},{1,fragmentSrc} });
	}

	D3DShader::~D3DShader()
	{
		if (m_VertexShader)
			m_VertexShader->Release();
		if (m_PixelShader)
			m_PixelShader->Release();
		if (m_VSBlob)
			m_VSBlob->Release();
		if (m_PSBlob)
			m_PSBlob->Release();
	}

	std::string D3DShader::ReadFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
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
			else
			{
				KD_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else
		{
			KD_CORE_ERROR("Could not open file '{0}'", filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> D3DShader::PreProcess(const std::string& source)
	{

		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			KD_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			//KD_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			KD_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void D3DShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		auto d3dContext = D3DContext::Get();
		auto& vbs = shaderSources.at(0);
		auto& pbs = shaderSources.at(1);
		const char* v = R"(
struct VertexInput
{
	float3 a_Position : POSITION;
	float4 a_Color : COLOR;
	int a_EntityID : TEXCOORD;
};

cbuffer Camera : register(b0)
{
    matrix u_ViewProjection;
};

struct VertexOutput
{
	float4 Pos : SV_Position;
	float4 Color : COLOR;
	int EntityID : TEXCOORD;
};

VertexOutput main(VertexInput input)
{
    VertexOutput output;
	output.Pos = mul(float4(input.a_Position,1.0f),u_ViewProjection);
    output.Color = input.a_Color;
	output.EntityID = input.a_EntityID;
    return output;
}
)";
		ID3DBlob* VSByteCode = nullptr;
		auto res = D3DCompile(
			vbs.data(),
			vbs.length(),
			nullptr,
			nullptr,
			nullptr,
			"main",
			"vs_5_0",
			D3DCOMPILE_DEBUG, 0,
			&VSByteCode, nullptr);


		D3DASSERT(d3dContext->GetDevice()->CreateVertexShader(VSByteCode->GetBufferPointer(),
			VSByteCode->GetBufferSize(), nullptr, &m_VertexShader));
		m_VSBlob = VSByteCode;
		ID3DBlob* PSByteCode = nullptr;
		res = D3DCompile(
			pbs.data(),
			pbs.length(),
			nullptr,
			nullptr,
			nullptr,
			"main",
			"ps_5_0",
			D3DCOMPILE_DEBUG, 0,
			&PSByteCode, nullptr);
		D3DASSERT(d3dContext->GetDevice()->CreatePixelShader(PSByteCode->GetBufferPointer(),
			PSByteCode->GetBufferSize(), nullptr, &m_PixelShader));
		m_PSBlob = PSByteCode;
	}

	void D3DShader::Bind() const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
		d3dContext->GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);
	}

	void D3DShader::Unbind() const
	{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->PSSetShader(nullptr,nullptr,0);
		d3dContext->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);
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
