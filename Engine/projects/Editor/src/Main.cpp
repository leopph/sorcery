#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <d3d11.h>
#include <dxgi1_2.h>

#include <wrl/client.h>

#include <format>

using Microsoft::WRL::ComPtr;


LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT IMGUI_IMPL_API ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void create_render_target_view(ComPtr<ID3D11Device> const& device, ComPtr<IDXGISwapChain1> const& swapChain, ComPtr<ID3D11RenderTargetView>& rtv);


struct Data
{
	ComPtr<IDXGISwapChain1> dxgiSwapChain1;
	ComPtr<ID3D11RenderTargetView> backBufRtv;
	ComPtr<ID3D11Device> d3dDevice;
};



int APIENTRY wWinMain(HINSTANCE const instance, [[maybe_unused]] HINSTANCE const prevInstance, [[maybe_unused]] wchar_t* const cmdLine, [[maybe_unused]] int const showCmd)
{
	WNDCLASSW wndClass{};
	wndClass.hInstance = instance;
	wndClass.lpfnWndProc = window_proc;
	wndClass.lpszClassName = L"LeopphEditor";

	if (!RegisterClassW(&wndClass))
	{
		MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR);
		return -1;
	}

	auto const hwnd = CreateWindowExW(0, wndClass.lpszClassName, L"Leopph Editor", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);

	if (!hwnd)
	{
		MessageBoxW(nullptr, L"Failed to create window.", L"Error", MB_ICONERROR);
		return -1;
	}

	ShowWindow(hwnd, showCmd);

	auto* const appData = new Data;
	SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(appData));

	ComPtr<ID3D11DeviceContext> d3dDeviceContext;

	auto constexpr featureLevel = D3D_FEATURE_LEVEL_11_0;
	UINT deviceCreationFlags = 0;
	#ifndef NDEBUG
	deviceCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceCreationFlags, &featureLevel, 1, D3D11_SDK_VERSION, appData->d3dDevice.GetAddressOf(), nullptr, d3dDeviceContext.GetAddressOf());

	#ifndef NDEBUG
	ComPtr<ID3D11Debug> debug;

	{
		appData->d3dDevice.As(&debug);

		ComPtr<ID3D11InfoQueue> infoQueue;
		debug.As<ID3D11InfoQueue>(&infoQueue);

		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
	}
	#endif

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
		.Scaling = DXGI_SCALING_NONE,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
		.Flags = 0
	};

	ComPtr<IDXGIDevice> dxgiDevice;
	appData->d3dDevice.As(&dxgiDevice);

	ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

	ComPtr<IDXGIFactory2> dxgiFactory2;
	dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory2)::InterfaceType), reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));

	dxgiFactory2->CreateSwapChainForHwnd(appData->d3dDevice.Get(), hwnd, &swapChainDesc1, nullptr, nullptr, appData->dxgiSwapChain1.GetAddressOf());

	dxgiFactory2->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES);

	create_render_target_view(appData->d3dDevice, appData->dxgiSwapChain1, appData->backBufRtv);

	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	static_cast<void>(io);

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(appData->d3dDevice.Get(), d3dDeviceContext.Get());

	while (true)
	{
		MSG msg;

		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				ImGui_ImplDX11_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();
				return static_cast<int>(msg.wParam);
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open"))
				{
					MessageBoxW(hwnd, L"Placeholder", L"Placeholder", 0);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		ImGui::Render();

		FLOAT constexpr clearColor[]{0.5f, 0, 1.f, 1.f};
		d3dDeviceContext->ClearRenderTargetView(appData->backBufRtv.Get(), clearColor);
		d3dDeviceContext->OMSetRenderTargets(1, appData->backBufRtv.GetAddressOf(), nullptr);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		appData->dxgiSwapChain1->Present(1, 0);
	}
}



LRESULT CALLBACK window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

	if (msg == WM_DESTROY)
	{
		delete reinterpret_cast<Data*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		PostQuitMessage(0);
		return 0;
	}

	if (msg == WM_SIZE)
	{
		if (auto* const appData = reinterpret_cast<Data*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)); wparam != SIZE_MINIMIZED && appData && appData->dxgiSwapChain1)
		{
			appData->backBufRtv.Reset();
			appData->dxgiSwapChain1->ResizeBuffers(0, LOWORD(lparam), HIWORD(lparam), DXGI_FORMAT_UNKNOWN, 0);
			create_render_target_view(appData->d3dDevice, appData->dxgiSwapChain1, appData->backBufRtv);
		}
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}



void create_render_target_view(ComPtr<ID3D11Device> const& device, ComPtr<IDXGISwapChain1> const& swapChain, ComPtr<ID3D11RenderTargetView>& rtv)
{
	ComPtr<ID3D11Texture2D> backBuf;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuf.GetAddressOf()));
	device->CreateRenderTargetView(backBuf.Get(), nullptr, rtv.GetAddressOf());
}
