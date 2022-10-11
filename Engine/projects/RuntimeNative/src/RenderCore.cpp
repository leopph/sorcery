#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>

#include <wrl/client.h>

#include "Window.hpp"

#include <cassert>
#include <functional>

using Microsoft::WRL::ComPtr;


namespace leopph
{
	class RenderCore
	{
	private:
		ComPtr<ID3D11Device> mDevice;
		ComPtr<ID3D11DeviceContext> mContext;
		ComPtr<IDXGISwapChain1> mSwapChain;
		ComPtr<ID3D11RenderTargetView> mBackBufRtv;

		static void on_window_resize(RenderCore* self, Extent2D size);

	public:
		RenderCore(Window& window);
	};


	RenderCore::RenderCore(Window& window)
	{
#ifdef NDEBUG
		UINT constexpr deviceCreationFlags = 0;
#else
		UINT constexpr deviceCreationFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{D3D_FEATURE_LEVEL_11_0};

		HRESULT hresult = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceCreationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, mDevice.GetAddressOf(), nullptr, mContext.GetAddressOf());
		assert(SUCCEEDED(hresult));


#ifndef NDEBUG
		ComPtr<ID3D11Debug> d3dDebug;
		hresult = mDevice.As(&d3dDebug);
		assert(SUCCEEDED(hresult));

		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		hresult = d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue);
		assert(SUCCEEDED(hresult));

		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

		ComPtr<IDXGIDevice> dxgiDevice;
		hresult = mDevice.As(&dxgiDevice);
		assert(SUCCEEDED(hresult));

		ComPtr<IDXGIAdapter> dxgiAdapter;
		hresult = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
		assert(SUCCEEDED(hresult));

		ComPtr<IDXGIFactory2> dxgiFactory2;
		hresult = dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory2)::InterfaceType), reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));
		assert(SUCCEEDED(hresult));

		DXGI_SWAP_CHAIN_DESC1 constexpr swapChainDesc1
		{
			.Width = 0,
			.Height = 0,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = FALSE,
			.SampleDesc =
			{
				.Count = 1,
				.Quality = 0
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = 0
		};

		hresult = dxgiFactory2->CreateSwapChainForHwnd(mDevice.Get(), window.get_hwnd(), &swapChainDesc1, nullptr, nullptr, mSwapChain.GetAddressOf());
		assert(SUCCEEDED(hresult));

		ComPtr<ID3D11Texture2D> backBuf;
		hresult = mSwapChain->GetBuffer(0, __uuidof(decltype(backBuf)::InterfaceType), reinterpret_cast<void**>(backBuf.GetAddressOf()));
		assert(SUCCEEDED(hresult));

		hresult = mDevice->CreateRenderTargetView(backBuf.Get(), nullptr, mBackBufRtv.GetAddressOf());
		assert(SUCCEEDED(hresult));

		window.OnSizeEvent.add_handler(this, &on_window_resize);
	}


	void RenderCore::on_window_resize(RenderCore* const self, Extent2D const size)
	{
		self->mBackBufRtv.Reset();
		self->mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		ComPtr<ID3D11Texture2D> backBuf;
		HRESULT hresult = self->mSwapChain->GetBuffer(0, __uuidof(decltype(backBuf)::InterfaceType), reinterpret_cast<void**>(backBuf.GetAddressOf()));
		assert(SUCCEEDED(hresult));
		hresult = self->mDevice->CreateRenderTargetView(backBuf.Get(), nullptr, self->mBackBufRtv.GetAddressOf());
		assert(SUCCEEDED(hresult));
	}
}