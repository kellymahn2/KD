#pragma once

#include "Kaidel/Renderer/GraphicsAPI/GraphicsContext.h"
#include <d3d11.h>
#include <D3DX10.h>
#include <D3DX11.h>
std::string _GetErrorMessage(HRESULT hr);
#include <iostream>
#ifdef KD_DEBUG
#define D3DASSERT(hr) {auto res = hr;if(!SUCCEEDED(res)){std::cout<<_GetErrorMessage(res)<<std::endl;__debugbreak();}}
#else
#define D3DASSERT(hr) hr
#endif // KD_DEBUG

struct GLFWwindow;

namespace Kaidel {

	class D3DContext : public GraphicsContext
	{
	public:
		D3DContext(GLFWwindow* windowHandle);
		~D3DContext();
		virtual void Init() override;
		virtual void SwapBuffers() override;
		void RecreateSwapChain(uint32_t width, uint32_t height);
		static D3DContext* Get();
		ID3D11Device* GetDevice()
		{
			return m_Device;
		}
		ID3D11DeviceContext* GetDefferedDeviceContext()
		{
			return m_DefferedDeviceContext;
		}
		ID3D11DeviceContext* GetDeviceContext() {
		
			return m_DeviceContext;
		}


		IDXGISwapChain* GetSwapChain()
		{
			return m_SwapChain;

		}


		ID3D11CommandList*& GetCommandList() {
			return m_DefferedContextCommandList;

		}

		D3D11_RASTERIZER_DESC& GetSettings() { return m_Settings; }
		ID3D11RenderTargetView* GetBackBuffer() { return m_BackBuffer; }
		ID3D11DepthStencilView* GetDepthStencilView() { return m_DepthStencilView; }
	private:
		GLFWwindow* m_WindowHandle;
		ID3D11Device* m_Device = nullptr;
		ID3D11DeviceContext* m_DeviceContext = nullptr;
		ID3D11DeviceContext* m_DefferedDeviceContext = nullptr;
		ID3D11CommandList* m_DefferedContextCommandList = nullptr;
		IDXGISwapChain* m_SwapChain;
		ID3D11RenderTargetView* m_BackBuffer = nullptr;
		ID3D11Texture2D* m_DepthStencilBuffer = nullptr;
		ID3D11RasterizerState* m_RasterizerState = nullptr;
		ID3D11DepthStencilView* m_DepthStencilView= nullptr;
		D3D11_RASTERIZER_DESC m_Settings{};

	};

}
