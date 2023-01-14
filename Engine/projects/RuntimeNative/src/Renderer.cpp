#include "Renderer.hpp"

#include "Platform.hpp"
#include "Entity.hpp"
#include "Systems.hpp"
#include "Util.hpp"
#include "TransformComponent.hpp"
#include "Mesh.hpp"

#ifndef NDEBUG
#include "shaders/generated/ClearColorPSBinDebug.h"
#include "shaders/generated/ClearColorVSBinDebug.h"
#include "shaders/generated/MeshBlinnPhongPSBinDebug.h"
#include "shaders/generated/MeshPbrPSBinDebug.h"
#include "shaders/generated/MeshVSBinDebug.h"
#else
#include "shaders/generated/ClearColorPSBin.h"
#include "shaders/generated/ClearColorVSBin.h"
#include "shaders/generated/MeshBlinnPhongPSBin.h"
#include "shaders/generated/MeshPbrPSBin.h"
#include "shaders/generated/MeshVSBin.h"
#endif

#include <DirectXMath.h>

#include <cassert>
#include <functional>
#include <utility>

using Microsoft::WRL::ComPtr;


namespace leopph {
	struct CBufLight {
		Vector3 color;
		f32 intensity;
	};

	struct CBufDirLight : CBufLight {
		Vector3 direction;
	};

	struct CBufLightBuffer {
		CBufDirLight dirLight;
		bool calcDirLight;
	};

	struct CBufCameraBuffer {
		DirectX::XMFLOAT4X4 viewProjMat;
		DirectX::XMFLOAT3 camPos;
	};

	struct CBufMaterial {
		Vector3 albedo;
		float metallic;
		float roughness;
		float ao;
	};

	struct CBufMaterialBuffer {
		CBufMaterial material;
	};

	struct CBufModelBuffer {
		DirectX::XMFLOAT4X4 modelMat;
		DirectX::XMFLOAT3X3 normalMat;
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
			0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
			0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
			0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
			0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
			-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
			0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
			0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
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


	auto Renderer::RecreateGameRenderTextureAndViews(u32 const width, u32 const height) const -> void {
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

		auto hresult = mResources->device->CreateTexture2D(&texDesc, nullptr, mResources->gameRenderTexture.ReleaseAndGetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to recreate game view render texture." };
		}

		D3D11_RENDER_TARGET_VIEW_DESC const rtvDesc
		{
			.Format = texDesc.Format,
			.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
			.Texture2D
			{
				.MipSlice = 0
			}
		};

		hresult = mResources->device->CreateRenderTargetView(mResources->gameRenderTexture.Get(), &rtvDesc, mResources->gameRenderTextureRtv.ReleaseAndGetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to recreate game view render target view." };
		}

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

		hresult = mResources->device->CreateShaderResourceView(mResources->gameRenderTexture.Get(), &srvDesc, mResources->gameRenderTextureSrv.ReleaseAndGetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to recreate game view shader resource view." };
		}
	}

