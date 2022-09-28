#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

#include <cassert>
#include <cstdio>

using Microsoft::WRL::ComPtr;


LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);


int main()
{
	WNDCLASSW const wndClass
	{
		.style = 0,
		.lpfnWndProc = window_proc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = GetModuleHandleW(0),
		.hIcon = nullptr,
		.hCursor = nullptr,
		.hbrBackground = nullptr,
		.lpszMenuName = nullptr,
		.lpszClassName = L"MyWindow",
	};

	if (!RegisterClassW(&wndClass))
	{
		std::fprintf(stderr, "Failed to register window class.\n");
		return -1;
	}

	auto const hwnd = CreateWindowEx(0, wndClass.lpszClassName, L"MyWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wndClass.hInstance, 0);

	if (!hwnd)
	{
		std::fprintf(stderr, "Failed to create window.\n");
		return -1;
	}

	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dDeviceContext;

#ifdef NDEBUG
	UINT constexpr deviceCreationFlags = 0;
#else
	UINT constexpr deviceCreationFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{ D3D_FEATURE_LEVEL_11_0 };

	assert(SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceCreationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, d3dDevice.GetAddressOf(), nullptr, d3dDeviceContext.GetAddressOf())));

#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	assert(SUCCEEDED(d3dDevice.As(&d3dDebug)));

	ComPtr<ID3D11InfoQueue> d3dInfoQueue;
	assert(SUCCEEDED(d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue)));

	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

	ComPtr<IDXGIDevice> dxgiDevice;
	assert(SUCCEEDED(d3dDevice.As(&dxgiDevice)));

	ComPtr<IDXGIAdapter> dxgiAdapter;
	assert(SUCCEEDED(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf())));

	ComPtr<IDXGIFactory2> dxgiFactory2;
	assert(SUCCEEDED(dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory2)::InterfaceType), reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()))));

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
	
	ComPtr<IDXGISwapChain1> dxgiSwapChain1;
	assert(SUCCEEDED(dxgiFactory2->CreateSwapChainForHwnd(d3dDevice.Get(), hwnd, &swapChainDesc1, nullptr, nullptr, dxgiSwapChain1.GetAddressOf())));

	ComPtr<ID3D11Texture2D> backBuf;
	assert(SUCCEEDED(dxgiSwapChain1->GetBuffer(0, __uuidof(decltype(backBuf)::InterfaceType), reinterpret_cast<void**>(backBuf.GetAddressOf()))));

	ComPtr<ID3D11RenderTargetView> backBufRtv;
	assert(SUCCEEDED(d3dDevice->CreateRenderTargetView(backBuf.Get(), nullptr, backBufRtv.GetAddressOf())));

	auto* const monoDomain = mono_jit_init("leopph");
	assert(monoDomain);

	auto* const monoAssembly = mono_domain_assembly_open(monoDomain, "leopph_managed.dll");
	assert(monoAssembly);

	auto* const monoImage = mono_assembly_get_image(monoAssembly);
	assert(monoImage);

	auto* const monoMethodDesc = mono_method_desc_new("leopph:Printer:Print", true);
	assert(monoMethodDesc);

	auto* const monoKlass = mono_class_from_name(monoImage, "leopph", "Printer");
	assert(monoKlass);

	auto* const monoMethod = mono_method_desc_search_in_class(monoMethodDesc, monoKlass);
	assert(monoMethod);

	MonoObject* exception;
	mono_runtime_invoke(monoMethod, nullptr, nullptr, &exception);
	assert(!exception);

	ShowWindow(hwnd, SW_SHOWDEFAULT);

	while (true)
	{
		MSG msg;

		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return static_cast<int>(msg.wParam);
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		FLOAT clearColor[]{ 0.5f, 0, 1, 1 };
		d3dDeviceContext->ClearRenderTargetView(backBufRtv.Get(), clearColor);
		d3dDeviceContext->OMSetRenderTargets(1, backBufRtv.GetAddressOf(), nullptr);

		dxgiSwapChain1->Present(0, 0);
	}
}


LRESULT CALLBACK window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}