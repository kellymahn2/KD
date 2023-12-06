#include "KDpch.h"
#include "D3DComputeShader.h"
#include "D3DContext.h"
#include "D3Dcompiler.h"


namespace Kaidel {

	namespace Utils {
		inline std::string GetFileContents(const std::string& filepath) {
			std::ifstream file(filepath);


			int length = 0;
			file.seekg(0, std::ios_base::end);
			length = file.tellg();
			file.seekg(0, std::ios_base::beg);
			std::string res;
			res.reserve(length);
			file.read(res.data(), length);
			return res;
		}

	}



	D3DComputeShader::D3DComputeShader(const std::string& filepath) {
		auto d3dContext = D3DContext::Get();
		ID3DBlob* csBlob = nullptr;
		{
			std::string src = Utils::GetFileContents(filepath);
			D3DASSERT(D3DCompile(src.c_str(), src.length(), nullptr, nullptr, nullptr, "main", "cs_5_0", 0, 0, &csBlob, nullptr));

		}
		D3DASSERT(d3dContext->GetDevice()->CreateComputeShader(csBlob->GetBufferPointer(), csBlob->GetBufferSize(),nullptr, &m_ComputeShader));
		csBlob->Release();
	}
	D3DComputeShader::~D3DComputeShader() {
		if (m_ComputeShader != nullptr) {
			m_ComputeShader->Release();
		}
	}
	void D3DComputeShader::Bind()  const {
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->CSSetShader(m_ComputeShader, nullptr, 0);
	}
	void D3DComputeShader::Unbind()  const {
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->CSSetShader(nullptr, nullptr, 0);
	}
	void D3DComputeShader::Execute(uint64_t x, uint64_t y, uint64_t z)  const{
		auto d3dContext = D3DContext::Get();
		d3dContext->GetDeviceContext()->Dispatch(x, y, z);
	}

}