	auto Renderer::RecreateSceneRenderTextureAndViews(u32 const width, u32 const height) const -> void {
		D3D11_TEXTURE2D_DESC const colTexDesc
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

		auto hresult = mResources->device->CreateTexture2D(&colTexDesc, nullptr, mResources->sceneRenderTexture.ReleaseAndGetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to recreate scene view color texture." };
		}

		D3D11_RENDER_TARGET_VIEW_DESC const colRtvDesc
		{
			.Format = colTexDesc.Format,
			.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
			.Texture2D
			{
				.MipSlice = 0
			}
		};

		hresult = mResources->device->CreateRenderTargetView(mResources->sceneRenderTexture.Get(), &colRtvDesc, mResources->sceneRenderTextureRtv.ReleaseAndGetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to recreate scene view render texture rtv." };
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC const colSrvDesc
		{
			.Format = colTexDesc.Format,
			.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
			.Texture2D =
			{
				.MostDetailedMip = 0,
				.MipLevels = 1
			}
		};

		hresult = mResources->device->CreateShaderResourceView(mResources->sceneRenderTexture.Get(), &colSrvDesc, mResources->sceneRenderTextureSrv.ReleaseAndGetAddressOf());

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to recreate scene view render texture srv." };
		}

		D3D11_TEXTURE2D_DESC const dsTexDesc{
			.Width = width,
			.Height = height,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
			.SampleDesc = { .Count = 1, .Quality = 0 },
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_DEPTH_STENCIL,
			.CPUAccessFlags = 0,
			.MiscFlags = 0
		};

		if (FAILED(mResources->device->CreateTexture2D(&dsTexDesc, nullptr, mResources->sceneDSTex.ReleaseAndGetAddressOf()))) {
			throw std::runtime_error{ "Failed to recreate scene view depth-stencil texture." };
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC const dsDsvDesc{
			.Format = dsTexDesc.Format,
			.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
			.Flags = 0,
			.Texture2D = { .MipSlice = 0 }
		};

		if (FAILED(mResources->device->CreateDepthStencilView(mResources->sceneDSTex.Get(), &dsDsvDesc, mResources->sceneDSV.ReleaseAndGetAddressOf()))) {
			throw std::runtime_error{ "Failed to recreate scene view depth-stencil texture dsv." };
		}
	}


	auto Renderer::on_window_resize(Renderer* const self, Extent2D<u32> const size) -> void {
		if (size.width == 0 || size.height == 0) {
			return;
		}

		self->mResources->swapChainRtv.Reset();
		self->mResources->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, self->mSwapChainFlags);

		ComPtr<ID3D11Texture2D> backBuf;
		HRESULT hresult = self->mResources->swapChain->GetBuffer(0, IID_PPV_ARGS(backBuf.GetAddressOf()));
		assert(SUCCEEDED(hresult));

		hresult = self->mResources->device->CreateRenderTargetView(backBuf.Get(), nullptr, self->mResources->swapChainRtv.GetAddressOf());
		assert(SUCCEEDED(hresult));
	}

