#include "Renderer.hpp"

#include "Platform.hpp"
#include "Entity.hpp"

#ifdef NDEBUG
#include "shaders/cinclude/BlinnPhongVertShadBin.h"
#include "shaders/cinclude/BlinnPhongPixShadBin.h"
#else
#include "shaders/cinclude/BlinnPhongVertShadBinDebug.h"
#include "shaders/cinclude/BlinnPhongPixShadBinDebug.h"
#endif

#include <DirectXMath.h>
#include <dxgi1_2.h>
#include <dxgi1_5.h>

#include <wrl/client.h>

#include <cassert>
#include <functional>
#include <utility>

using Microsoft::WRL::ComPtr;


namespace leopph::rendering {
	struct LightData {
		Vector3 color;
		f32 intensity;
	};

	struct DirLightData : LightData {
		Vector3 direction;
	};

	struct LightBufferData {
		DirLightData dirLightData;
		bool calcDirLight;
	};


	struct Resources {
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;
		ComPtr<IDXGISwapChain1> swapChain;
		ComPtr<ID3D11RenderTargetView> swapChainRtv;
		ComPtr<ID3D11Texture2D> renderTexture;
		ComPtr<ID3D11RenderTargetView> renderTextureRtv;
		ComPtr<ID3D11ShaderResourceView> renderTextureSrv;
		ComPtr<ID3D11VertexShader> cubeVertShader;
		ComPtr<ID3D11PixelShader> cubePixShader;
		ComPtr<ID3D11Buffer> instanceBuffer;
		ComPtr<ID3D11Buffer> cbuffer;
		ComPtr<ID3D11Buffer> lightBuffer;
	};


	namespace {
		UINT constexpr INSTANCE_BUFFER_ELEMENT_SIZE{ sizeof(DirectX::XMFLOAT4X4) };
		UINT constexpr VERTEX_BUFFER_SLOT{ 0 };
		UINT constexpr INSTANCE_BUFFER_SLOT{ 1 };

		Resources* gResources{ nullptr };
		UINT gPresentFlags{ 0 };
		UINT gSwapChainFlags{ 0 };
		Extent2D<u32> gGameRes;
		f32 gGameAspect;
		UINT gInstanceBufferElementCapacity;
		UINT gIndexCount;
		u32 gSyncInterval{ 0 };
		std::vector<CubeModel const*> gCubeModels;
		std::vector<DirectionalLight const*> gDirLights;
		std::vector<SpotLight const*> gSpotLights;
		std::vector<PointLight const*> gPointLights;


		void RecreateRenderTextureAndViews(u32 const width, u32 const height) {
			D3D11_TEXTURE2D_DESC const texDesc
			{
				.Width = width,
				.Height = height,
				.MipLevels = 1,
				.ArraySize = 1,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.SampleDesc
				{
					.Count = 1,
					.Quality = 0
				},
				.Usage = D3D11_USAGE_DEFAULT,
				.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
				.CPUAccessFlags = 0,
				.MiscFlags = 0
			};

			auto hresult = gResources->device->CreateTexture2D(&texDesc, nullptr, gResources->renderTexture.ReleaseAndGetAddressOf());
			assert(SUCCEEDED(hresult));

			D3D11_RENDER_TARGET_VIEW_DESC const rtvDesc
			{
				.Format = texDesc.Format,
				.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
				.Texture2D
				{
					.MipSlice = 0
				}
			};

			hresult = gResources->device->CreateRenderTargetView(gResources->renderTexture.Get(), &rtvDesc, gResources->renderTextureRtv.ReleaseAndGetAddressOf());
			assert(SUCCEEDED(hresult));

			D3D11_SHADER_RESOURCE_VIEW_DESC const srvDesc
			{
				.Format = texDesc.Format,
				.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
				.Texture2D =
				{
					.MostDetailedMip = 0,
					.MipLevels = 1
				}
			};

			hresult = gResources->device->CreateShaderResourceView(gResources->renderTexture.Get(), &srvDesc, gResources->renderTextureSrv.ReleaseAndGetAddressOf());
			assert(SUCCEEDED(hresult));
		}


