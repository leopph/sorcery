#include "Renderer.hpp"

#include "Platform.hpp"
#include "Entity.hpp"
#include "Systems.hpp"
#include "Util.hpp"
#include "TransformComponent.hpp"
#include "Mesh.hpp"

#ifndef NDEBUG
#include "shaders/generated/ClearColorPSBinDebug.h"
#include "shaders/generated/MeshBlinnPhongPSBinDebug.h"
#include "shaders/generated/MeshPbrPSBinDebug.h"
#include "shaders/generated/MeshVSBinDebug.h"
#include "shaders/generated/QuadVSBinDebug.h"
#include "shaders/generated/TexQuadVSBinDebug.h"
#include "shaders/generated/ToneMapGammaPSBinDebug.h"

#else
#include "shaders/generated/ClearColorPSBin.h"
#include "shaders/generated/MeshBlinnPhongPSBin.h"
#include "shaders/generated/MeshPbrPSBin.h"
#include "shaders/generated/MeshVSBin.h"
#include "shaders/generated/QuadVSBin.h"
#include "shaders/generated/TexQuadVSBin.h"
#include "shaders/generated/ToneMapGammaPSBin.h"
#endif

#include <cassert>
#include <functional>

using Microsoft::WRL::ComPtr;


namespace leopph {
struct CBufLight {
	Vector3 color;
	f32 intensity;
};

struct CBufDirLight : CBufLight {
	Vector3 direction;
};

struct PerFrameCBufferData {
	CBufDirLight dirLight;
	BOOL calcDirLight;
};

struct PerCameraCBufferData {
	Matrix4 viewProjMat;
	Vector3 camPos;
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

struct PerModelCBufferData {
	Matrix4 modelMat;
	Matrix4 normalMat;
};

struct ToneMapGammaCBData {
	float invGamma;
};


namespace {
Vector2 const QUAD_POSITIONS[]{
	Vector2{ -1, 1 }, Vector2{ -1, -1 }, Vector2{ 1, -1 }, Vector2{ 1, 1 }
};

Vector2 const QUAD_UVS[]{
	Vector2{ 0, 0 }, Vector2{ 0, 1 }, Vector2{ 1, 1 }, Vector2{ 1, 0 }
};

UINT constexpr QUAD_INDICES[]{
	2, 1, 0, 0, 3, 2
};

std::vector const CUBE_POSITIONS{
	Vector3{ 0.5f, 0.5f, 0.5f },
	Vector3{ 0.5f, 0.5f, 0.5f },
	Vector3{ 0.5f, 0.5f, 0.5f },

	Vector3{ -0.5f, 0.5f, 0.5f },
	Vector3{ -0.5f, 0.5f, 0.5f },
	Vector3{ -0.5f, 0.5f, 0.5f },

	Vector3{ -0.5f, 0.5f, -0.5f },
	Vector3{ -0.5f, 0.5f, -0.5f },
	Vector3{ -0.5f, 0.5f, -0.5f },

	Vector3{ 0.5f, 0.5f, -0.5f },
	Vector3{ 0.5f, 0.5f, -0.5f },
	Vector3{ 0.5f, 0.5f, -0.5f },

	Vector3{ 0.5f, -0.5f, 0.5f },
	Vector3{ 0.5f, -0.5f, 0.5f },
	Vector3{ 0.5f, -0.5f, 0.5f },

	Vector3{ -0.5f, -0.5f, 0.5f },
	Vector3{ -0.5f, -0.5f, 0.5f },
	Vector3{ -0.5f, -0.5f, 0.5f },

	Vector3{ -0.5f, -0.5f, -0.5f },
	Vector3{ -0.5f, -0.5f, -0.5f },
	Vector3{ -0.5f, -0.5f, -0.5f },

	Vector3{ 0.5f, -0.5f, -0.5f },
	Vector3{ 0.5f, -0.5f, -0.5f },
	Vector3{ 0.5f, -0.5f, -0.5f },
};

std::vector const CUBE_NORMALS{
	Vector3{ 1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, 1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, 1.0f },

	Vector3{ -1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, 1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, 1.0f },

	Vector3{ -1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, 1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, -1.0f },

	Vector3{ 1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, 1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, -1.0f },

	Vector3{ 1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, -1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, 1.0f },

	Vector3{ -1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, -1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, 1.0f },

	Vector3{ -1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, -1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, -1.0f },

	Vector3{ 1.0f, 0.0f, 0.0f },
	Vector3{ 0.0f, -1.0f, 0.0f },
	Vector3{ 0.0f, 0.0f, -1.0f },
};

std::vector const CUBE_UVS{
	Vector2{ 1, 0 },
	Vector2{ 1, 0 },
	Vector2{ 0, 0 },

	Vector2{ 0, 0 },
	Vector2{ 0, 0 },
	Vector2{ 1, 0 },

	Vector2{ 1, 0 },
	Vector2{ 0, 1 },
	Vector2{ 0, 0 },

	Vector2{ 0, 0 },
	Vector2{ 1, 1 },
	Vector2{ 1, 0 },

	Vector2{ 1, 1 },
	Vector2{ 1, 1 },
	Vector2{ 0, 1 },

	Vector2{ 0, 1 },
	Vector2{ 0, 1 },
	Vector2{ 1, 1 },

	Vector2{ 1, 1 },
	Vector2{ 0, 0 },
	Vector2{ 0, 1 },

	Vector2{ 0, 1 },
	Vector2{ 1, 0 },
	Vector2{ 1, 1 }
};

std::vector<UINT> const CUBE_INDICES{
	// Top face
	7, 4, 1,
	1, 10, 7,
	// Bottom face
	16, 19, 22,
	22, 13, 16,
	// Front face
	23, 20, 8,
	8, 11, 23,
	// Back face
	17, 14, 2,
	2, 5, 17,
	// Right face
	21, 9, 0,
	0, 12, 21,
	// Left face
	15, 3, 6,
	6, 18, 15
};
}


auto Renderer::RecreateGameTexturesAndViews(u32 const width, u32 const height) const -> void {
	D3D11_TEXTURE2D_DESC const hdrTexDesc{
		.Width = width,
		.Height = height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
		.SampleDesc = { .Count = 1, .Quality = 0 },
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = 0
	};

	if (FAILED(mResources->device->CreateTexture2D(&hdrTexDesc, nullptr, mResources->gameHdrTexture.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view hdr texture." };
	}

	D3D11_RENDER_TARGET_VIEW_DESC const hdrRtvDesc{
		.Format = hdrTexDesc.Format,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		.Texture2D
		{
			.MipSlice = 0
		}
	};

	if (FAILED(mResources->device->CreateRenderTargetView(mResources->gameHdrTexture.Get(), &hdrRtvDesc, mResources->gameHdrTextureRtv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view hdr texture rtv." };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC const hdrSrvDesc{
		.Format = hdrTexDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D =
		{
			.MostDetailedMip = 0,
			.MipLevels = 1
		}
	};

	if (FAILED(mResources->device->CreateShaderResourceView(mResources->gameHdrTexture.Get(), &hdrSrvDesc, mResources->gameHdrTextureSrv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view output texture srv." };
	}

	D3D11_TEXTURE2D_DESC const outputTexDesc{
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

	if (FAILED(mResources->device->CreateTexture2D(&outputTexDesc, nullptr, mResources->gameOutputTexture.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view output texture." };
	}

	D3D11_RENDER_TARGET_VIEW_DESC const outputRtvDesc{
		.Format = outputTexDesc.Format,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		.Texture2D
		{
			.MipSlice = 0
		}
	};

	if (FAILED(mResources->device->CreateRenderTargetView(mResources->gameOutputTexture.Get(), &outputRtvDesc, mResources->gameOutputTextureRtv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view output texture rtv." };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC const outputSrvDesc{
		.Format = outputTexDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D =
		{
			.MostDetailedMip = 0,
			.MipLevels = 1
		}
	};

	if (FAILED(mResources->device->CreateShaderResourceView(mResources->gameOutputTexture.Get(), &outputSrvDesc, mResources->gameOutputTextureSrv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view output texture srv." };
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

	if (FAILED(mResources->device->CreateTexture2D(&dsTexDesc, nullptr, mResources->gameDSTex.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view depth-stencil texture." };
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC const dsDsvDesc{
		.Format = dsTexDesc.Format,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0,
		.Texture2D = { .MipSlice = 0 }
	};

	if (FAILED(mResources->device->CreateDepthStencilView(mResources->gameDSTex.Get(), &dsDsvDesc, mResources->gameDSV.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view depth-stencil texture dsv." };
	}
}

auto Renderer::RecreateSceneTexturesAndViews(u32 const width, u32 const height) const -> void {
	D3D11_TEXTURE2D_DESC const hdrTexDesc{
		.Width = width,
		.Height = height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
		.SampleDesc = { .Count = 1, .Quality = 0 },
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = 0
	};

	if (FAILED(mResources->device->CreateTexture2D(&hdrTexDesc, nullptr, mResources->sceneHdrTexture.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view hdr texture." };
	}

	D3D11_RENDER_TARGET_VIEW_DESC const hdrRtvDesc{
		.Format = hdrTexDesc.Format,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		.Texture2D
		{
			.MipSlice = 0
		}
	};

	if (FAILED(mResources->device->CreateRenderTargetView(mResources->sceneHdrTexture.Get(), &hdrRtvDesc, mResources->sceneHdrTextureRtv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view hdr texture rtv." };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC const hdrSrvDesc{
		.Format = hdrTexDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D =
		{
			.MostDetailedMip = 0,
			.MipLevels = 1
		}
	};

	if (FAILED(mResources->device->CreateShaderResourceView(mResources->sceneHdrTexture.Get(), &hdrSrvDesc, mResources->sceneHdrTextureSrv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view output texture srv." };
	}

	D3D11_TEXTURE2D_DESC const outputTexDesc{
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

	if (FAILED(mResources->device->CreateTexture2D(&outputTexDesc, nullptr, mResources->sceneOutputTexture.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view output texture." };
	}

	D3D11_RENDER_TARGET_VIEW_DESC const outputRtvDesc{
		.Format = outputTexDesc.Format,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		.Texture2D
		{
			.MipSlice = 0
		}
	};

	if (FAILED(mResources->device->CreateRenderTargetView(mResources->sceneOutputTexture.Get(), &outputRtvDesc, mResources->sceneOutputTextureRtv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view output texture rtv." };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC const outputSrvDesc{
		.Format = outputTexDesc.Format,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D =
		{
			.MostDetailedMip = 0,
			.MipLevels = 1
		}
	};

	if (FAILED(mResources->device->CreateShaderResourceView(mResources->sceneOutputTexture.Get(), &outputSrvDesc, mResources->sceneOutputTextureSrv.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view output texture srv." };
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

	self->RecreateSwapChainRtv();
}

auto Renderer::CreateDeviceAndContext() const -> void {
	UINT creationFlags{ 0 };
	D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{ D3D_FEATURE_LEVEL_11_0 };

#ifndef NDEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, mResources->device.GetAddressOf(), nullptr, mResources->context.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create D3D device." };
	}
}

auto Renderer::SetDebugBreaks() const -> void {
	ComPtr<ID3D11Debug> d3dDebug;
	if (FAILED(mResources->device.As(&d3dDebug))) {
		throw std::runtime_error{ "Failed to get ID3D11Debug interface." };
	}

	ComPtr<ID3D11InfoQueue> d3dInfoQueue;
	if (FAILED(d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue))) {
		throw std::runtime_error{ "Failed to get ID3D11InfoQueue interface." };
	}

	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
}

auto Renderer::CheckTearingSupport(IDXGIFactory2* factory2) -> void {
	if (ComPtr<IDXGIFactory5> dxgiFactory5; SUCCEEDED(factory2->QueryInterface(IID_PPV_ARGS(dxgiFactory5.GetAddressOf())))) {
		BOOL allowTearing{};
		dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof allowTearing);

		if (allowTearing) {
			mSwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			mPresentFlags |= DXGI_PRESENT_ALLOW_TEARING;
		}
	}
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

	if (FAILED(mResources->device->CreateInputLayout(meshInputDesc, ARRAYSIZE(meshInputDesc), gMeshVSBin, ARRAYSIZE(gMeshVSBin), mResources->meshIL.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh input layout." };
	}

	D3D11_INPUT_ELEMENT_DESC constexpr quadInputDesc{
		.SemanticName = "POSITION",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32_FLOAT,
		.InputSlot = 0,
		.AlignedByteOffset = 0,
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0,
	};

	if (FAILED(mResources->device->CreateInputLayout(&quadInputDesc, 1, gQuadVSBin, ARRAYSIZE(gQuadVSBin), mResources->quadIL.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create quad input layout." };
	}

	D3D11_INPUT_ELEMENT_DESC constexpr texQuadInputDescs[]{
		{
			.SemanticName = "POSITION",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		{
			.SemanticName = "TEXCOORD",
			.SemanticIndex = 0,
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.InputSlot = 1,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		}
	};

	if (FAILED(mResources->device->CreateInputLayout(texQuadInputDescs, ARRAYSIZE(texQuadInputDescs), gTexQuadVSBin, ARRAYSIZE(gTexQuadVSBin), mResources->texQuadIL.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create textured quad input layout." };
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

	if (FAILED(mResources->device->CreateVertexShader(gQuadVSBin, ARRAYSIZE(gQuadVSBin), nullptr, mResources->quadVS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create quad vertex shader." };
	}

	if (FAILED(mResources->device->CreatePixelShader(gClearColorPSBin, ARRAYSIZE(gClearColorPSBin), nullptr, mResources->clearColorPS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create clear color pixel shader." };
	}

	if (FAILED(mResources->device->CreateVertexShader(gTexQuadVSBin, ARRAYSIZE(gTexQuadVSBin), nullptr, mResources->texQuadVS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create textured quad vertex shader." };
	}

	if (FAILED(mResources->device->CreatePixelShader(gToneMapGammaPSBin, ARRAYSIZE(gToneMapGammaPSBin), nullptr, mResources->toneMapGammaPS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create textured tonemap-gamma pixel shader." };
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

	RecreateSwapChainRtv();
}

auto Renderer::RecreateSwapChainRtv() const -> void {
	ComPtr<ID3D11Texture2D> backBuf;
	if (FAILED(mResources->swapChain->GetBuffer(0, IID_PPV_ARGS(backBuf.GetAddressOf())))) {
		throw std::runtime_error{ "Failed to get swapchain backbuffer." };
	}

	D3D11_RENDER_TARGET_VIEW_DESC constexpr rtvDesc{
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		.Texture2D = { .MipSlice = 0 }
	};

	if (FAILED(mResources->device->CreateRenderTargetView(backBuf.Get(), &rtvDesc, mResources->swapChainRtv.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create swapchain backbuffer render target view." };
	}
}

auto Renderer::CreateVertexAndIndexBuffers() const -> void {
	D3D11_BUFFER_DESC constexpr quadPosVBDesc{
		.ByteWidth = sizeof QUAD_POSITIONS,
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER
	};

	D3D11_SUBRESOURCE_DATA constexpr quadPosVBInitData{
		.pSysMem = QUAD_POSITIONS
	};

	if (FAILED(mResources->device->CreateBuffer(&quadPosVBDesc, &quadPosVBInitData, mResources->quadPosVB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create quad position vertex buffer." };
	}

	D3D11_BUFFER_DESC constexpr quadUvVBDesc{
		.ByteWidth = sizeof QUAD_UVS,
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER
	};

	D3D11_SUBRESOURCE_DATA constexpr quadUvVBInitData{
		.pSysMem = QUAD_UVS
	};

	if (FAILED(mResources->device->CreateBuffer(&quadUvVBDesc, &quadUvVBInitData, mResources->quadUvVB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create quad uv vertex buffer." };
	}

	D3D11_BUFFER_DESC constexpr quadIBDesc{
		.ByteWidth = sizeof QUAD_INDICES,
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_INDEX_BUFFER
	};

	D3D11_SUBRESOURCE_DATA constexpr quadIBInitData{
		.pSysMem = QUAD_INDICES
	};

	if (FAILED(mResources->device->CreateBuffer(&quadIBDesc, &quadIBInitData, mResources->quadIB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create quad index buffer." };
	}
}

auto Renderer::CreateConstantBuffers() const -> void {
	D3D11_BUFFER_DESC constexpr perLightCBufDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerFrameCBufferData), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(mResources->device->CreateBuffer(&perLightCBufDesc, nullptr, mResources->perFrameCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create light constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr perCamCBufDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerCameraCBufferData), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(mResources->device->CreateBuffer(&perCamCBufDesc, nullptr, mResources->perCamCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create camera constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr perModelCBufDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerModelCBufferData), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(mResources->device->CreateBuffer(&perModelCBufDesc, nullptr, mResources->perModelCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create model constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr clearColorCBufDesc{
		.ByteWidth = 16,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	if (FAILED(mResources->device->CreateBuffer(&clearColorCBufDesc, nullptr, mResources->clearColorCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create clear color constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr toneMapGammaCBDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(ToneMapGammaCBData), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(mResources->device->CreateBuffer(&toneMapGammaCBDesc, nullptr, mResources->toneMapGammaCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create tonemap-gamma constant buffer." };
	}
}

auto Renderer::DrawMeshes() const noexcept -> void {
	for (auto const& staticMeshComponent : mStaticMeshComponents) {
		auto const& mesh{ staticMeshComponent->GetMesh() };
		auto const& mat{ staticMeshComponent->GetMaterial() };

		D3D11_MAPPED_SUBRESOURCE mappedPerModelCBuf;
		mResources->context->Map(mResources->perModelCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerModelCBuf);
		auto& [modelMatData, normalMatData]{ *static_cast<PerModelCBufferData*>(mappedPerModelCBuf.pData) };
		modelMatData = staticMeshComponent->GetEntity()->GetTransform().GetModelMatrix();
		normalMatData = Matrix4{ staticMeshComponent->GetEntity()->GetTransform().GetNormalMatrix() };
		mResources->context->Unmap(mResources->perModelCB.Get(), 0);

		ID3D11Buffer* vertexBuffers[]{ mesh.GetPositionBuffer().Get(), mesh.GetNormalBuffer().Get(), mesh.GetUVBuffer().Get() };
		UINT constexpr strides[]{ sizeof(Vector3), sizeof(Vector3), sizeof(Vector2) };
		UINT constexpr offsets[]{ 0, 0, 0 };
		mResources->context->IASetVertexBuffers(0, 3, vertexBuffers, strides, offsets);
		mResources->context->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
		mResources->context->IASetInputLayout(mResources->meshIL.Get());

		ID3D11Buffer* const constantBuffers[]{ mResources->perFrameCB.Get(), mResources->perCamCB.Get(), mat.GetBuffer(), mResources->perModelCB.Get() };

		mResources->context->VSSetShader(mResources->meshVS.Get(), nullptr, 0);
		mResources->context->VSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);

		mResources->context->PSSetShader(mResources->meshPbrPS.Get(), nullptr, 0);
		mResources->context->PSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);

		if (mat.GetAlbedoMap()) {
			auto const srv{ mat.GetAlbedoMap()->GetSrv() };
			mResources->context->PSSetShaderResources(0, 1, &srv);
		}
		else {
			mResources->context->PSSetShaderResources(0, 0, nullptr);
		}

		mResources->context->DrawIndexed(clamp_cast<UINT>(mesh.GetIndices().size()), 0, 0);
	}
}

auto Renderer::UpdatePerFrameCB() const noexcept -> void {
	D3D11_MAPPED_SUBRESOURCE mappedPerFrameCB;
	mResources->context->Map(mResources->perFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerFrameCB);

	auto const perFrameCBData{ static_cast<PerFrameCBufferData*>(mappedPerFrameCB.pData) };

	perFrameCBData->calcDirLight = !mLights.empty();

	if (!mLights.empty() && mLights[0]->GetType() == LightComponent::Type::Directional) {
		perFrameCBData->dirLight.color = mLights[0]->GetColor();
		perFrameCBData->dirLight.direction = mLights[0]->GetDirection();
		perFrameCBData->dirLight.intensity = mLights[0]->GetIntensity();
	}

	mResources->context->Unmap(mResources->perFrameCB.Get(), 0);
}

auto Renderer::DoToneMapGammaCorrectionStep(ID3D11ShaderResourceView* const src, ID3D11RenderTargetView* const dst) const noexcept -> void {
	// Back up old views to restore later.

	ComPtr<ID3D11RenderTargetView> rtvBackup;
	ComPtr<ID3D11DepthStencilView> dsvBackup;
	ComPtr<ID3D11ShaderResourceView> srvBackup;
	mResources->context->OMGetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.GetAddressOf());
	mResources->context->PSGetShaderResources(0, 1, srvBackup.GetAddressOf());

	// Do the step

	mResources->context->OMSetRenderTargets(1, &dst, nullptr);

	D3D11_MAPPED_SUBRESOURCE mappedToneMapGammaCB;
	mResources->context->Map(mResources->toneMapGammaCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedToneMapGammaCB);
	auto const toneMapGammaCBData{ static_cast<ToneMapGammaCBData*>(mappedToneMapGammaCB.pData) };
	toneMapGammaCBData->invGamma = mInvGamma;
	mResources->context->Unmap(mResources->toneMapGammaCB.Get(), 0);

	ID3D11Buffer* vertexBuffers[]{ mResources->quadPosVB.Get(), mResources->quadUvVB.Get() };
	UINT constexpr strides[]{ sizeof(Vector2), sizeof(Vector2) };
	UINT constexpr offsets[]{ 0, 0 };
	mResources->context->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
	mResources->context->IASetIndexBuffer(mResources->quadIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	mResources->context->IASetInputLayout(mResources->texQuadIL.Get());

	mResources->context->VSSetShader(mResources->texQuadVS.Get(), nullptr, 0);

	mResources->context->PSSetShader(mResources->toneMapGammaPS.Get(), nullptr, 0);
	mResources->context->PSSetConstantBuffers(0, 1, mResources->toneMapGammaCB.GetAddressOf());
	mResources->context->PSSetShaderResources(0, 1, &src);
	mResources->context->PSSetSamplers(0, 1, mResources->hdrTextureSS.GetAddressOf());

	mResources->context->DrawIndexed(ARRAYSIZE(QUAD_INDICES), 0, 0);

	// Restore old view bindings to that we don't leave any input/output conflicts behind.

	mResources->context->PSSetShaderResources(0, 1, srvBackup.GetAddressOf());
	mResources->context->OMSetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.Get());
}


auto Renderer::StartUp() -> void {
	mResources = new Resources{};

	CreateDeviceAndContext();

#ifndef NDEBUG
	SetDebugBreaks();
#endif

	ComPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(mResources->device.As(&dxgiDevice))) {
		throw std::runtime_error{ "Failed to query IDXGIDevice interface." };
	}

	ComPtr<IDXGIAdapter> dxgiAdapter;
	if (FAILED(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to get IDXGIAdapter." };
	}

	ComPtr<IDXGIFactory2> dxgiFactory2;
	if (FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory2.GetAddressOf())))) {
		throw std::runtime_error{ "Failed to query IDXGIFactory2 interface." };
	}

	CheckTearingSupport(dxgiFactory2.Get());

	D3D11_SAMPLER_DESC constexpr hdrTextureSamplerDesc{
		.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_NEVER,
		.BorderColor = { 1, 1, 1, 1 },
		.MinLOD = 0,
		.MaxLOD = D3D11_FLOAT32_MAX
	};

	if (FAILED(mResources->device->CreateSamplerState(&hdrTextureSamplerDesc, mResources->hdrTextureSS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create hdr texture sampler state." };
	}

	mGameRes = gWindow.GetCurrentClientAreaSize();
	mGameAspect = static_cast<f32>(mGameRes.width) / static_cast<f32>(mGameRes.height);

	mSceneRes = mGameRes;
	mSceneAspect = mGameAspect;

	RecreateGameTexturesAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);
	RecreateSceneTexturesAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);

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
	mResources->cubeMesh = std::make_shared<Mesh>();
	mResources->cubeMesh->SetName("Cube");
	mResources->cubeMesh->SetPositions(CUBE_POSITIONS);
	mResources->cubeMesh->SetNormals(CUBE_NORMALS);
	mResources->cubeMesh->SetUVs(CUBE_UVS);
	mResources->cubeMesh->SetIndices(CUBE_INDICES);
	mResources->cubeMesh->ValidateAndUpdate();
}


auto Renderer::ShutDown() noexcept -> void {
	delete mResources;
	mResources = nullptr;
}


auto Renderer::DrawCamera(CameraComponent const* const cam) const noexcept -> void {
	auto const& camPos{ cam->GetEntity()->GetTransform().GetWorldPosition() };
	auto const& camForward{ cam->GetEntity()->GetTransform().GetForwardAxis() };
	auto const viewMat{ Matrix4::LookToLH(camPos, camForward, Vector3::Up()) };
	auto const projMat{
		cam->GetType() == CameraComponent::Type::Perspective ?
			Matrix4::PerspectiveAsymZLH(2.0f * std::atanf(std::tanf(ToRadians(cam->GetPerspectiveFov()) / 2.0f) / mGameAspect), mGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane()) :
			Matrix4::OrthographicAsymZLH(cam->GetOrthographicSize(), cam->GetOrthographicSize() / mGameAspect, cam->GetNearClipPlane(), cam->GetFarClipPlane())
	};

	auto const viewProjMat{ viewMat * projMat };

	D3D11_MAPPED_SUBRESOURCE mappedPerCamCBuf;
	mResources->context->Map(mResources->perCamCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerCamCBuf);
	auto const perCamCBufData{ static_cast<PerCameraCBufferData*>(mappedPerCamCBuf.pData) };
	perCamCBufData->viewProjMat = viewProjMat;
	perCamCBufData->camPos = camPos;
	mResources->context->Unmap(mResources->perCamCB.Get(), 0);

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

	/*D3D11_MAPPED_SUBRESOURCE mappedClearColorCbuf;
	mResources->context->Map(mResources->clearColorCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedClearColorCbuf);
	std::memcpy(mappedClearColorCbuf.pData, cam->GetBackgroundColor().GetData(), 16);
	mResources->context->Unmap(mResources->clearColorCB.Get(), 0);

	mResources->context->IASetInputLayout(mResources->quadIL.Get());
	mResources->context->IASetVertexBuffers(0, 1, mResources->quadPosVB.GetAddressOf(), &QUAD_VERT_BUF_STRIDE, &QUAD_VERT_BUF_OFFSET);
	mResources->context->IASetIndexBuffer(mResources->quadIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	mResources->context->VSSetShader(mResources->quadVS.Get(), nullptr, 0);
	mResources->context->PSSetShader(mResources->clearColorPS.Get(), nullptr, 0);
	mResources->context->PSSetConstantBuffers(0, 1, mResources->clearColorCB.GetAddressOf());
	mResources->context->OMSetRenderTargets(1, mResources->gameOutputTextureRtv.GetAddressOf(), nullptr);
	mResources->context->DrawIndexed(ARRAYSIZE(QUAD_INDICES), 0, 0);*/

	DrawMeshes();
}


auto Renderer::DrawGame() const noexcept -> void {
	FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
	mResources->context->ClearRenderTargetView(mResources->gameHdrTextureRtv.Get(), clearColor);
	mResources->context->ClearDepthStencilView(mResources->gameDSV.Get(), D3D11_CLEAR_DEPTH, 1, 0);

	if (mStaticMeshComponents.empty()) {
		return;
	}

	mResources->context->OMSetRenderTargets(1, mResources->gameHdrTextureRtv.GetAddressOf(), mResources->gameDSV.Get());

	UpdatePerFrameCB();

	for (auto const* const cam : CameraComponent::GetAllInstances()) {
		DrawCamera(cam);
	}

	DoToneMapGammaCorrectionStep(mResources->gameHdrTextureSrv.Get(), mResources->gameOutputTextureRtv.Get());
}

auto Renderer::DrawSceneView(EditorCamera const& cam) const noexcept -> void {
	FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
	mResources->context->ClearRenderTargetView(mResources->sceneHdrTextureRtv.Get(), clearColor);
	mResources->context->ClearDepthStencilView(mResources->sceneDSV.Get(), D3D11_CLEAR_DEPTH, 1, 0);

	if (mStaticMeshComponents.empty()) {
		return;
	}

	mResources->context->OMSetRenderTargets(1, mResources->sceneHdrTextureRtv.GetAddressOf(), mResources->sceneDSV.Get());

	UpdatePerFrameCB();

	auto const& camPos{ cam.position };
	auto const& camForward{ cam.orientation.Rotate(Vector3::Forward()) };
	auto const viewMat{ Matrix4::LookToLH(camPos, camForward, Vector3::Up()) };
	auto const projMat{ Matrix4::PerspectiveAsymZLH(cam.fovVertRad, mSceneAspect, cam.nearClip, cam.farClip) };
	auto const viewProjMat{ viewMat * projMat };

	D3D11_MAPPED_SUBRESOURCE mappedPerCamCBuf;
	mResources->context->Map(mResources->perCamCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerCamCBuf);
	auto const perCamCBufData{ static_cast<PerCameraCBufferData*>(mappedPerCamCBuf.pData) };
	perCamCBufData->viewProjMat = viewProjMat;
	perCamCBufData->camPos = camPos;
	mResources->context->Unmap(mResources->perCamCB.Get(), 0);

	D3D11_VIEWPORT const viewport{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<FLOAT>(mSceneRes.width),
		.Height = static_cast<FLOAT>(mSceneRes.height),
		.MinDepth = 0,
		.MaxDepth = 1
	};

	mResources->context->RSSetViewports(1, &viewport);

	DrawMeshes();
	DoToneMapGammaCorrectionStep(mResources->sceneHdrTextureSrv.Get(), mResources->sceneOutputTextureRtv.Get());
}


auto Renderer::GetGameResolution() const noexcept -> Extent2D<u32> {
	return mGameRes;
}


auto Renderer::SetGameResolution(Extent2D<u32> resolution) noexcept -> void {
	mGameRes = resolution;
	mGameAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
	RecreateGameTexturesAndViews(mGameRes.width, mGameRes.height);
}


auto Renderer::GetSceneResolution() const noexcept -> Extent2D<u32> {
	return mSceneRes;
}

auto Renderer::SetSceneResolution(Extent2D<u32> const resolution) noexcept -> void {
	mSceneRes = resolution;
	mSceneAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
	RecreateSceneTexturesAndViews(mSceneRes.width, mSceneRes.height);
}


auto Renderer::GetGameFrame() const noexcept -> ID3D11ShaderResourceView* {
	return mResources->gameOutputTextureSrv.Get();
}

auto Renderer::GetSceneFrame() const noexcept -> ID3D11ShaderResourceView* {
	return mResources->sceneOutputTextureSrv.Get();
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


auto Renderer::RegisterCubeModel(ModelComponent const* const cubeModel) -> void {
	mStaticMeshComponents.emplace_back(cubeModel);
}


auto Renderer::UnregisterCubeModel(ModelComponent const* const cubeModel) -> void {
	std::erase(mStaticMeshComponents, cubeModel);
}


auto Renderer::GetDevice() const noexcept -> ID3D11Device* {
	return mResources->device.Get();
}


auto Renderer::GetImmediateContext() const noexcept -> ID3D11DeviceContext* {
	return mResources->context.Get();
}


auto Renderer::RegisterLight(LightComponent const* light) -> void {
	mLights.emplace_back(light);
}

auto Renderer::UnregisterLight(LightComponent const* light) -> void {
	std::erase(mLights, light);
}

auto Renderer::GetDefaultMaterial() const noexcept -> std::shared_ptr<Material> {
	return mResources->defaultMaterial;
}

auto Renderer::GetCubeMesh() const noexcept -> std::shared_ptr<Mesh> {
	return mResources->cubeMesh;
}

auto Renderer::GetGamma() const noexcept -> f32 {
	return 1.f / mInvGamma;
}

auto Renderer::SetGamma(f32 const gamma) noexcept -> void {
	mInvGamma = 1.f / gamma;
}
}
