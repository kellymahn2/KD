#include "KDpch.h"
#include "Platform/D3D/D3DShader.h"
#include "Platform/D3D/D3DContext.h"
#include <fstream>
#include "D3Dcompiler.h"

#include <glm/gtc/type_ptr.hpp>
namespace Kaidel {

	static ShaderType ShaderTypeFromString(const std::string& type)
	{
		//return type == "vertex"?Shader::ShaderType_Vertex: type=="fragment" ? Shader::ShaderType_Pixel: type=="geometry" ? Shader::ShaderType_Geometry: Shader::ShaderType_Compute;
		return ShaderType::VertexShader;
	}

	D3DShader::D3DShader(const std::string& filepath)
	{
		std::string source = ReadFile(filepath);
		std::unordered_map<ShaderType,std::string> shaderSources = PreProcess(source);
		Compile(shaderSources);
	}

	D3DShader::D3DShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		//Compile({ {ShaderType_Vertex,vertexSrc},{ShaderType_Pixel,fragmentSrc} });
	}

	D3DShader::~D3DShader()
	{
		if (m_VertexShader)
			m_VertexShader->Release();
		if (m_PixelShader)
			m_PixelShader->Release();
		if (m_GeometryShader)
			m_GeometryShader->Release();
		if (m_VSBlob)
			m_VSBlob->Release();
		if (m_PSBlob)
			m_PSBlob->Release();
		if (m_GSBlob)
			m_GSBlob->Release();
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

	std::unordered_map<ShaderType, std::string> D3DShader::PreProcess(const std::string& source)
	{

		std::unordered_map<ShaderType, std::string> shaderSources;

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

	void D3DShader::Compile(const std::unordered_map<ShaderType, std::string>& shaderSources)
	{
//		auto d3dContext = D3DContext::Get();
//		auto compilationFlags = 0;
//#ifdef KD_DEBUG
//		compilationFlags |= D3DCOMPILE_DEBUG;
//#endif // KD_DEBUG
//
//		if (shaderSources.find(ShaderType_Vertex)!=shaderSources.end()) {
//			auto& vss = shaderSources.at(ShaderType_Vertex);
//			auto res = D3DCompile(
//				vss.data(),
//				vss.length(),
//				nullptr,
//				nullptr,
//				nullptr,
//				"main",
//				"vs_5_0",
//				compilationFlags, 0,
//				&m_VSBlob, nullptr);
//			D3DASSERT(d3dContext->GetDevice()->CreateVertexShader(m_VSBlob->GetBufferPointer(),
//				m_VSBlob->GetBufferSize(), nullptr, &m_VertexShader));
//			m_ShaderType |= ShaderType_Vertex;
//		}
//		if (shaderSources.find(ShaderType_Pixel) != shaderSources.end()) {
//
//			auto& pss = shaderSources.at(ShaderType_Pixel);
//			D3DASSERT(D3DCompile(
//				pss.data(),
//				pss.length(),
//				nullptr,
//				nullptr,
//				nullptr,
//				"main",
//				"ps_5_0",
//				compilationFlags, 0,
//				&m_PSBlob, nullptr));
//			D3DASSERT(d3dContext->GetDevice()->CreatePixelShader(m_PSBlob->GetBufferPointer(),
//				m_PSBlob->GetBufferSize(), nullptr, &m_PixelShader));
//			m_ShaderType |= ShaderType_Pixel;
//		}
//		if (shaderSources.find(ShaderType_Geometry) != shaderSources.end()) {
//			auto& gss = shaderSources.at(ShaderType_Geometry);
//			D3DASSERT(D3DCompile(
//				gss.data(),
//				gss.length(),
//				nullptr,
//				nullptr,
//				nullptr,
//				"main",
//				"gs_5_0",
//				compilationFlags,
//				0,
//				&m_GSBlob,
//				nullptr));
//			D3DASSERT(d3dContext->GetDevice()->CreateGeometryShader(m_GSBlob->GetBufferPointer(),
//				m_GSBlob->GetBufferSize(), nullptr, &m_GeometryShader));
//			m_ShaderType |= ShaderType_Geometry;
//		}
//		if (shaderSources.find(ShaderType_Compute) != shaderSources.end()) {
//			auto& css = shaderSources.at(ShaderType_Compute);
//			D3DASSERT(D3DCompile(
//				css.data(),
//				css.length(),
//				nullptr,
//				nullptr,
//				nullptr,
//				"main",
//				"cs_5_0",
//				compilationFlags,
//				0,
//				&m_CSBlob,
//				nullptr
//			));
//			D3DASSERT(d3dContext->GetDevice()->CreateComputeShader(m_CSBlob->GetBufferPointer(),
//				m_CSBlob->GetBufferSize(), nullptr, &m_ComputeShader));
//			m_ShaderType |= ShaderType_Compute;
//		}

	}

	void D3DShader::Bind() const
	{
		/*auto d3dContext = D3DContext::Get();
		if(m_ShaderType&ShaderType_Vertex)
			d3dContext->GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
		if (m_ShaderType & ShaderType_Pixel)
		d3dContext->GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);
		if (m_ShaderType & ShaderType_Geometry)
		d3dContext->GetDeviceContext()->GSSetShader(m_GeometryShader, nullptr, 0);
		if (m_ShaderType & ShaderType_Compute)
			d3dContext->GetDeviceContext()->CSSetShader(m_ComputeShader, nullptr, 0);*/
	}

	void D3DShader::Unbind() const
	{
		/*auto d3dContext = D3DContext::Get();
		if (m_ShaderType & ShaderType_Vertex)
			d3dContext->GetDeviceContext()->VSSetShader(nullptr, nullptr, 0);
		if (m_ShaderType & ShaderType_Pixel)
			d3dContext->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);
		if (m_ShaderType & ShaderType_Geometry)
			d3dContext->GetDeviceContext()->GSSetShader(nullptr, nullptr, 0);
		if (m_ShaderType & ShaderType_Compute)
			d3dContext->GetDeviceContext()->CSSetShader(nullptr, nullptr, 0);*/
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
