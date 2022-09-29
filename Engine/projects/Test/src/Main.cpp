#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

#ifdef NDEBUG
#include "CubeVertexShader.h"
#include "CubePixelShader.h"
#else
#include "CubeVertexShaderDebug.h"
#include "CubePixelShaderDebug.h"
#endif

#include <array>
#include <cassert>
#include <cstdio>

#define DllImport __declspec(dllimport)

using Microsoft::WRL::ComPtr;

using Vec3f = std::array<float, 3>;

auto constexpr WINDOW_WIDTH = 1280;
auto constexpr WINDOW_HEIGHT = 720;
auto constexpr MAX_OBJECTS = 10;

extern "C"
{
	DllImport Vec3f const* get_positions();
	DllImport std::size_t get_num_positions();
}

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
		.hCursor = LoadCursorW(nullptr, IDC_ARROW),
		.hbrBackground = nullptr,
		.lpszMenuName = nullptr,
		.lpszClassName = L"MyWindow",
	};

	if (!RegisterClassW(&wndClass))
	{
		std::fprintf(stderr, "Failed to register window class.\n");
		return -1;
	}

	auto const hwnd = CreateWindowExW(0, wndClass.lpszClassName, L"MyWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, wndClass.hInstance, nullptr);

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

	D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{D3D_FEATURE_LEVEL_11_0};

	auto hresult = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceCreationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, d3dDevice.GetAddressOf(), nullptr, d3dDeviceContext.GetAddressOf());
	assert(SUCCEEDED(hresult));