		void on_window_resize(Extent2D<u32> const size) {
			if (size.width == 0 || size.height == 0) {
				return;
			}

			gResources->swapChainRtv.Reset();
			gResources->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, gSwapChainFlags);

			ComPtr<ID3D11Texture2D> backBuf;
			HRESULT hresult = gResources->swapChain->GetBuffer(0, __uuidof(decltype(backBuf)::InterfaceType), reinterpret_cast<void**>(backBuf.GetAddressOf()));
			assert(SUCCEEDED(hresult));

			hresult = gResources->device->CreateRenderTargetView(backBuf.Get(), nullptr, gResources->swapChainRtv.GetAddressOf());
			assert(SUCCEEDED(hresult));
		}
	}


	bool InitRenderer() {
		gResources = new Resources{};

#ifdef NDEBUG
		UINT constexpr deviceCreationFlags = 0;
#else
		UINT constexpr deviceCreationFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{ D3D_FEATURE_LEVEL_11_0 };

		HRESULT hresult = D3D11CreateDevice(nullptr,
											D3D_DRIVER_TYPE_HARDWARE,
											nullptr,
											deviceCreationFlags,
											requestedFeatureLevels,
											1,
											D3D11_SDK_VERSION,
											gResources->device.GetAddressOf(),
											nullptr,
											gResources->context.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create D3D device.", L"Error", MB_ICONERROR);
			return false;
		}

#ifndef NDEBUG
		ComPtr<ID3D11Debug> d3dDebug;
		hresult = gResources->device.As(&d3dDebug);

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to get ID3D11Debug interface.", L"Error", MB_ICONERROR);
			return false;
		}

		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		hresult = d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue);

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to get ID3D11InfoQueue interface.", L"Error", MB_ICONERROR);
			return false;
		}

		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

		ComPtr<IDXGIDevice> dxgiDevice;
		hresult = gResources->device.As(&dxgiDevice);

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to query IDXGIDevice interface.", L"Error", MB_ICONERROR);
			return false;
		}

		ComPtr<IDXGIAdapter> dxgiAdapter;
		hresult = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to get IDXGIAdapter.", L"Error", MB_ICONERROR);
			return false;
		}

		ComPtr<IDXGIFactory2> dxgiFactory2;
		hresult = dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory2)::InterfaceType),
										 reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to query IDXGIFactory2 interface.", L"Error", MB_ICONERROR);
			return false;
		}

		ComPtr<IDXGIFactory5> dxgiFactory5;
		hresult = dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory5)::InterfaceType),
										 reinterpret_cast<void**>(dxgiFactory5.GetAddressOf()));

		if (SUCCEEDED(hresult)) {
			BOOL allowTearing{};
			dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof allowTearing);

			if (allowTearing) {
				gSwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				gPresentFlags |= DXGI_PRESENT_ALLOW_TEARING;
			}
		}

		DXGI_SWAP_CHAIN_DESC1 const swapChainDesc1
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
			.Flags = gSwapChainFlags
		};

		hresult = dxgiFactory2->CreateSwapChainForHwnd(gResources->device.Get(),
													   platform::get_hwnd(),
													   &swapChainDesc1,
													   nullptr,
													   nullptr,
													   gResources->swapChain.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create swapchain.", L"Error", MB_ICONERROR);
			return false;
		}

		dxgiFactory2->MakeWindowAssociation(platform::get_hwnd(), DXGI_MWA_NO_WINDOW_CHANGES);

		RecreateRenderTextureAndViews(platform::get_window_current_client_area_size().width, platform::get_window_current_client_area_size().height);

		ComPtr<ID3D11Texture2D> backBuf;
		hresult = gResources->swapChain->GetBuffer(0,
												   __uuidof(decltype(backBuf)::InterfaceType),
												   reinterpret_cast<void**>(backBuf.GetAddressOf()));

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to get backbuffer.", L"Error", MB_ICONERROR);
			return false;
		}

		hresult = gResources->device->CreateRenderTargetView(backBuf.Get(),
															 nullptr,
															 gResources->swapChainRtv.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create backbuffer RTV.", L"Error", MB_ICONERROR);
			return false;
		}

		hresult = gResources->device->CreateVertexShader(gBlinnPhongVertShadBin,
														 ARRAYSIZE(gBlinnPhongVertShadBin),
														 nullptr,
														 gResources->cubeVertShader.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create cube vertex shader.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->VSSetShader(gResources->cubeVertShader.Get(), nullptr, 0);

		hresult = gResources->device->CreatePixelShader(gBlinnPhongPixShadBin,
														ARRAYSIZE(gBlinnPhongPixShadBin),
														nullptr,
														gResources->cubePixShader.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create cube pixel shader.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->PSSetShader(gResources->cubePixShader.Get(), nullptr, 0);

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
		hresult = gResources->device->CreateInputLayout(inputElementDescs,
														ARRAYSIZE(inputElementDescs),
														gBlinnPhongVertShadBin,
														ARRAYSIZE(gBlinnPhongVertShadBin),
														inputLayout.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create cube input layout.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->IASetInputLayout(inputLayout.Get());
		gResources->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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
		hresult = gResources->device->CreateBuffer(&cubeVertexBufferDesc,
												   &cubeVertexSubresourceData,
												   vertexBuffer.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create cube vertex buffer.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->IASetVertexBuffers(VERTEX_BUFFER_SLOT,
												1,
												vertexBuffer.GetAddressOf(),
												&vertexStride,
												&vertexOffset);

		UINT const instanceBufferElementCapacity{ 1 };

		D3D11_BUFFER_DESC const desc
		{
			.ByteWidth = instanceBufferElementCapacity * INSTANCE_BUFFER_ELEMENT_SIZE,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = gResources->device->CreateBuffer(&desc, nullptr, gResources->instanceBuffer.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create cube instance buffer.", L"Error", MB_ICONERROR);
			return false;
		}

		UINT constexpr instanceBufferOffset{ 0 };

		gResources->context->IASetVertexBuffers(INSTANCE_BUFFER_SLOT,
												1,
												gResources->instanceBuffer.GetAddressOf(),
												&INSTANCE_BUFFER_ELEMENT_SIZE,
												&instanceBufferOffset);

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

		gIndexCount = ARRAYSIZE(indexData);

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
		hresult = gResources->device->CreateBuffer(&indexBufferDesc,
												   &indexSubresourceData,
												   indexBuffer.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create cube index buffer.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		D3D11_BUFFER_DESC constexpr cbufferDesc
		{
			.ByteWidth = 16 * sizeof(float),
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = gResources->device->CreateBuffer(&cbufferDesc,
												   nullptr,
												   gResources->cbuffer.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create cube constant buffer.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->VSSetConstantBuffers(0, 1, gResources->cbuffer.GetAddressOf());

		D3D11_BUFFER_DESC constexpr lightBufferDesc{
			.ByteWidth = sizeof(LightBufferData) + 16 - sizeof(LightBufferData) % 16,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = gResources->device->CreateBuffer(&lightBufferDesc, nullptr, gResources->lightBuffer.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create light constant buffer.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->PSSetConstantBuffers(0, 1, gResources->lightBuffer.GetAddressOf());

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
		hresult = gResources->device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());

		if (FAILED(hresult)) {
			MessageBoxW(platform::get_hwnd(), L"Failed to create rasterizer state.", L"Error", MB_ICONERROR);
			return false;
		}

		gResources->context->RSSetState(rasterizerState.Get());

		gGameRes = platform::get_window_current_client_area_size();
		gGameAspect = static_cast<f32>(gGameRes.width) / static_cast<f32>(gGameRes.height);

		platform::OnWindowSize.add_handler(&on_window_resize);
		return true;
	}


	void CleanupRenderer() {
		delete gResources;
	}


	bool DrawCamera(Camera const* const cam) {
		if (gCubeModels.empty()) {
			return true;
		}

		auto const& camViewport{ cam->GetViewport() };
		D3D11_VIEWPORT const viewport{
			.TopLeftX = static_cast<FLOAT>(gGameRes.width) * camViewport.position.x,
			.TopLeftY = static_cast<FLOAT>(gGameRes.height) * camViewport.position.y,
			.Width = static_cast<FLOAT>(gGameRes.width) * camViewport.extent.width,
			.Height = static_cast<FLOAT>(gGameRes.height) * camViewport.extent.height,
			.MinDepth = 0,
			.MaxDepth = 1
		};

		gResources->context->RSSetViewports(1, &viewport);

		DirectX::XMFLOAT3 const camPos{ cam->GetTransform().GetWorldPosition().get_data() };
		DirectX::XMFLOAT3 const camForward{ cam->GetTransform().GetForwardAxis().get_data() };
		DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&camPos), DirectX::XMLoadFloat3(&camForward), { 0, 1, 0 });

		DirectX::XMMATRIX projMat;

		if (cam->GetType() == Camera::Type::Perspective) {
			auto const fovHorizRad = DirectX::XMConvertToRadians(cam->GetPerspectiveFov());
			auto const fovVertRad = 2.0f * std::atanf(std::tanf(fovHorizRad / 2.0f) / gGameAspect);
			projMat = DirectX::XMMatrixPerspectiveFovLH(fovVertRad, gGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane());
		}
		else {
			projMat = DirectX::XMMatrixOrthographicLH(cam->GetOrthographicSize(), cam->GetOrthographicSize() / gGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane());
		}

		D3D11_MAPPED_SUBRESOURCE mappedCbuffer;
		gResources->context->Map(gResources->cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCbuffer);

		DirectX::XMStoreFloat4x4(static_cast<DirectX::XMFLOAT4X4*>(mappedCbuffer.pData), viewMat * projMat);
		gResources->context->Unmap(gResources->cbuffer.Get(), 0);

		D3D11_MAPPED_SUBRESOURCE mappedLightBuffer;
		gResources->context->Map(gResources->lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedLightBuffer);
		auto const lightBufferData{ static_cast<LightBufferData*>(mappedLightBuffer.pData) };
		lightBufferData->calcDirLight = !gDirLights.empty();
		if (!gDirLights.empty()) {
			lightBufferData->dirLightData.color = gDirLights[0]->GetColor();
			lightBufferData->dirLightData.direction = gDirLights[0]->get_direction();
			lightBufferData->dirLightData.intensity = gDirLights[0]->GetIntensity();
		}
		gResources->context->Unmap(gResources->lightBuffer.Get(), 0);


		if (gCubeModels.size() > gInstanceBufferElementCapacity) {
			gInstanceBufferElementCapacity = static_cast<UINT>(gCubeModels.size());

			D3D11_BUFFER_DESC const desc
			{
				.ByteWidth = gInstanceBufferElementCapacity * INSTANCE_BUFFER_ELEMENT_SIZE,
				.Usage = D3D11_USAGE_DYNAMIC,
				.BindFlags = D3D11_BIND_VERTEX_BUFFER,
				.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
				.MiscFlags = 0,
				.StructureByteStride = 0
			};

			auto const hresult = gResources->device->CreateBuffer(&desc, nullptr, gResources->instanceBuffer.ReleaseAndGetAddressOf());
			if (FAILED(hresult)) {
				MessageBoxW(nullptr, L"Failed to resize cube instance buffer.", L"Error", MB_ICONERROR);
				return false;
			}

			UINT constexpr instanceBufferOffset{ 0 };
			gResources->context->IASetVertexBuffers(INSTANCE_BUFFER_SLOT, 1, gResources->instanceBuffer.GetAddressOf(), &INSTANCE_BUFFER_ELEMENT_SIZE, &instanceBufferOffset);
		}

		D3D11_MAPPED_SUBRESOURCE mappedInstanceBuffer;
		gResources->context->Map(gResources->instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedInstanceBuffer);
		DirectX::XMFLOAT4X4* mappedInstanceBufferData{ static_cast<DirectX::XMFLOAT4X4*>(mappedInstanceBuffer.pData) };

		for (std::size_t i = 0; i < gCubeModels.size(); i++) {
			DirectX::XMFLOAT4X4 modelMat{ gCubeModels[i]->GetTransform().GetModelMatrix().get_data() };
			DirectX::XMStoreFloat4x4(mappedInstanceBufferData + i, DirectX::XMLoadFloat4x4(&modelMat));
		}

		gResources->context->Unmap(gResources->instanceBuffer.Get(), 0);

		FLOAT clearColor[]{ 0.5f, 0, 1, 1 };
		gResources->context->ClearRenderTargetView(gResources->renderTextureRtv.Get(), clearColor);
		gResources->context->OMSetRenderTargets(1, gResources->renderTextureRtv.GetAddressOf(), nullptr);

		gResources->context->DrawIndexedInstanced(gIndexCount, static_cast<UINT>(gCubeModels.size()), 0, 0, 0);
		return true;
	}


	bool DrawGame() {
		auto ret{ true };
		for (auto const* const cam : Camera::GetAllInstances()) {
			ret = ret && DrawCamera(cam);
		}
		return ret;
	}


	Extent2D<u32> GetGameResolution() {
		return gGameRes;
	}


	void SetGameResolution(Extent2D<u32> resolution) {
		gGameRes = resolution;
		gGameAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
		RecreateRenderTextureAndViews(gGameRes.width, gGameRes.height);
	}


	ID3D11ShaderResourceView* GetGameFrame() {
		return gResources->renderTextureSrv.Get();
	}


	f32 GetGameAspectRatio() {
		return gGameAspect;
	}


	void BindAndClearSwapChain() {
		FLOAT clearColor[]{ 0, 0, 0, 1 };
		gResources->context->ClearRenderTargetView(gResources->swapChainRtv.Get(), clearColor);
		gResources->context->OMSetRenderTargets(1, gResources->swapChainRtv.GetAddressOf(), nullptr);
	}


	void Present() {
		gResources->swapChain->Present(gSyncInterval, gSyncInterval ? gPresentFlags & ~DXGI_PRESENT_ALLOW_TEARING : gPresentFlags);
	}


	u32 GetSyncInterval() {
		return gSyncInterval;
	}


	void SetSyncInterval(u32 const interval) {
		gSyncInterval = interval;
	}


	void RegisterCubeModel(CubeModel const* const cubeModel) {
		gCubeModels.emplace_back(cubeModel);
	}


	void UnregisterCubeModel(CubeModel const* const cubeModel) {
		std::erase(gCubeModels, cubeModel);
	}


	ID3D11Device* GetDevice() {
		return gResources->device.Get();
	}


	ID3D11DeviceContext* GetImmediateContext() {
		return gResources->context.Get();
	}


	auto RegisterDirLight(DirectionalLight const* dirLight) -> void {
		gDirLights.emplace_back(dirLight);
	}

	auto UnregisterDirLight(DirectionalLight const* dirLight) -> void {
		std::erase(gDirLights, dirLight);
	}

	auto RegisterSpotLight(SpotLight const* spotLight) -> void {
		gSpotLights.emplace_back(spotLight);
	}

	auto UnregisterSpotLight(SpotLight const* spotLight) -> void {
		std::erase(gSpotLights, spotLight);
	}

	auto RegisterPointLight(PointLight const* pointLight) -> void {
		gPointLights.emplace_back(pointLight);
	}

	auto UnregisterPointLight(PointLight const* pointLight) -> void {
		std::erase(gPointLights, pointLight);
	}
}