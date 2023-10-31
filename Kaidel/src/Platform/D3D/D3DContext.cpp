#include "KDpch.h"
#include "Platform/D3D/D3DContext.h"
#include "Kaidel/Core/Application.h"

#include "Kaidel\Renderer\Renderer.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#pragma comment(lib,"DXGI.lib")
std::string _GetErrorMessage(HRESULT hr)
 {
	 LPSTR errorMessage = nullptr;
	 size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		 nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorMessage, 0, nullptr);
	 return size > 0 ? errorMessage : "Unspecified Error";
}
namespace Kaidel {
	
	static D3DContext* s_MainContext = nullptr;
	struct ImGuiDirectXRes {
		ID3D11Device* Device;
		ID3D11DeviceContext* DeviceContext;
	};
	ImGuiDirectXRes InitImGuiForDirectX() {
		return { s_MainContext->GetDevice(),s_MainContext->GetDeviceContext() };
	}

	D3DContext::D3DContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle) , m_Device(nullptr),m_DeviceContext(nullptr),m_SwapChain(nullptr)
	{
		KD_CORE_ASSERT(windowHandle, "Window handle is null!");
		
	}

	D3DContext::~D3DContext()
	{
		if (m_BackBuffer)
			m_BackBuffer->Release();
		if (m_SwapChain)
			m_SwapChain->Release();
		if (m_Device)
			m_Device->Release();
		if (m_DeviceContext)
			m_DeviceContext->Release();
		if (m_DefferedDeviceContext)
			m_DefferedDeviceContext->Release();
		if (m_DefferedContextCommandList)
			m_DefferedContextCommandList->Release();
	}

	void D3DContext::Init()
	{
		if (!s_MainContext)
			s_MainContext = this;
		KD_PROFILE_FUNCTION();
		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
		scd.BufferDesc.Width = 1280;
		scd.BufferDesc.Height = 720;
		scd.BufferCount = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = glfwGetWin32Window(m_WindowHandle);
		scd.SampleDesc.Count = 4;
		scd.Windowed = true;
		D3D11CreateDeviceAndSwapChain(NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			0,
			NULL,
			NULL,
			D3D11_SDK_VERSION,
			&scd,
			&m_SwapChain,
			&m_Device,
			NULL,
			&m_DeviceContext);
		ID3D11Resource* back;
		m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**) & back);
		D3DASSERT(m_Device->CreateRenderTargetView(back, nullptr, &m_BackBuffer));
		back->Release();


		D3D11_TEXTURE2D_DESC depthStencilDesc{};

		depthStencilDesc.Width = scd.BufferDesc.Width;
		depthStencilDesc.Height = scd.BufferDesc.Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Depth-stencil format
		depthStencilDesc.SampleDesc.Count = scd.SampleDesc.Count; // Use the same sample count as the swap chain
		depthStencilDesc.SampleDesc.Quality = scd.SampleDesc.Quality; // Use the same sample quality as the swap chain
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		m_Device->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthStencilBuffer);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		D3DASSERT(m_Device->CreateDepthStencilView(m_DepthStencilBuffer,nullptr, &m_DepthStencilView));


		m_DeviceContext->OMSetRenderTargets(1, &m_BackBuffer, nullptr);



		m_Settings.CullMode = D3D11_CULL_NONE;
		m_Settings.FillMode = D3D11_FILL_SOLID;
		m_Settings.FrontCounterClockwise = true;
		m_Settings.DepthBias = 0;
		m_Settings.SlopeScaledDepthBias = 0.0f;
		m_Settings.DepthBiasClamp = 0.0f;
		m_Settings.DepthClipEnable = TRUE;
		m_Settings.ScissorEnable = FALSE;
		m_Settings.MultisampleEnable = FALSE;
		m_Settings.AntialiasedLineEnable = FALSE;
		D3DASSERT(m_Device->CreateRasterizerState(&m_Settings, &m_RasterizerState));
		m_DeviceContext->RSSetState(m_RasterizerState);

		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = 1280;
		viewport.Height = 720;

		m_DeviceContext->RSSetViewports(1, &viewport);
	}

	void D3DContext::SwapBuffers()
	{
		KD_PROFILE_FUNCTION();
		D3DASSERT(m_SwapChain->Present(0, 0));
		m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void D3DContext::RecreateSwapChain(uint32_t width, uint32_t height)
	{
		if (m_SwapChain)
			m_SwapChain->Release();
		if (m_BackBuffer)
			m_BackBuffer->Release();
		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
		scd.BufferDesc.Width = width;
		scd.BufferDesc.Height = height;
		scd.BufferCount = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = glfwGetWin32Window(m_WindowHandle);
		scd.SampleDesc.Count = 4;
		scd.Windowed = true;
		IDXGIFactory* factory;
		D3DASSERT(CreateDXGIFactory(__uuidof(IDXGIFactory),(void**) & factory));
		factory->CreateSwapChain(m_Device, &scd, &m_SwapChain);
		factory->Release();
		ID3D11Resource* back;
		m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void**)&back);
		D3DASSERT(m_Device->CreateRenderTargetView(back, nullptr, &m_BackBuffer));
		back->Release();
		if (m_DepthStencilBuffer)
			m_DepthStencilBuffer->Release();
		if (m_DepthStencilView)
			m_DepthStencilView->Release();

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

		depthStencilDesc.Width = scd.BufferDesc.Width;
		depthStencilDesc.Height = scd.BufferDesc.Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Depth-stencil format
		depthStencilDesc.SampleDesc.Count = scd.SampleDesc.Count; // Use the same sample count as the swap chain
		depthStencilDesc.SampleDesc.Quality = scd.SampleDesc.Quality; // Use the same sample quality as the swap chain
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		m_Device->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthStencilBuffer);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		D3DASSERT(m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, &m_DepthStencilView));

		m_DeviceContext->OMSetRenderTargets(1, &m_BackBuffer, m_DepthStencilView);
	}

	D3DContext* D3DContext::Get()
	{
		return s_MainContext;
	}

}
