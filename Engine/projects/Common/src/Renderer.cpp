#include "Renderer.hpp"

using Microsoft::WRL::ComPtr;


namespace leopph
{
	Renderer::Renderer(Window& window)
	{
		UINT deviceCreationFlags = 0;
		#ifndef NDEBUG
		deviceCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif

		D3D_FEATURE_LEVEL constexpr featureLevels[]{D3D_FEATURE_LEVEL_11_0};

		D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceCreationFlags, featureLevels, 1, D3D11_SDK_VERSION, mD3dDevice.GetAddressOf(), nullptr, mD3dDeviceContext.GetAddressOf());

		#ifndef NDEBUG
		{
			mD3dDevice.As<ID3D11Debug>(&mD3dDebug);

			ComPtr<ID3D11InfoQueue> infoQueue;
			mD3dDebug.As<ID3D11InfoQueue>(&infoQueue);

			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
		}
		#endif

		DXGI_SWAP_CHAIN_DESC1 constexpr swapChainDesc
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
			.Flags = SWAP_CHAIN_FLAGS
		};

		ComPtr<IDXGIDevice2> dxgiDevice2;
		mD3dDevice.As(&dxgiDevice2);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		dxgiDevice2->GetAdapter(dxgiAdapter.GetAddressOf());

		ComPtr<IDXGIFactory5> dxgiFactory5;
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory5), reinterpret_cast<void**>(dxgiFactory5.GetAddressOf()));

		dxgiFactory5->CreateSwapChainForHwnd(mD3dDevice.Get(), window.get_hwnd(), &swapChainDesc, nullptr, nullptr, mSwapChain.GetAddressOf());

		dxgiFactory5->MakeWindowAssociation(window.get_hwnd(), DXGI_MWA_NO_WINDOW_CHANGES);

		window.set_size_callback([this](WORD const width, WORD const height)
		{
			if (width && height)
			{
				mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, SWAP_CHAIN_FLAGS);
			}
		});
	}



	void Renderer::render() const
	{
		mSwapChain->Present(mSyncInterval, DXGI_PRESENT_ALLOW_TEARING);
	}



	void Renderer::set_sync_interval(UINT const interval)
	{
		mSyncInterval = interval;
	}
}
