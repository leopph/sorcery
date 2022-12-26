#include "Renderer.hpp"

#include "Platform.hpp"
#include "Entity.hpp"
#include "Systems.hpp"
#include "Util.hpp"

#ifdef NDEBUG
#include "shaders/cinclude/BlinnPhongVertShadBin.h"
#include "shaders/cinclude/BlinnPhongPixShadBin.h"
#include "shaders/cinclude/ClearColorPsBin.h"
#include "shaders/cinclude/ClearColorVsBin.h"
#include "shaders/cinclude/PBRPs.h"
#else
#include "shaders/cinclude/BlinnPhongVertShadBinDebug.h"
#include "shaders/cinclude/BlinnPhongPixShadBinDebug.h"
#include "shaders/cinclude/ClearColorPsBinDebug.h"
#include "shaders/cinclude/ClearColorVsBinDebug.h"
#include "shaders/cinclude/PBRPsDebug.h"
#endif

#include <DirectXMath.h>

#include <cassert>
#include <functional>
#include <utility>

using Microsoft::WRL::ComPtr;


namespace leopph {
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

	struct CubeInstanceData {
		DirectX::XMFLOAT4X4 modelMat;
		DirectX::XMFLOAT3X3 normalMat;
	};

	struct MaterialData {
		Vector3 albedo;
		float metallic;
		float roughness;
		float ao;
	};

	struct CameraBufferData {
		Vector3 camPos;
	};


	namespace {
		FLOAT constexpr QUAD_VERTICES[]{
			-1, 1, -1, -1, 1, -1, 1, 1
		};
		UINT constexpr QUAD_INDICES[]{
			0, 1, 2, 2, 3, 0
		};
		UINT constexpr QUAD_VERT_BUF_STRIDE{ 8 };
		UINT constexpr QUAD_VERT_BUF_OFFSET{ 0 };

		FLOAT constexpr CUBE_VERTICES[]{
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		};
		UINT constexpr CUBE_INDICES[]{
			// Top face
			1, 4, 7,
			7, 10, 1,
			// Bottom face
			22, 19, 16,
			16, 13, 22,
			// Front face
			8, 20, 23,
			23, 11, 8,
			// Back face
			2, 14, 17,
			17, 5, 2,
			// Right face
			0, 9, 21,
			21, 12, 0,
			// Left face
			6, 3, 15,
			15, 18, 6
		};
	}


	void Renderer::RecreateRenderTextureAndViews(u32 const width, u32 const height) {
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

		auto hresult = mResources->device->CreateTexture2D(&texDesc, nullptr, mResources->renderTexture.ReleaseAndGetAddressOf());
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

		hresult = mResources->device->CreateRenderTargetView(mResources->renderTexture.Get(), &rtvDesc, mResources->renderTextureRtv.ReleaseAndGetAddressOf());
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

		hresult = mResources->device->CreateShaderResourceView(mResources->renderTexture.Get(), &srvDesc, mResources->renderTextureSrv.ReleaseAndGetAddressOf());
		assert(SUCCEEDED(hresult));
	}


	void Renderer::on_window_resize(Renderer* const self, Extent2D<u32> const size) {
		if (size.width == 0 || size.height == 0) {
			return;
		}

		self->mResources->swapChainRtv.Reset();
		self->mResources->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, self->mSwapChainFlags);

		ComPtr<ID3D11Texture2D> backBuf;
		HRESULT hresult = self->mResources->swapChain->GetBuffer(0, __uuidof(decltype(backBuf)::InterfaceType), reinterpret_cast<void**>(backBuf.GetAddressOf()));
		assert(SUCCEEDED(hresult));

		hresult = self->mResources->device->CreateRenderTargetView(backBuf.Get(), nullptr, self->mResources->swapChainRtv.GetAddressOf());
		assert(SUCCEEDED(hresult));
	}



	auto Renderer::StartUp() -> void {
		mResources = new Resources{};

#ifdef NDEBUG
		UINT constexpr deviceCreationFlags = 0;
#else
		UINT constexpr deviceCreationFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{ D3D_FEATURE_LEVEL_11_0 };

		HRESULT hresult = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceCreationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, mResources->device.GetAddressOf(), nullptr, mResources->context.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create D3D device." };
		}