#ifndef NDEBUG
	ComPtr<ID3D11Debug> d3dDebug;
	hresult = d3dDevice.As(&d3dDebug);
	assert(SUCCEEDED(hresult));

	ComPtr<ID3D11InfoQueue> d3dInfoQueue;
	hresult = d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue);
	assert(SUCCEEDED(hresult));

	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

	ComPtr<IDXGIDevice> dxgiDevice;
	hresult = d3dDevice.As(&dxgiDevice);
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

	ComPtr<IDXGISwapChain1> dxgiSwapChain1;
	hresult = dxgiFactory2->CreateSwapChainForHwnd(d3dDevice.Get(), hwnd, &swapChainDesc1, nullptr, nullptr, dxgiSwapChain1.GetAddressOf());
	assert(SUCCEEDED(hresult));

	ComPtr<ID3D11Texture2D> backBuf;
	hresult = dxgiSwapChain1->GetBuffer(0, __uuidof(decltype(backBuf)::InterfaceType), reinterpret_cast<void**>(backBuf.GetAddressOf()));
	assert(SUCCEEDED(hresult));

	ComPtr<ID3D11RenderTargetView> backBufRtv;
	hresult = d3dDevice->CreateRenderTargetView(backBuf.Get(), nullptr, backBufRtv.GetAddressOf());
	assert(SUCCEEDED(hresult));

	ComPtr<ID3D11VertexShader> vertexShader;
	hresult = d3dDevice->CreateVertexShader(gCubeVertShader, ARRAYSIZE(gCubeVertShader), nullptr, vertexShader.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);

	ComPtr<ID3D11PixelShader> pixelShader;
	hresult = d3dDevice->CreatePixelShader(gCubePixShader, ARRAYSIZE(gCubePixShader), nullptr, pixelShader.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

	D3D11_INPUT_ELEMENT_DESC constexpr inputElementDescs[]
	{
		{
			.SemanticName = "VERTEXPOS",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		{
			.SemanticName = "OBJECTPOS",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 1,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
			.InstanceDataStepRate = 1
		}
	};

	ComPtr<ID3D11InputLayout> inputLayout;
	hresult = d3dDevice->CreateInputLayout(inputElementDescs, ARRAYSIZE(inputElementDescs), gCubeVertShader, ARRAYSIZE(gCubeVertShader), inputLayout.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->IASetInputLayout(inputLayout.Get());
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float constexpr cubeVertexData[]
	{
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
	};

	UINT constexpr vertexStride = 3 * sizeof(float);
	UINT constexpr vertexOffset = 0;

	D3D11_BUFFER_DESC constexpr cubeVertexBufferDesc
	{
		.ByteWidth = sizeof cubeVertexData,
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA const cubeVertexSubresourceData
	{
		.pSysMem = cubeVertexData,
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};

	ComPtr<ID3D11Buffer> vertexBuffer;
	hresult = d3dDevice->CreateBuffer(&cubeVertexBufferDesc, &cubeVertexSubresourceData, vertexBuffer.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);

	D3D11_BUFFER_DESC constexpr cubePositionBufferDesc
	{
		.ByteWidth = MAX_OBJECTS * 3 * sizeof(float),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	UINT constexpr positionStride = 3 * sizeof(float);
	UINT constexpr positionOffset = 0;

	ComPtr<ID3D11Buffer> positionBuffer;
	hresult = d3dDevice->CreateBuffer(&cubePositionBufferDesc, nullptr, positionBuffer.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->IASetVertexBuffers(1, 1, positionBuffer.GetAddressOf(), &positionStride, &positionOffset);

	unsigned constexpr indexData[]
	{
		// Top face
		0, 1, 2,
		2, 3, 0,
		// Bottom face
		4, 5, 6,
		6, 7, 4,
		// Front face
		2, 6, 7,
		7, 3, 2,
		// Back face
		0, 1, 5,
		5, 4, 0,
		// Right face
		0, 3, 7,
		7, 4, 0,
		// Left face
		2, 1, 5,
		5, 6, 2
	};

	D3D11_BUFFER_DESC constexpr indexBufferDesc
	{
		.ByteWidth = sizeof indexData,
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
		.CPUAccessFlags = 0,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	D3D11_SUBRESOURCE_DATA const indexSubresourceData
	{
		.pSysMem = indexData,
		.SysMemPitch = 0,
		.SysMemSlicePitch = 0
	};

	ComPtr<ID3D11Buffer> indexBuffer;
	hresult = d3dDevice->CreateBuffer(&indexBufferDesc, &indexSubresourceData, indexBuffer.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	D3D11_BUFFER_DESC constexpr cbufferDesc
	{
		.ByteWidth = 16 * sizeof(float),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	ComPtr<ID3D11Buffer> cbuffer;
	hresult = d3dDevice->CreateBuffer(&cbufferDesc, nullptr, cbuffer.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->VSSetConstantBuffers(0, 1, cbuffer.GetAddressOf());

	D3D11_RASTERIZER_DESC constexpr rasterizerDesc
	{
		.FillMode = D3D11_FILL_SOLID,
		.CullMode = D3D11_CULL_NONE,
		.FrontCounterClockwise = FALSE,
		.DepthBias = 0,
		.DepthBiasClamp = 0,
		.SlopeScaledDepthBias = 0,
		.DepthClipEnable = TRUE,
		.ScissorEnable = FALSE,
		.MultisampleEnable = FALSE,
		.AntialiasedLineEnable = FALSE
	};

	ComPtr<ID3D11RasterizerState> rasterizerState;
	hresult = d3dDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->RSSetState(rasterizerState.Get());

	D3D11_VIEWPORT constexpr viewPort
	{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = 1280,
		.Height = 720,
		.MinDepth = 0,
		.MaxDepth = 1
	};
	d3dDeviceContext->RSSetViewports(1, &viewPort);

	auto viewMat = DirectX::XMMatrixLookAtLH({0, 0, 0}, {0, 0, 1}, {0, 1, 0});
	auto projMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90), static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.3f, 100.f);
	auto viewProjMat = viewMat * projMat;

	D3D11_MAPPED_SUBRESOURCE mappedCbuffer;
	d3dDeviceContext->Map(cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCbuffer);
	DirectX::XMStoreFloat4x4(static_cast<DirectX::XMFLOAT4X4*>(mappedCbuffer.pData), viewProjMat);
	d3dDeviceContext->Unmap(cbuffer.Get(), 0);

	auto* const monoDomain = mono_jit_init("leopph");
	assert(monoDomain);

	auto* const monoAssembly = mono_domain_assembly_open(monoDomain, "leopph_managed.dll");
	assert(monoAssembly);

	auto* const monoImage = mono_assembly_get_image(monoAssembly);
	assert(monoImage);

	auto* const monoMethodDesc = mono_method_desc_new("Test:DoTest", false);
	assert(monoMethodDesc);

	auto* const monoKlass = mono_class_from_name(monoImage, "", "Test");
	assert(monoKlass);

	auto* const monoMethod = mono_method_desc_search_in_class(monoMethodDesc, monoKlass);
	assert(monoMethod);

	MonoObject* exception;
	mono_runtime_invoke(monoMethod, nullptr, nullptr, &exception);

	if (exception)
	{
		mono_print_unhandled_exception(exception);
	}

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

		auto numObj = static_cast<UINT>(get_num_positions());

		if (numObj == 0)
		{
			continue;
		}

		numObj = numObj > MAX_OBJECTS ? MAX_OBJECTS : numObj;

		D3D11_MAPPED_SUBRESOURCE mappedPosBuffer;
		d3dDeviceContext->Map(positionBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPosBuffer);
		auto* mappedPosBufferData = static_cast<DirectX::XMFLOAT3*>(mappedPosBuffer.pData);

		auto const* positions = get_positions();

		for (int i = 0; i < MAX_OBJECTS; i++)
		{
			DirectX::XMFLOAT3 posData{positions[i].data()};
			DirectX::XMStoreFloat3(mappedPosBufferData + i, DirectX::XMLoadFloat3({&posData}));
		}

		d3dDeviceContext->Unmap(positionBuffer.Get(), 0);

		FLOAT clearColor[]{0.5f, 0, 1, 1};
		d3dDeviceContext->ClearRenderTargetView(backBufRtv.Get(), clearColor);
		d3dDeviceContext->OMSetRenderTargets(1, backBufRtv.GetAddressOf(), nullptr);

		d3dDeviceContext->DrawIndexedInstanced(ARRAYSIZE(indexData), numObj, 0, 0, 0);

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