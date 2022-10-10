#ifndef NDEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <DirectXMath.h>

#include "Managed.hpp"

#ifdef NDEBUG
#include "CubeVertexShader.h"
#include "CubePixelShader.h"
#else
#include "CubeVertexShaderDebug.h"
#include "CubePixelShaderDebug.h"
#endif

#include <Camera.hpp>
#include <Entity.hpp>
#include <Input.hpp>
#include <Managed.hpp>
#include <Time.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>

using Microsoft::WRL::ComPtr;

int constexpr WIDTH{1280};
int constexpr HEIGHT{720};
float constexpr ASPECT_RATIO{static_cast<float>(WIDTH) / HEIGHT};
int constexpr MAX_NODES{10};


LRESULT CALLBACK window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
{
	if (msg == WM_CLOSE)
	{
		DestroyWindow(hwnd);
		PostQuitMessage(0);
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}


int main()
{
#ifndef NDEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

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

	RECT windowRect
	{
		.left = 0,
		.top = 0,
		.right = WIDTH,
		.bottom = HEIGHT
	};

	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

	auto const hwnd = CreateWindowExW(0, wndClass.lpszClassName, L"MyWindow", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, wndClass.hInstance, nullptr);

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
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		{
			.SemanticName = "MODELMATRIX",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 1,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
			.InstanceDataStepRate = 1
		},
		{
			.SemanticName = "MODELMATRIX",
			.SemanticIndex = 1,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 1,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
			.InstanceDataStepRate = 1
		},
		{
			.SemanticName = "MODELMATRIX",
			.SemanticIndex = 2,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 1,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
			.InstanceDataStepRate = 1
		},
		{
			.SemanticName = "MODELMATRIX",
			.SemanticIndex = 3,
			.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
			.InputSlot = 1,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
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

	UINT constexpr cubeInstanceBufferElementSize = sizeof(DirectX::XMFLOAT4X4);

	D3D11_BUFFER_DESC constexpr cubeInstanceBufferDesc
	{
		.ByteWidth = MAX_NODES * cubeInstanceBufferElementSize,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	UINT constexpr cubeInstanceBufferOffset = 0;

	ComPtr<ID3D11Buffer> instanceBuffer;
	hresult = d3dDevice->CreateBuffer(&cubeInstanceBufferDesc, nullptr, instanceBuffer.GetAddressOf());
	assert(SUCCEEDED(hresult));
	d3dDeviceContext->IASetVertexBuffers(1, 1, instanceBuffer.GetAddressOf(), &cubeInstanceBufferElementSize, &cubeInstanceBufferOffset);

	unsigned constexpr indexData[]
	{
		// Top face
		0, 1, 2,
		2, 3, 0,
		// Bottom face
		7, 6, 5,
		5, 4, 7,
		// Front face
		2, 6, 7,
		7, 3, 2,
		// Back face
		0, 4, 5,
		5, 1, 0,
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
		.CullMode = D3D11_CULL_BACK,
		.FrontCounterClockwise = TRUE,
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
		.Width = WIDTH,
		.Height = HEIGHT,
		.MinDepth = 0,
		.MaxDepth = 1
	};
	d3dDeviceContext->RSSetViewports(1, &viewPort);
	
	leopph::initialize_managed_runtime();

	ShowWindow(hwnd, SW_SHOWDEFAULT);

	leopph::init_time();

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

		leopph::update_keyboard_state();

		for (auto const& entity : leopph::managedEntityInstances)
		{
			entity.tick();
		}

		if (leopph::entities.empty())
		{
			continue;
		}

		D3D11_MAPPED_SUBRESOURCE mappedInstanceBuffer;
		d3dDeviceContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedInstanceBuffer);
		DirectX::XMFLOAT4X4* mappedInstanceBufferData{static_cast<DirectX::XMFLOAT4X4*>(mappedInstanceBuffer.pData)};

		for (int i = 0; auto const& [id, entity] : leopph::entities)
		{
			if (i >= MAX_NODES)
			{
				break;
			}

			DirectX::XMFLOAT3 scaling{entity->get_scale().get_data()};
			DirectX::XMFLOAT4 rotation{reinterpret_cast<float const*>(&entity->get_rotation())};
			DirectX::XMFLOAT3 translation{entity->get_position().get_data()};

			DirectX::XMMATRIX const modelMat = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scaling)) *
				DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)) *
				DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&translation));

			DirectX::XMStoreFloat4x4(mappedInstanceBufferData + i, modelMat);

			i++;
		}

		d3dDeviceContext->Unmap(instanceBuffer.Get(), 0);

		DirectX::XMFLOAT3 const camPos{leopph::camPos.get_data()};
		DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&camPos), {0, 0, 1}, {0, 1, 0});
		DirectX::XMMATRIX projMat = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(90.0f / ASPECT_RATIO), ASPECT_RATIO, 0.3f, 100.f);

		D3D11_MAPPED_SUBRESOURCE mappedCbuffer;
		d3dDeviceContext->Map(cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCbuffer);
		DirectX::XMStoreFloat4x4(static_cast<DirectX::XMFLOAT4X4*>(mappedCbuffer.pData), viewMat * projMat);
		d3dDeviceContext->Unmap(cbuffer.Get(), 0);

		FLOAT clearColor[]{0.5f, 0, 1, 1};
		d3dDeviceContext->ClearRenderTargetView(backBufRtv.Get(), clearColor);
		d3dDeviceContext->OMSetRenderTargets(1, backBufRtv.GetAddressOf(), nullptr);

		d3dDeviceContext->DrawIndexedInstanced(ARRAYSIZE(indexData), static_cast<UINT>(std::min<std::size_t>(MAX_NODES, leopph::entities.size())), 0, 0, 0);

		dxgiSwapChain1->Present(0, 0);

		leopph::measure_time();
	}
}