#ifndef NDEBUG
		ComPtr<ID3D11Debug> d3dDebug;
		hresult = mResources->device.As(&d3dDebug);

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to get ID3D11Debug interface." };
		}

		ComPtr<ID3D11InfoQueue> d3dInfoQueue;
		hresult = d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue);

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to get ID3D11InfoQueue interface." };
		}

		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

		ComPtr<IDXGIDevice> dxgiDevice;
		hresult = mResources->device.As(&dxgiDevice);

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to query IDXGIDevice interface." };
		}

		ComPtr<IDXGIAdapter> dxgiAdapter;
		hresult = dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to get IDXGIAdapter." };
		}

		ComPtr<IDXGIFactory2> dxgiFactory2;
		hresult = dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory2)::InterfaceType), reinterpret_cast<void**>(dxgiFactory2.GetAddressOf()));

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to query IDXGIFactory2 interface." };
		}

		ComPtr<IDXGIFactory5> dxgiFactory5;
		hresult = dxgiAdapter->GetParent(__uuidof(decltype(dxgiFactory5)::InterfaceType), reinterpret_cast<void**>(dxgiFactory5.GetAddressOf()));

		if (SUCCEEDED(hresult)) {
			BOOL allowTearing{};
			dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof allowTearing);

			if (allowTearing) {
				mSwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				mPresentFlags |= DXGI_PRESENT_ALLOW_TEARING;
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
			.Flags = mSwapChainFlags
		};

		hresult = dxgiFactory2->CreateSwapChainForHwnd(mResources->device.Get(), gWindow.GetHandle(), &swapChainDesc1, nullptr, nullptr, mResources->swapChain.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create swapchain." };
		}

		dxgiFactory2->MakeWindowAssociation(gWindow.GetHandle(), DXGI_MWA_NO_WINDOW_CHANGES);

		RecreateRenderTextureAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);

		ComPtr<ID3D11Texture2D> backBuf;
		hresult = mResources->swapChain->GetBuffer(0, __uuidof(decltype(backBuf)::InterfaceType), reinterpret_cast<void**>(backBuf.GetAddressOf()));

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to get backbuffer." };
		}

		hresult = mResources->device->CreateRenderTargetView(backBuf.Get(), nullptr, mResources->swapChainRtv.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create backbuffer RTV." };
		}

		hresult = mResources->device->CreateVertexShader(gBlinnPhongVertShadBin, ARRAYSIZE(gBlinnPhongVertShadBin), nullptr, mResources->cubeVertShader.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create cube vertex shader." };
		}

		hresult = mResources->device->CreatePixelShader(gBlinnPhongPixShadBin, ARRAYSIZE(gBlinnPhongPixShadBin), nullptr, mResources->cubePixShader.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create cube pixel shader." };
		}

		D3D11_INPUT_ELEMENT_DESC constexpr cubeInputElementsDescs[]
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
				.SemanticName = "VERTEXNORMAL",
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
			},
						{
				.SemanticName = "NORMALMATRIX",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
				.InstanceDataStepRate = 1
			},
			{
				.SemanticName = "NORMALMATRIX",
				.SemanticIndex = 1,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
				.InstanceDataStepRate = 1
			},
			{
				.SemanticName = "NORMALMATRIX",
				.SemanticIndex = 2,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA,
				.InstanceDataStepRate = 1
			}
		};

		hresult = mResources->device->CreateInputLayout(cubeInputElementsDescs, ARRAYSIZE(cubeInputElementsDescs), gBlinnPhongVertShadBin, ARRAYSIZE(gBlinnPhongVertShadBin), mResources->cubeIa.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create cube input layout." };
		}

		mResources->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D11_BUFFER_DESC constexpr cubeVertBufDesc
		{
			.ByteWidth = sizeof CUBE_VERTICES,
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		D3D11_SUBRESOURCE_DATA const cubeVertBufInitData
		{
			.pSysMem = CUBE_VERTICES,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hresult = mResources->device->CreateBuffer(&cubeVertBufDesc, &cubeVertBufInitData, mResources->cubeVertBuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create cube vertex buffer." };
		}

		D3D11_BUFFER_DESC const cubeInstBufDesc
		{
			.ByteWidth = sizeof(CubeInstanceData),
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = mResources->device->CreateBuffer(&cubeInstBufDesc, nullptr, mResources->cubeInstBuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create cube instance buffer." };
		}

		D3D11_BUFFER_DESC constexpr cubeIndBufDesc
		{
			.ByteWidth = sizeof CUBE_INDICES,
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_INDEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		D3D11_SUBRESOURCE_DATA const cubeIndBufInitData
		{
			.pSysMem = CUBE_INDICES,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		hresult = mResources->device->CreateBuffer(&cubeIndBufDesc, &cubeIndBufInitData, mResources->cubeIndBuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create cube index buffer." };
		}

		D3D11_BUFFER_DESC constexpr cbufferDesc
		{
			.ByteWidth = 16 * sizeof(float),
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = mResources->device->CreateBuffer(&cbufferDesc, nullptr, mResources->cbuffer.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create cube constant buffer." };
		}

		D3D11_BUFFER_DESC constexpr lightBufferDesc{
			.ByteWidth = sizeof(LightBufferData) + 16 - sizeof(LightBufferData) % 16,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = mResources->device->CreateBuffer(&lightBufferDesc, nullptr, mResources->lightBuffer.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create light constant buffer." };
		}

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
		hresult = mResources->device->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create rasterizer state." };
		}

		mResources->context->RSSetState(rasterizerState.Get());

		mGameRes = gWindow.GetCurrentClientAreaSize();
		mGameAspect = static_cast<f32>(mGameRes.width) / static_cast<f32>(mGameRes.height);

		gWindow.OnWindowSize.add_handler(this, &on_window_resize);

		D3D11_BUFFER_DESC const quadVertBufDesc{
			.ByteWidth = sizeof(QUAD_VERTICES),
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER
		};
		D3D11_SUBRESOURCE_DATA const quadVertBufInitData{
			.pSysMem = QUAD_VERTICES
		};
		hresult = mResources->device->CreateBuffer(&quadVertBufDesc, &quadVertBufInitData, mResources->quadVertBuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create quad vertex buffer." };
		}

		D3D11_INPUT_ELEMENT_DESC const quadInputDesc{
			.SemanticName = "VertexPos",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0,
		};

		hresult = mResources->device->CreateInputLayout(&quadInputDesc, 1, gClearColorVsBin, ARRAYSIZE(gClearColorVsBin), mResources->quadIa.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create quad IA." };
		}

		D3D11_BUFFER_DESC const quadIndBufDesc{
			.ByteWidth = sizeof(QUAD_INDICES),
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_INDEX_BUFFER
		};

		D3D11_SUBRESOURCE_DATA const quadIndBufInitData{
			.pSysMem = QUAD_INDICES
		};

		hresult = mResources->device->CreateBuffer(&quadIndBufDesc, &quadIndBufInitData, mResources->quadIndBuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create quad index buffer." };
		}


		D3D11_BUFFER_DESC const clearColorCbufDesc{
			.ByteWidth = 16,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		};

		hresult = mResources->device->CreateBuffer(&clearColorCbufDesc, nullptr, mResources->clearColorCbuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create clear color cbuf." };
		}

		hresult = mResources->device->CreateVertexShader(gClearColorVsBin, ARRAYSIZE(gClearColorVsBin), nullptr, mResources->clearColorVs.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create clear color VS." };
		}

		hresult = mResources->device->CreatePixelShader(gClearColorPsBin, ARRAYSIZE(gClearColorPsBin), nullptr, mResources->clearColorPs.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create clear color PS." };
		}

		hresult = mResources->device->CreatePixelShader(gPBRPsBin, ARRAYSIZE(gPBRPsBin), nullptr, mResources->pbrPs.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create PBR PS." };
		}

		D3D11_BUFFER_DESC constexpr materialCBufDesc{
			.ByteWidth = sizeof(MaterialData) + 16 - sizeof(MaterialData) % 16,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = mResources->device->CreateBuffer(&materialCBufDesc, nullptr, mResources->materialCBuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create material cbuffer." };
		}

		D3D11_BUFFER_DESC constexpr camCBufDesc{
			.ByteWidth = sizeof(CameraBufferData) + 16 - sizeof(CameraBufferData) % 16,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		hresult = mResources->device->CreateBuffer(&camCBufDesc, nullptr, mResources->cameraCBuf.GetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to create camera cbuffer." };
		}
	}


	auto Renderer::ShutDown() noexcept -> void {
		delete mResources;
		mResources = nullptr;
	}


	auto Renderer::DrawCamera(Camera const* const cam) -> void {
		auto const& camViewport{ cam->GetViewport() };
		D3D11_VIEWPORT const viewport{
			.TopLeftX = static_cast<FLOAT>(mGameRes.width) * camViewport.position.x,
			.TopLeftY = static_cast<FLOAT>(mGameRes.height) * camViewport.position.y,
			.Width = static_cast<FLOAT>(mGameRes.width) * camViewport.extent.width,
			.Height = static_cast<FLOAT>(mGameRes.height) * camViewport.extent.height,
			.MinDepth = 0,
			.MaxDepth = 1
		};

		mResources->context->RSSetViewports(1, &viewport);

		D3D11_MAPPED_SUBRESOURCE mappedClearColorCbuf;
		mResources->context->Map(mResources->clearColorCbuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedClearColorCbuf);
		std::memcpy(mappedClearColorCbuf.pData, cam->GetBackgroundColor().get_data(), 16);
		mResources->context->Unmap(mResources->clearColorCbuf.Get(), 0);

		mResources->context->IASetInputLayout(mResources->quadIa.Get());
		mResources->context->IASetVertexBuffers(0, 1, mResources->quadVertBuf.GetAddressOf(), &QUAD_VERT_BUF_STRIDE, &QUAD_VERT_BUF_OFFSET);
		mResources->context->IASetIndexBuffer(mResources->quadIndBuf.Get(), DXGI_FORMAT_R32_UINT, 0);
		mResources->context->VSSetShader(mResources->clearColorVs.Get(), nullptr, 0);
		mResources->context->PSSetShader(mResources->clearColorPs.Get(), nullptr, 0);
		mResources->context->PSSetConstantBuffers(0, 1, mResources->clearColorCbuf.GetAddressOf());
		mResources->context->OMSetRenderTargets(1, mResources->renderTextureRtv.GetAddressOf(), nullptr);
		mResources->context->DrawIndexed(ARRAYSIZE(QUAD_INDICES), 0, 0);

		if (mCubeModels.empty()) {
			return;
		}

		DirectX::XMFLOAT3 const camPos{ cam->GetTransform().GetWorldPosition().get_data() };
		DirectX::XMFLOAT3 const camForward{ cam->GetTransform().GetForwardAxis().get_data() };
		DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&camPos), DirectX::XMLoadFloat3(&camForward), { 0, 1, 0 });

		DirectX::XMMATRIX projMat;

		if (cam->GetType() == Camera::Type::Perspective) {
			auto const fovHorizRad = DirectX::XMConvertToRadians(cam->GetPerspectiveFov());
			auto const fovVertRad = 2.0f * std::atanf(std::tanf(fovHorizRad / 2.0f) / mGameAspect);
			projMat = DirectX::XMMatrixPerspectiveFovLH(fovVertRad, mGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane());
		}
		else {
			projMat = DirectX::XMMatrixOrthographicLH(cam->GetOrthographicSize(), cam->GetOrthographicSize() / mGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane());
		}

		D3D11_MAPPED_SUBRESOURCE mappedCbuffer;
		mResources->context->Map(mResources->cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCbuffer);

		DirectX::XMStoreFloat4x4(static_cast<DirectX::XMFLOAT4X4*>(mappedCbuffer.pData), viewMat * projMat);
		mResources->context->Unmap(mResources->cbuffer.Get(), 0);

		D3D11_MAPPED_SUBRESOURCE mappedLightBuffer;
		mResources->context->Map(mResources->lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedLightBuffer);
		auto const lightBufferData{ static_cast<LightBufferData*>(mappedLightBuffer.pData) };
		lightBufferData->calcDirLight = !mDirLights.empty();
		if (!mDirLights.empty()) {
			lightBufferData->dirLightData.color = mDirLights[0]->GetColor();
			lightBufferData->dirLightData.direction = mDirLights[0]->get_direction();
			lightBufferData->dirLightData.intensity = mDirLights[0]->GetIntensity();
		}
		mResources->context->Unmap(mResources->lightBuffer.Get(), 0);

		/*D3D11_MAPPED_SUBRESOURCE mappedMatBuf;
		mResources->context->Map(mResources->materialCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedMatBuf);
		auto const matBufData{ static_cast<MaterialData*>(mappedMatBuf.pData) };
		matBufData->albedo = Vector3{ 1 };
		matBufData->metallic = 0;
		matBufData->ao = 0;
		matBufData->roughness = 0.5;
		mResources->context->Unmap(mResources->materialCBuf.Get(), 0);*/

		D3D11_MAPPED_SUBRESOURCE mappedCamBuf;
		mResources->context->Map(mResources->cameraCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCamBuf);
		auto const camBufData{ static_cast<CameraBufferData*>(mappedCamBuf.pData) };
		camBufData->camPos = cam->GetEntity()->GetTransform().GetWorldPosition();
		mResources->context->Unmap(mResources->cameraCBuf.Get(), 0);


		if (mCubeModels.size() > mInstanceBufferElementCapacity) {
			mInstanceBufferElementCapacity = static_cast<UINT>(mCubeModels.size());

			D3D11_BUFFER_DESC const desc
			{
				.ByteWidth = clamp_cast<UINT>(mInstanceBufferElementCapacity * sizeof(CubeInstanceData)),
				.Usage = D3D11_USAGE_DYNAMIC,
				.BindFlags = D3D11_BIND_VERTEX_BUFFER,
				.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
				.MiscFlags = 0,
				.StructureByteStride = 0
			};

			if (auto const hresult = mResources->device->CreateBuffer(&desc, nullptr, mResources->cubeInstBuf.ReleaseAndGetAddressOf()); FAILED(hresult)) {
				throw std::runtime_error{ "Failed to resize cube instance buffer." };
			}
		}

		D3D11_MAPPED_SUBRESOURCE mappedCubeInstBuf;
		mResources->context->Map(mResources->cubeInstBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCubeInstBuf);
		auto const mappedInstBufData{ static_cast<CubeInstanceData*>(mappedCubeInstBuf.pData) };
		for (std::size_t i = 0; i < mCubeModels.size(); i++) {
			mappedInstBufData[i].modelMat = DirectX::XMFLOAT4X4{ mCubeModels[i]->GetTransform().GetModelMatrix().get_data() };
			mappedInstBufData[i].normalMat = DirectX::XMFLOAT3X3{ mCubeModels[i]->GetTransform().GetNormalMatrix().get_data() };
		}
		mResources->context->Unmap(mResources->cubeInstBuf.Get(), 0);

		ID3D11Buffer* cubeVertBufs[]{ mResources->cubeVertBuf.Get(), mResources->cubeInstBuf.Get() };
		UINT cubeVertStrides[]{ 6 * sizeof(float), 25 * sizeof(float) };
		UINT cubeVertOffsets[]{ 0, 0 };
		mResources->context->IASetVertexBuffers(0, 2, cubeVertBufs, cubeVertStrides, cubeVertOffsets);
		mResources->context->IASetIndexBuffer(mResources->cubeIndBuf.Get(), DXGI_FORMAT_R32_UINT, 0);
		mResources->context->IASetInputLayout(mResources->cubeIa.Get());
		mResources->context->VSSetShader(mResources->cubeVertShader.Get(), nullptr, 0);
		mResources->context->VSSetConstantBuffers(0, 1, mResources->cbuffer.GetAddressOf());
		mResources->context->PSSetShader(mResources->pbrPs.Get(), nullptr, 0);
		ID3D11Buffer* const psCBuffers[]{ mCubeModels[0]->GetMaterial().GetBuffer(), mResources->cameraCBuf.Get(), mResources->lightBuffer.Get()};
		mResources->context->PSSetConstantBuffers(0, ARRAYSIZE(psCBuffers), psCBuffers);
		mResources->context->DrawIndexedInstanced(ARRAYSIZE(CUBE_INDICES), static_cast<UINT>(mCubeModels.size()), 0, 0, 0);
	}


	auto Renderer::DrawGame() -> void {
		for (auto const* const cam : Camera::GetAllInstances()) {
			DrawCamera(cam);
		}
	}


	auto Renderer::GetGameResolution() const noexcept -> Extent2D<u32> {
		return mGameRes;
	}


	auto Renderer::SetGameResolution(Extent2D<u32> resolution) noexcept -> void {
		mGameRes = resolution;
		mGameAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
		RecreateRenderTextureAndViews(mGameRes.width, mGameRes.height);
	}


	auto Renderer::GetGameFrame() const noexcept -> ID3D11ShaderResourceView* {
		return mResources->renderTextureSrv.Get();
	}


	auto Renderer::GetGameAspectRatio() const noexcept -> f32 {
		return mGameAspect;
	}


	auto Renderer::BindAndClearSwapChain() const noexcept -> void {
		FLOAT clearColor[]{ 0, 0, 0, 1 };
		mResources->context->ClearRenderTargetView(mResources->swapChainRtv.Get(), clearColor);
		mResources->context->OMSetRenderTargets(1, mResources->swapChainRtv.GetAddressOf(), nullptr);
	}


	auto Renderer::Present() const noexcept -> void {
		mResources->swapChain->Present(mSyncInterval, mSyncInterval ? mPresentFlags & ~DXGI_PRESENT_ALLOW_TEARING : mPresentFlags);
	}


	auto Renderer::GetSyncInterval() const noexcept -> u32{
		return mSyncInterval;
	}


	auto Renderer::SetSyncInterval(u32 const interval) noexcept -> void {
		mSyncInterval = interval;
	}


	auto Renderer::RegisterCubeModel(CubeModel const* const cubeModel) -> void {
		mCubeModels.emplace_back(cubeModel);
	}


	auto Renderer::UnregisterCubeModel(CubeModel const* const cubeModel) -> void {
		std::erase(mCubeModels, cubeModel);
	}


	auto Renderer::GetDevice() const noexcept -> ID3D11Device*{
		return mResources->device.Get();
	}


	auto Renderer::GetImmediateContext() const noexcept -> ID3D11DeviceContext* {
		return mResources->context.Get();
	}


	auto Renderer::RegisterDirLight(DirectionalLight const* dirLight) -> void {
		mDirLights.emplace_back(dirLight);
	}

	auto Renderer::UnregisterDirLight(DirectionalLight const* dirLight) -> void {
		std::erase(mDirLights, dirLight);
	}

	auto Renderer::RegisterSpotLight(SpotLight const* spotLight) -> void {
		mSpotLights.emplace_back(spotLight);
	}

	auto Renderer::UnregisterSpotLight(SpotLight const* spotLight) -> void {
		std::erase(mSpotLights, spotLight);
	}

	auto Renderer::RegisterPointLight(PointLight const* pointLight) -> void {
		mPointLights.emplace_back(pointLight);
	}

	auto Renderer::UnregisterPointLight(PointLight const* pointLight) -> void {
		std::erase(mPointLights, pointLight);
	}
}