	auto Renderer::CreateInputLayouts() const -> void {
		D3D11_INPUT_ELEMENT_DESC constexpr meshInputDesc[]{
			{
				.SemanticName = "POSITION",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 0,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			},
			{
				.SemanticName = "NORMAL",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 1,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			},
			{
				.SemanticName = "TEXCOORD",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32_FLOAT,
				.InputSlot = 2,
				.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			}
		};

		if (FAILED(mResources->device->CreateInputLayout(meshInputDesc, ARRAYSIZE(meshInputDesc), gMeshVSBin, ARRAYSIZE(gMeshVSBin), mResources->meshIA.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create mesh input layout." };
		}

		D3D11_INPUT_ELEMENT_DESC constexpr clearColorInputDesc{
			.SemanticName = "POSITION",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0,
		};

		if (FAILED(mResources->device->CreateInputLayout(&clearColorInputDesc, 1, gClearColorVSBin, ARRAYSIZE(gClearColorVSBin), mResources->clearColorIA.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create clear color input layout." };
		}
	}

	auto Renderer::CreateShaders() const -> void {
		if (FAILED(mResources->device->CreateVertexShader(gMeshVSBin, ARRAYSIZE(gMeshVSBin), nullptr, mResources->meshVS.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create mesh vertex shader." };
		}

		if (FAILED(mResources->device->CreatePixelShader(gMeshBlinnPhongPSBin, ARRAYSIZE(gMeshBlinnPhongPSBin), nullptr, mResources->meshBlinnPhongPS.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create mesh blinn-phong pixel shader." };
		}

		if (FAILED(mResources->device->CreatePixelShader(gMeshPbrPSBin, ARRAYSIZE(gMeshPbrPSBin), nullptr, mResources->meshPbrPS.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create mesh pbr pixel shader." };
		}

		if (FAILED(mResources->device->CreateVertexShader(gClearColorVSBin, ARRAYSIZE(gClearColorVSBin), nullptr, mResources->clearColorVs.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create clear color vertex shader." };
		}

		if (FAILED(mResources->device->CreatePixelShader(gClearColorPSBin, ARRAYSIZE(gClearColorPSBin), nullptr, mResources->clearColorPs.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create clear color pixel shader." };
		}
	}

	auto Renderer::CreateSwapChain(ComPtr<IDXGIFactory2> const factory2) const -> void {
		DXGI_SWAP_CHAIN_DESC1 const swapChainDesc1{
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

		if (FAILED(factory2->CreateSwapChainForHwnd(mResources->device.Get(), gWindow.GetHandle(), &swapChainDesc1, nullptr, nullptr, mResources->swapChain.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create swapchain." };
		}

		ComPtr<ID3D11Texture2D> backBuf;
		if (FAILED(mResources->swapChain->GetBuffer(0, IID_PPV_ARGS(backBuf.GetAddressOf())))) {
			throw std::runtime_error{ "Failed to get swapchain backbuffer." };
		}

		if (FAILED(mResources->device->CreateRenderTargetView(backBuf.Get(), nullptr, mResources->swapChainRtv.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create swapchain backbuffer render target view." };
		}
	}

	auto Renderer::CreateVertexAndIndexBuffers() const -> void {
		D3D11_BUFFER_DESC constexpr cubeVertBufDesc{
			.ByteWidth = sizeof CUBE_VERTICES,
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		D3D11_SUBRESOURCE_DATA constexpr cubeVertBufInitData{
			.pSysMem = CUBE_VERTICES,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		if (FAILED(mResources->device->CreateBuffer(&cubeVertBufDesc, &cubeVertBufInitData, mResources->cubeVB.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create cube vertex buffer." };
		}

		D3D11_BUFFER_DESC constexpr cubeIndBufDesc{
			.ByteWidth = sizeof CUBE_INDICES,
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_INDEX_BUFFER,
			.CPUAccessFlags = 0,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		D3D11_SUBRESOURCE_DATA constexpr cubeIndBufInitData{
			.pSysMem = CUBE_INDICES,
			.SysMemPitch = 0,
			.SysMemSlicePitch = 0
		};

		if (FAILED(mResources->device->CreateBuffer(&cubeIndBufDesc, &cubeIndBufInitData, mResources->cubeIB.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create cube index buffer." };
		}

		D3D11_BUFFER_DESC constexpr quadVertBufDesc{
			.ByteWidth = sizeof QUAD_VERTICES,
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_VERTEX_BUFFER
		};

		D3D11_SUBRESOURCE_DATA constexpr quadVertBufInitData{
			.pSysMem = QUAD_VERTICES
		};

		if (FAILED(mResources->device->CreateBuffer(&quadVertBufDesc, &quadVertBufInitData, mResources->quadVB.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create quad vertex buffer." };
		}

		D3D11_BUFFER_DESC constexpr quadIndBufDesc{
			.ByteWidth = sizeof(QUAD_INDICES),
			.Usage = D3D11_USAGE_IMMUTABLE,
			.BindFlags = D3D11_BIND_INDEX_BUFFER
		};

		D3D11_SUBRESOURCE_DATA constexpr quadIndBufInitData{
			.pSysMem = QUAD_INDICES
		};

		if (FAILED(mResources->device->CreateBuffer(&quadIndBufDesc, &quadIndBufInitData, mResources->quadIB.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create quad index buffer." };
		}
	}

	auto Renderer::CreateConstantBuffers() const -> void {
		D3D11_BUFFER_DESC constexpr lightCBufDesc{
			.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(CBufLightBuffer), 16)),
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		if (FAILED(mResources->device->CreateBuffer(&lightCBufDesc, nullptr, mResources->lightCBuf.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create light constant buffer." };
		}

		D3D11_BUFFER_DESC constexpr camCBufDesc{
			.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(CBufCameraBuffer), 16)),
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		if (FAILED(mResources->device->CreateBuffer(&camCBufDesc, nullptr, mResources->cameraCBuf.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create camera constant buffer." };
		}

		D3D11_BUFFER_DESC constexpr modelCBufDesc{
			.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(CBufModelBuffer), 16)),
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
			.MiscFlags = 0,
			.StructureByteStride = 0
		};

		if (FAILED(mResources->device->CreateBuffer(&modelCBufDesc, nullptr, mResources->modelCBuf.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create model constant buffer." };
		}

		D3D11_BUFFER_DESC constexpr clearColorCBufDesc{
			.ByteWidth = 16,
			.Usage = D3D11_USAGE_DYNAMIC,
			.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
			.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		};

		if (FAILED(mResources->device->CreateBuffer(&clearColorCBufDesc, nullptr, mResources->clearColorCBuf.GetAddressOf()))) {
			throw std::runtime_error{ "Failed to create clear color constant buffer." };
		}
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
		hresult = dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory2.GetAddressOf()));

		if (FAILED(hresult)) {
			throw std::runtime_error{ "Failed to query IDXGIFactory2 interface." };
		}

		ComPtr<IDXGIFactory5> dxgiFactory5;
		hresult = dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory5.GetAddressOf()));

		if (SUCCEEDED(hresult)) {
			BOOL allowTearing{};
			dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof allowTearing);

			if (allowTearing) {
				mSwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				mPresentFlags |= DXGI_PRESENT_ALLOW_TEARING;
			}
		}

		mGameRes = gWindow.GetCurrentClientAreaSize();
		mGameAspect = static_cast<f32>(mGameRes.width) / static_cast<f32>(mGameRes.height);

		mSceneRes = mGameRes;
		mSceneAspect = mGameAspect;

		RecreateGameRenderTextureAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);
		RecreateSceneRenderTextureAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);

		CreateSwapChain(dxgiFactory2);
		CreateInputLayouts();
		CreateShaders();
		CreateVertexAndIndexBuffers();
		CreateConstantBuffers();

		gWindow.OnWindowSize.add_handler(this, &on_window_resize);

		dxgiFactory2->MakeWindowAssociation(gWindow.GetHandle(), DXGI_MWA_NO_WINDOW_CHANGES);
		mResources->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mResources->defaultMaterial = std::make_shared<Material>();
		mResources->defaultMaterial->SetName("Default Material");
	}


	auto Renderer::ShutDown() noexcept -> void {
		delete mResources;
		mResources = nullptr;
	}


	auto Renderer::DrawCamera(CameraComponent const* const cam) -> void {
		/*auto const& camViewport{ cam->GetViewport() };
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
		mResources->context->Map(mResources->clearColorCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedClearColorCbuf);
		std::memcpy(mappedClearColorCbuf.pData, cam->GetBackgroundColor().get_data(), 16);
		mResources->context->Unmap(mResources->clearColorCBuf.Get(), 0);

		mResources->context->IASetInputLayout(mResources->clearColorIA.Get());
		mResources->context->IASetVertexBuffers(0, 1, mResources->quadVB.GetAddressOf(), &QUAD_VERT_BUF_STRIDE, &QUAD_VERT_BUF_OFFSET);
		mResources->context->IASetIndexBuffer(mResources->quadIB.Get(), DXGI_FORMAT_R32_UINT, 0);
		mResources->context->VSSetShader(mResources->clearColorVs.Get(), nullptr, 0);
		mResources->context->PSSetShader(mResources->clearColorPs.Get(), nullptr, 0);
		mResources->context->PSSetConstantBuffers(0, 1, mResources->clearColorCBuf.GetAddressOf());
		mResources->context->OMSetRenderTargets(1, mResources->gameRenderTextureRtv.GetAddressOf(), nullptr);
		mResources->context->DrawIndexed(ARRAYSIZE(QUAD_INDICES), 0, 0);

		if (mCubeModels.empty()) {
			return;
		}

		DirectX::XMFLOAT3 const camPos{ cam->GetEntity()->GetTransform().GetWorldPosition().get_data() };
		DirectX::XMFLOAT3 const camForward{ cam->GetEntity()->GetTransform().GetForwardAxis().get_data() };
		DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&camPos), DirectX::XMLoadFloat3(&camForward), { 0, 1, 0 });

		DirectX::XMMATRIX projMat;

		if (cam->GetType() == CameraComponent::Type::Perspective) {
			auto const fovHorizRad = DirectX::XMConvertToRadians(cam->GetPerspectiveFov());
			auto const fovVertRad = 2.0f * std::atanf(std::tanf(fovHorizRad / 2.0f) / mGameAspect);
			projMat = DirectX::XMMatrixPerspectiveFovLH(fovVertRad, mGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane());
		}
		else {
			projMat = DirectX::XMMatrixOrthographicLH(cam->GetOrthographicSize(), cam->GetOrthographicSize() / mGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane());
		}

		D3D11_MAPPED_SUBRESOURCE mappedCbuffer;
		mResources->context->Map(mResources->modelCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCbuffer);

		DirectX::XMStoreFloat4x4(static_cast<DirectX::XMFLOAT4X4*>(mappedCbuffer.pData), viewMat * projMat);
		mResources->context->Unmap(mResources->modelCBuf.Get(), 0);

		D3D11_MAPPED_SUBRESOURCE mappedLightBuffer;
		mResources->context->Map(mResources->lightCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedLightBuffer);
		auto const lightBufferData{ static_cast<CBufLightBuffer*>(mappedLightBuffer.pData) };
		lightBufferData->calcDirLight = !mDirLights.empty();
		if (!mDirLights.empty()) {
			lightBufferData->dirLight.color = mDirLights[0]->GetColor();
			lightBufferData->dirLight.direction = mDirLights[0]->get_direction();
			lightBufferData->dirLight.intensity = mDirLights[0]->GetIntensity();
		}
		mResources->context->Unmap(mResources->lightCBuf.Get(), 0);

		/*D3D11_MAPPED_SUBRESOURCE mappedMatBuf;
		mResources->context->Map(mResources->materialCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedMatBuf);
		auto const matBufData{ static_cast<CBufMaterialBuffer*>(mappedMatBuf.pData) };
		matBufData->albedo = Vector3{ 1 };
		matBufData->metallic = 0;
		matBufData->ao = 0;
		matBufData->roughness = 0.5;
		mResources->context->Unmap(mResources->materialCBuf.Get(), 0);

		D3D11_MAPPED_SUBRESOURCE mappedCamBuf;
		mResources->context->Map(mResources->cameraCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCamBuf);
		auto const camBufData{ static_cast<CBufCameraBuffer*>(mappedCamBuf.pData) };
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
			mappedInstBufData[i].modelMat = DirectX::XMFLOAT4X4{ mCubeModels[i]->GetEntity()->GetTransform().GetModelMatrix().get_data() };
			mappedInstBufData[i].normalMat = DirectX::XMFLOAT3X3{ mCubeModels[i]->GetEntity()->GetTransform().GetNormalMatrix().get_data() };
		}
		mResources->context->Unmap(mResources->cubeInstBuf.Get(), 0);

		ID3D11Buffer* cubeVertBufs[]{ mResources->cubeVB.Get(), mResources->cubeInstBuf.Get() };
		UINT cubeVertStrides[]{ 6 * sizeof(float), 25 * sizeof(float) };
		UINT cubeVertOffsets[]{ 0, 0 };
		mResources->context->IASetVertexBuffers(0, 2, cubeVertBufs, cubeVertStrides, cubeVertOffsets);
		mResources->context->IASetIndexBuffer(mResources->cubeIB.Get(), DXGI_FORMAT_R32_UINT, 0);
		mResources->context->IASetInputLayout(mResources->meshIA.Get());
		mResources->context->VSSetShader(mResources->meshVS.Get(), nullptr, 0);
		mResources->context->VSSetConstantBuffers(0, 1, mResources->modelCBuf.GetAddressOf());
		mResources->context->PSSetShader(mResources->meshBlinnPhongPS.Get(), nullptr, 0);
		ID3D11Buffer* const psCBuffers[]{ mCubeModels[0]->GetMaterial()->GetBuffer(), mResources->cameraCBuf.Get(), mResources->lightCBuf.Get() };
		mResources->context->PSSetConstantBuffers(0, ARRAYSIZE(psCBuffers), psCBuffers);
		mResources->context->DrawIndexedInstanced(ARRAYSIZE(CUBE_INDICES), static_cast<UINT>(mCubeModels.size()), 0, 0, 0);*/
	}


	auto Renderer::DrawGame() -> void {
		for (auto const* const cam : CameraComponent::GetAllInstances()) {
			DrawCamera(cam);
		}
	}

	auto Renderer::DrawSceneView(EditorCamera const& cam) -> void {
		D3D11_VIEWPORT const viewport{
			.TopLeftX = 0,
			.TopLeftY = 0,
			.Width = static_cast<FLOAT>(mSceneRes.width),
			.Height = static_cast<FLOAT>(mSceneRes.height),
			.MinDepth = 0,
			.MaxDepth = 1
		};

		mResources->context->RSSetViewports(1, &viewport);

		FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
		mResources->context->ClearRenderTargetView(mResources->sceneRenderTextureRtv.Get(), clearColor);
		mResources->context->ClearDepthStencilView(mResources->sceneDSV.Get(), D3D11_CLEAR_DEPTH, 1, 0);

		if (mCubeModels.empty()) {
			return;
		}

		D3D11_MAPPED_SUBRESOURCE mappedLightBuffer;
		mResources->context->Map(mResources->lightCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedLightBuffer);
		auto const lightBufferData{ static_cast<CBufLightBuffer*>(mappedLightBuffer.pData) };
		lightBufferData->calcDirLight = !mDirLights.empty();
		if (!mDirLights.empty()) {
			lightBufferData->dirLight.color = mDirLights[0]->GetColor();
			lightBufferData->dirLight.direction = mDirLights[0]->get_direction();
			lightBufferData->dirLight.intensity = mDirLights[0]->GetIntensity();
		}
		mResources->context->Unmap(mResources->lightCBuf.Get(), 0);

		DirectX::XMFLOAT3 const camPos{ cam.position.get_data() };
		DirectX::XMFLOAT3 const camForward{ cam.orientation.Rotate(Vector3::forward()).get_data() };
		DirectX::XMMATRIX const viewMat{ DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&camPos), DirectX::XMLoadFloat3(&camForward), { 0, 1, 0 }) };
		DirectX::XMMATRIX const projMat{ DirectX::XMMatrixPerspectiveFovLH(cam.fovVertRad, mSceneAspect, cam.nearClip, cam.farClip) };
		auto const viewProjMat{ XMMatrixMultiply(viewMat, projMat) };

		D3D11_MAPPED_SUBRESOURCE mappedCamBuf;
		mResources->context->Map(mResources->cameraCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCamBuf);
		auto const camBufData{ static_cast<CBufCameraBuffer*>(mappedCamBuf.pData) };
		XMStoreFloat4x4(&camBufData->viewProjMat, viewProjMat);
		camBufData->camPos = camPos;
		mResources->context->Unmap(mResources->cameraCBuf.Get(), 0);

		for (auto const& cubeModel : mCubeModels) {
			auto const mesh{ cubeModel->GetMesh() };
			auto const mat{ cubeModel->GetMaterial() };

			if (!mesh || !mat) {
				continue;
			}

			DirectX::XMFLOAT4X4 const modelMat{ cubeModel->GetEntity()->GetTransform().GetModelMatrix().get_data() };
			DirectX::XMFLOAT3X3 const normalMat{ cubeModel->GetEntity()->GetTransform().GetNormalMatrix().get_data() };

			D3D11_MAPPED_SUBRESOURCE mappedMatrixCBuffer;
			mResources->context->Map(mResources->modelCBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedMatrixCBuffer);
			auto& [modelMatData, normalMatData]{ *static_cast<CBufModelBuffer*>(mappedMatrixCBuffer.pData) };
			modelMatData = modelMat;
			normalMatData = normalMat;
			mResources->context->Unmap(mResources->modelCBuf.Get(), 0);

			ID3D11Buffer* vertexBuffers[]{ mesh->GetPositionBuffer().Get(), mesh->GetNormalBuffer().Get(), mesh->GetUVBuffer().Get() };
			UINT constexpr strides[]{ sizeof(Vector3), sizeof(Vector3), sizeof(Vector2) };
			UINT constexpr offsets[]{ 0, 0, 0 };
			mResources->context->IASetVertexBuffers(0, 3, vertexBuffers, strides, offsets);
			mResources->context->IASetIndexBuffer(mesh->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

			mResources->context->VSSetShader(mResources->meshVS.Get(), nullptr, 0);
			
			mResources->context->VSSetConstantBuffers(1, 1, mResources->cameraCBuf.GetAddressOf());
			mResources->context->VSSetConstantBuffers(3, 1, mResources->modelCBuf.GetAddressOf());

			mResources->context->PSSetShader(mResources->meshPbrPS.Get(), nullptr, 0);

			ID3D11Buffer* const psConstantBuffers[]{ mResources->lightCBuf.Get(), mResources->cameraCBuf.Get(), mCubeModels[0]->GetMaterial()->GetBuffer() };
			mResources->context->PSSetConstantBuffers(0, ARRAYSIZE(psConstantBuffers), psConstantBuffers);

			mResources->context->IASetInputLayout(mResources->meshIA.Get());
			mResources->context->OMSetRenderTargets(1, mResources->sceneRenderTextureRtv.GetAddressOf(), mResources->sceneDSV.Get());
			mResources->context->DrawIndexed(clamp_cast<UINT>(mesh->GetIndices().size()), 0, 0);
		}
	}


	auto Renderer::GetGameResolution() const noexcept -> Extent2D<u32> {
		return mGameRes;
	}


	auto Renderer::SetGameResolution(Extent2D<u32> resolution) noexcept -> void {
		mGameRes = resolution;
		mGameAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
		RecreateGameRenderTextureAndViews(mGameRes.width, mGameRes.height);
	}


	auto Renderer::GetSceneResolution() const noexcept -> Extent2D<u32> {
		return mSceneRes;
	}

	auto Renderer::SetSceneResolution(Extent2D<u32> const resolution) noexcept -> void {
		mSceneRes = resolution;
		mSceneAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
		RecreateSceneRenderTextureAndViews(mSceneRes.width, mSceneRes.height);
	}


	auto Renderer::GetGameFrame() const noexcept -> ID3D11ShaderResourceView* {
		return mResources->gameRenderTextureSrv.Get();
	}

	auto Renderer::GetSceneFrame() const noexcept -> ID3D11ShaderResourceView* {
		return mResources->sceneRenderTextureSrv.Get();
	}


	auto Renderer::GetGameAspectRatio() const noexcept -> f32 {
		return mGameAspect;
	}

	auto Renderer::GetSceneAspectRatio() const noexcept -> f32 {
		return mSceneAspect;
	}


	auto Renderer::BindAndClearSwapChain() const noexcept -> void {
		FLOAT clearColor[]{ 0, 0, 0, 1 };
		mResources->context->ClearRenderTargetView(mResources->swapChainRtv.Get(), clearColor);
		mResources->context->OMSetRenderTargets(1, mResources->swapChainRtv.GetAddressOf(), nullptr);
	}


	auto Renderer::Present() const noexcept -> void {
		mResources->swapChain->Present(mSyncInterval, mSyncInterval ? mPresentFlags & ~DXGI_PRESENT_ALLOW_TEARING : mPresentFlags);
	}


	auto Renderer::GetSyncInterval() const noexcept -> u32 {
		return mSyncInterval;
	}


	auto Renderer::SetSyncInterval(u32 const interval) noexcept -> void {
		mSyncInterval = interval;
	}


	auto Renderer::RegisterCubeModel(CubeModelComponent const* const cubeModel) -> void {
		mCubeModels.emplace_back(cubeModel);
	}


	auto Renderer::UnregisterCubeModel(CubeModelComponent const* const cubeModel) -> void {
		std::erase(mCubeModels, cubeModel);
	}


	auto Renderer::GetDevice() const noexcept -> ID3D11Device* {
		return mResources->device.Get();
	}


	auto Renderer::GetImmediateContext() const noexcept -> ID3D11DeviceContext* {
		return mResources->context.Get();
	}


	auto Renderer::RegisterDirLight(DirectionalLightComponent const* dirLight) -> void {
		mDirLights.emplace_back(dirLight);
	}

	auto Renderer::UnregisterDirLight(DirectionalLightComponent const* dirLight) -> void {
		std::erase(mDirLights, dirLight);
	}

	auto Renderer::RegisterSpotLight(SpotLight const* spotLight) -> void {
		mSpotLights.emplace_back(spotLight);
	}

	auto Renderer::UnregisterSpotLight(SpotLight const* spotLight) -> void {
		std::erase(mSpotLights, spotLight);
	}

	auto Renderer::RegisterPointLight(PointLightComponent const* pointLight) -> void {
		mPointLights.emplace_back(pointLight);
	}

	auto Renderer::UnregisterPointLight(PointLightComponent const* pointLight) -> void {
		std::erase(mPointLights, pointLight);
	}

	auto Renderer::GetDefaultMaterial() const noexcept -> std::shared_ptr<Material> {
		return mResources->defaultMaterial;
	}
}
