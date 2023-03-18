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
#include "shaders/generated/SkyboxPSBinDebug.h"
#include "shaders/generated/SkyboxVSBinDebug.h"
#include "shaders/generated/ShadowVSBinDebug.h"

#else
#include "shaders/generated/ClearColorPSBin.h"
#include "shaders/generated/MeshBlinnPhongPSBin.h"
#include "shaders/generated/MeshPbrPSBin.h"
#include "shaders/generated/MeshVSBin.h"
#include "shaders/generated/QuadVSBin.h"
#include "shaders/generated/TexQuadVSBin.h"
#include "shaders/generated/ToneMapGammaPSBin.h"
#include "shaders/generated/SkyboxPSBin.h"
#include "shaders/generated/SkyboxVSBin.h"
#include "shaders/generated/ShadowVSBin.h"
#endif

#include <cassert>
#include <functional>

using Microsoft::WRL::ComPtr;


namespace leopph {
constexpr static int MAX_LIGHT_COUNT{ 128 };

struct LightCBufferData {
	Vector3 color;
	f32 intensity;

	Vector3 direction;
	int type;

	f32 shadowNearPlane;
	f32 range;
	f32 innerAngleCos;
	int isCastingShadow;

	Vector3 position;
	f32 outerAngleCos;

	Vector2 shadowAtlasOffset;
	Vector2 shadowAtlasScale;

	Matrix4 lightSpaceMtx;
};

struct PerFrameCBuffer {};

struct PerCamCBuffer {
	Matrix4 viewProjMat;
	Vector3 camPos;
	int lightCount{};
	LightCBufferData lights[MAX_LIGHT_COUNT];
};

struct MaterialCBufferData {
	Vector3 albedo;
	float metallic;
	float roughness;
	float ao;
};

struct PerMaterialCBuffer {
	MaterialCBufferData material;
};

struct PerModelCBuffer {
	Matrix4 modelMat;
	Matrix4 normalMat;
};

struct ToneMapGammaCBData {
	float invGamma;
};

struct SkyboxCBData {
	Matrix4 viewProjMtx;
};

struct ShadowCBData {
	Matrix4 lightVPMtx;
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


auto Renderer::ShadowAtlasAllocation::GetSize() const noexcept -> int {
	return SPOT_POINT_SHADOW_ATLAS_SIZE;
}

auto Renderer::ShadowAtlasAllocation::GetQuadrantCells(int const idx) -> std::span<std::optional<ShadowAtlasCellData>> {
	switch (idx) {
	case 0: {
		return std::span{ &q1Light, 1 };
	}
	case 1: {
		return std::span{ q2Lights };
	}
	case 2: {
		return std::span{ q3Lights };
	}
	case 3: {
		return std::span{ q4Lights };
	}
	default: {
		throw std::out_of_range{ "Shadow atlas quadrant index out of bounds." };
	}
	}
}

auto Renderer::ShadowAtlasAllocation::GetQuadrantRowColCount(int const idx) const -> int {
	if (idx > 3 || idx < 0) {
		throw std::out_of_range{ "Shadow atlas quadrant index out of bounds." };
	}

	return Pow(2, idx);
}

auto Renderer::ShadowAtlasAllocation::GetQuadrantCellSizeNormalized(int const idx) const -> float {
	return 0.5f / static_cast<float>(GetQuadrantRowColCount(idx));
}

auto Renderer::ShadowAtlasAllocation::GetQuadrantOffsetNormalized(int const idx) const -> Vector2 {
	switch (idx) {
	case 0: {
		return Vector2{ 0 };
	}
	case 1: {
		return Vector2{ 0.5f, 0 };
	}
	case 2: {
		return Vector2{ 0, 0.5f };
	}
	case 3: {
		return Vector2{ 0.5f };
	}
	default: {
		throw std::out_of_range{ "Shadow atlas quadrant index out of bounds." };
	}
	}
}

auto Renderer::ShadowAtlasAllocation::GetCellOffsetNormalized(int const quadrantIdx, int const cellIdx) const -> Vector2 {
	auto const rowColCount{ GetQuadrantRowColCount(quadrantIdx) };

	if (cellIdx < 0 || cellIdx > Pow(rowColCount, 2)) {
		throw std::out_of_range{ "Shadow atlas cell index out of bounds." };
	}

	auto const quadrantOffset{ GetQuadrantOffsetNormalized(quadrantIdx) };
	auto const cellSizeNorm{ GetQuadrantCellSizeNormalized(quadrantIdx) };
	auto const rowIdx{ cellIdx / rowColCount };
	auto const colIdx{ cellIdx % rowColCount };

	return quadrantOffset + Vector2{ static_cast<float>(colIdx) * cellSizeNorm, static_cast<float>(rowIdx) * cellSizeNorm };
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

	D3D11_INPUT_ELEMENT_DESC constexpr skyboxInputDesc
	{
		.SemanticName = "POSITION",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32B32_FLOAT,
		.InputSlot = 0,
		.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	};

	if (FAILED(mResources->device->CreateInputLayout(&skyboxInputDesc, 1, gSkyboxVSBin, ARRAYSIZE(gSkyboxVSBin), mResources->skyboxIL.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass input layout." };
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

	if (FAILED(mResources->device->CreateVertexShader(gSkyboxVSBin, ARRAYSIZE(gSkyboxVSBin), nullptr, mResources->skyboxVS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox vertex shader." };
	}

	if (FAILED(mResources->device->CreatePixelShader(gSkyboxPSBin, ARRAYSIZE(gSkyboxPSBin), nullptr, mResources->skyboxPS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pixel shader." };
	}

	if (FAILED(mResources->device->CreateVertexShader(gShadowVSBin, ARRAYSIZE(gShadowVSBin), nullptr, mResources->shadowVS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create shadow vertex shader." };
	}
}

auto Renderer::CreateSwapChain(IDXGIFactory2* const factory2) const -> void {
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
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerFrameCBuffer), 16)),
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
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerCamCBuffer), 16)),
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
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerModelCBuffer), 16)),
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

	D3D11_BUFFER_DESC constexpr skyboxCBDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(SkyboxCBData), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(mResources->device->CreateBuffer(&skyboxCBDesc, nullptr, mResources->skyboxCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr shadowCBDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(ShadowCBData), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(mResources->device->CreateBuffer(&shadowCBDesc, nullptr, mResources->shadowCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create shadow constant buffer." };
	}
}

auto Renderer::CreateRasterizerStates() const -> void {
	D3D11_RASTERIZER_DESC constexpr skyboxPassRasterizerDesc{
		.FillMode = D3D11_FILL_SOLID,
		.CullMode = D3D11_CULL_NONE,
		.FrontCounterClockwise = FALSE,
		.DepthBias = 0,
		.DepthBiasClamp = 0.0f,
		.SlopeScaledDepthBias = 0.0f,
		.DepthClipEnable = TRUE,
		.ScissorEnable = FALSE,
		.MultisampleEnable = FALSE,
		.AntialiasedLineEnable = FALSE
	};

	if (FAILED(mResources->device->CreateRasterizerState(&skyboxPassRasterizerDesc, mResources->skyboxPassRS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass rasterizer state." };
	}
}

auto Renderer::CreateDepthStencilStates() const -> void {
	D3D11_DEPTH_STENCIL_DESC constexpr skyboxPassDepthStencilDesc{
		.DepthEnable = TRUE,
		.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO,
		.DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
		.StencilEnable = FALSE,
		.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
		.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = {
			.StencilFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilPassOp = D3D11_STENCIL_OP_KEEP,
			.StencilFunc = D3D11_COMPARISON_ALWAYS
		},
		.BackFace = {
			.StencilFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
			.StencilPassOp = D3D11_STENCIL_OP_KEEP,
			.StencilFunc = D3D11_COMPARISON_ALWAYS
		}
	};

	if (FAILED(mResources->device->CreateDepthStencilState(&skyboxPassDepthStencilDesc, mResources->skyboxPassDSS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass depth-stencil state." };
	}
}

auto Renderer::CreateShadowAtlases() const -> void {
	D3D11_TEXTURE2D_DESC constexpr spotPointTexDesc{
		.Width = SPOT_POINT_SHADOW_ATLAS_SIZE,
		.Height = SPOT_POINT_SHADOW_ATLAS_SIZE,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R16_TYPELESS,
		.SampleDesc = {
			.Count = 1,
			.Quality = 0
		},
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = 0,
		.MiscFlags = 0
	};

	if (FAILED(mResources->device->CreateTexture2D(&spotPointTexDesc, nullptr, mResources->spotPointShadowAtlas.tex.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create spot/point shadow atlas texture." };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC constexpr spotPointSrvDesc{
		.Format = DXGI_FORMAT_R16_UNORM,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = {
			.MostDetailedMip = 0,
			.MipLevels = 1
		}
	};

	if (FAILED(mResources->device->CreateShaderResourceView(mResources->spotPointShadowAtlas.tex.Get(), &spotPointSrvDesc, mResources->spotPointShadowAtlas.srv.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create spot/point shadow atlas srv." };
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC constexpr spotPointDsvDesc{
		.Format = DXGI_FORMAT_D16_UNORM,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0,
		.Texture2D = {
			.MipSlice = 0
		}
	};

	if (FAILED(mResources->device->CreateDepthStencilView(mResources->spotPointShadowAtlas.tex.Get(), &spotPointDsvDesc, mResources->spotPointShadowAtlas.dsv.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create spot/point shadow atlas dsv." };
	}
}

auto Renderer::CreateSamplerStates() const -> void {
	D3D11_SAMPLER_DESC constexpr hdrTextureSamplerDesc{
		.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_NEVER,
		.BorderColor = { 1, 1, 1, 1 },
		.MinLOD = -FLT_MAX,
		.MaxLOD = FLT_MAX
	};

	if (FAILED(mResources->device->CreateSamplerState(&hdrTextureSamplerDesc, mResources->hdrTextureSS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create hdr texture sampler state." };
	}

	D3D11_SAMPLER_DESC constexpr shadowSamplerDesc{
		.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_LESS,
		.BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		.MinLOD = -FLT_MAX,
		.MaxLOD = FLT_MAX
	};

	if (FAILED(mResources->device->CreateSamplerState(&shadowSamplerDesc, mResources->shadowSS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create shadow sampler state." };
	}

	D3D11_SAMPLER_DESC constexpr materialSamplerDesc{
		.Filter = D3D11_FILTER_ANISOTROPIC,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0,
		.MaxAnisotropy = 16,
		.ComparisonFunc = D3D11_COMPARISON_NEVER,
		.BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		.MinLOD = -FLT_MAX,
		.MaxLOD = FLT_MAX
	};

	if (FAILED(mResources->device->CreateSamplerState(&materialSamplerDesc, mResources->materialSS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create material sampler state." };
	}
}

auto Renderer::DrawMeshes(std::span<StaticMeshComponent const* const> const camVisibleMeshes, bool const useMaterials) const noexcept -> void {
	for (auto const& staticMeshComponent : camVisibleMeshes) {
		auto const& mesh{ staticMeshComponent->GetMesh() };

		ID3D11Buffer* vertexBuffers[]{ mesh.GetPositionBuffer().Get(), mesh.GetNormalBuffer().Get(), mesh.GetUVBuffer().Get() };
		UINT constexpr strides[]{ sizeof(Vector3), sizeof(Vector3), sizeof(Vector2) };
		UINT constexpr offsets[]{ 0, 0, 0 };
		mResources->context->IASetVertexBuffers(0, 3, vertexBuffers, strides, offsets);
		mResources->context->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
		mResources->context->IASetInputLayout(mResources->meshIL.Get());

		D3D11_MAPPED_SUBRESOURCE mappedPerModelCBuf;
		mResources->context->Map(mResources->perModelCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerModelCBuf);
		auto& [modelMatData, normalMatData]{ *static_cast<PerModelCBuffer*>(mappedPerModelCBuf.pData) };
		modelMatData = staticMeshComponent->GetEntity()->GetTransform().GetModelMatrix();
		normalMatData = Matrix4{ staticMeshComponent->GetEntity()->GetTransform().GetNormalMatrix() };
		mResources->context->Unmap(mResources->perModelCB.Get(), 0);

		mResources->context->VSSetConstantBuffers(2, 1, mResources->perModelCB.GetAddressOf());
		mResources->context->PSSetConstantBuffers(2, 1, mResources->perModelCB.GetAddressOf());

		auto const subMeshes{ mesh.GetSubMeshes() };
		auto const& materials{ staticMeshComponent->GetMaterials() };

		for (int i = 0; i < static_cast<int>(subMeshes.size()); i++) {
			auto const& [baseVertex, firstIndex, indexCount]{ subMeshes[i] };

			if (useMaterials) {
				auto const& mtl{ static_cast<int>(materials.size()) > i ? *materials[i] : *mResources->defaultMaterial };

				auto const mtlBuffer{ mtl.GetBuffer() };
				mResources->context->VSSetConstantBuffers(3, 1, &mtlBuffer);
				mResources->context->PSSetConstantBuffers(3, 1, &mtlBuffer);

				std::array const srvs{
					mtl.GetAlbedoMap() ? mtl.GetAlbedoMap()->GetSrv() : nullptr,
					mtl.GetMetallicMap() ? mtl.GetMetallicMap()->GetSrv() : nullptr,
					mtl.GetRoughnessMap() ? mtl.GetRoughnessMap()->GetSrv() : nullptr,
					mtl.GetAoMap() ? mtl.GetAoMap()->GetSrv() : nullptr
				};

				mResources->context->PSSetShaderResources(0, static_cast<UINT>(srvs.size()), srvs.data());
			}

			mResources->context->DrawIndexed(indexCount, firstIndex, baseVertex);
		}
	}
}

auto Renderer::UpdatePerFrameCB() const noexcept -> void {
	/*D3D11_MAPPED_SUBRESOURCE mappedPerFrameCB;
	mResources->context->Map(mResources->perFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerFrameCB);

	auto const perFrameCBData{ static_cast<PerFrameCBuffer*>(mappedPerFrameCB.pData) };

	perFrameCBData->lightCount = clamp_cast<int>(mLights.size());

	for (int i = 0; i < std::min(MAX_LIGHT_COUNT, static_cast<int>(mLights.size())); i++) {
		perFrameCBData->lights[i].color = mLights[i]->GetColor();
		perFrameCBData->lights[i].intensity = mLights[i]->GetIntensity();
		perFrameCBData->lights[i].type = static_cast<int>(mLights[i]->GetType());
		perFrameCBData->lights[i].direction = mLights[i]->GetDirection();
		perFrameCBData->lights[i].isCastingShadow = false;
		perFrameCBData->lights[i].shadowNearPlane = mLights[i]->GetShadowNearPlane();
		perFrameCBData->lights[i].range = mLights[i]->GetRange();
		perFrameCBData->lights[i].innerAngleCos = std::cos(ToRadians(mLights[i]->GetInnerAngle()));
		perFrameCBData->lights[i].outerAngleCos = std::cos(ToRadians(mLights[i]->GetOuterAngle()));
		perFrameCBData->lights[i].position = mLights[i]->GetEntity()->GetTransform().GetWorldPosition();
	}

	mResources->context->Unmap(mResources->perFrameCB.Get(), 0);*/
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

auto Renderer::DrawSkybox(Matrix4 const& camViewMtx, Matrix4 const& camProjMtx) const noexcept -> void {
	if (mSkyboxes.empty()) {
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedSkyboxCB;
	mResources->context->Map(mResources->skyboxCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSkyboxCB);
	auto const skyboxCBData{ static_cast<SkyboxCBData*>(mappedSkyboxCB.pData) };
	skyboxCBData->viewProjMtx = Matrix4{ Matrix3{ camViewMtx } } * camProjMtx;
	mResources->context->Unmap(mResources->skyboxCB.Get(), 0);

	ID3D11Buffer* const vertexBuffer{ mResources->cubeMesh->GetPositionBuffer().Get() };
	UINT constexpr stride{ sizeof(Vector3) };
	UINT constexpr offset{ 0 };
	mResources->context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	mResources->context->IASetIndexBuffer(mResources->cubeMesh->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
	mResources->context->IASetInputLayout(mResources->skyboxIL.Get());

	mResources->context->VSSetShader(mResources->skyboxVS.Get(), nullptr, 0);
	mResources->context->PSSetShader(mResources->skyboxPS.Get(), nullptr, 0);

	auto const cubemapSrv{ mSkyboxes[0]->GetCubemap()->GetSrv() };
	mResources->context->PSSetShaderResources(0, 1, &cubemapSrv);

	auto const cb{ mResources->skyboxCB.Get() };
	mResources->context->VSSetConstantBuffers(0, 1, &cb);

	mResources->context->OMSetDepthStencilState(mResources->skyboxPassDSS.Get(), 0);
	mResources->context->RSSetState(mResources->skyboxPassRS.Get());

	mResources->context->DrawIndexed(clamp_cast<UINT>(CUBE_INDICES.size()), 0, 0);

	// Restore state
	mResources->context->OMSetDepthStencilState(nullptr, 0);
	mResources->context->RSSetState(nullptr);
}

auto Renderer::DrawFullWithCameras(std::span<RenderCamera const* const> const cameras, ID3D11RenderTargetView* const rtv, ID3D11DepthStencilView* const dsv, ID3D11ShaderResourceView* const srv, ID3D11RenderTargetView* const outRtv) noexcept -> void {
	FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
	mResources->context->ClearRenderTargetView(rtv, clearColor);
	mResources->context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1, 0);

	if (mStaticMeshComponents.empty()) {
		return;
	}

	ComPtr<ID3D11Resource> rtvResource;
	rtv->GetResource(rtvResource.GetAddressOf());
	ComPtr<ID3D11Texture2D> renderTarget;
	rtvResource.As(&renderTarget);
	D3D11_TEXTURE2D_DESC renderTargetDesc;
	renderTarget->GetDesc(&renderTargetDesc);
	auto const aspectRatio{ static_cast<float>(renderTargetDesc.Width) / static_cast<float>(renderTargetDesc.Height) };

	D3D11_VIEWPORT const viewport{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<FLOAT>(renderTargetDesc.Width),
		.Height = static_cast<FLOAT>(renderTargetDesc.Height),
		.MinDepth = 0,
		.MaxDepth = 1
	};

	std::array const samplers{ mResources->materialSS.Get(), mResources->shadowSS.Get() };
	mResources->context->PSSetSamplers(0, static_cast<UINT>(samplers.size()), samplers.data());

	UpdatePerFrameCB();
	mResources->context->VSSetConstantBuffers(0, 1, mResources->perFrameCB.GetAddressOf());
	mResources->context->PSSetConstantBuffers(0, 1, mResources->perFrameCB.GetAddressOf());

	for (auto const cam : cameras) {
		auto const camPos{ cam->GetPosition() };
		auto const camForward{ cam->GetForwardAxis() };
		auto const viewMat{ Matrix4::LookToLH(camPos, camForward, Vector3::Up()) };
		auto const projMat{
			cam->GetType() == RenderCamera::Type::Perspective ?
				Matrix4::PerspectiveAsymZLH(2.0f * std::atanf(std::tanf(ToRadians(cam->GetHorizontalPerspectiveFov()) / 2.0f) / aspectRatio), aspectRatio, cam->GetNearClipPlane(), cam->GetFarClipPlane()) :
				Matrix4::OrthographicAsymZLH(cam->GetHorizontalOrthographicSize(), cam->GetHorizontalOrthographicSize() / aspectRatio, cam->GetNearClipPlane(), cam->GetFarClipPlane())
		};
		auto const viewProjMat{ viewMat * projMat };

		auto const camFrust{ cam->GetFrustum(aspectRatio) };

		std::vector<LightComponent const*> static visibleLights;
		visibleLights.clear();

		for (auto const light : mLights) {
			switch (light->GetType()) {
			case LightComponent::Type::Directional: {
				visibleLights.emplace_back(light);
				break;
			}

			case LightComponent::Type::Spot: {
				auto const range{ light->GetRange() };
				auto const boundXY{ std::tan(light->GetOuterAngle()) * light->GetRange() };
				Vector4 const center{ 0, 0, 0, 1 };
				Vector4 const rightTop{ boundXY, boundXY, range, 1 };
				Vector4 const leftTop{ -boundXY, boundXY, range, 1 };
				Vector4 const rightBottom{ boundXY, -boundXY, range, 1 };
				Vector4 const leftBottom{ -boundXY, -boundXY, range, 1 };

				AABB const bounds{
					.min = Vector3{
						std::min(center[0], std::min(rightTop[0], std::min(leftTop[0], std::min(rightBottom[0], leftBottom[0])))),
						std::min(center[1], std::min(rightTop[1], std::min(leftTop[1], std::min(rightBottom[1], leftBottom[1])))),
						std::min(center[2], std::min(rightTop[2], std::min(leftTop[2], std::min(rightBottom[2], leftBottom[2]))))
					},
					.max = Vector3{
						std::max(center[0], std::max(rightTop[0], std::max(leftTop[0], std::max(rightBottom[0], leftBottom[0])))),
						std::max(center[1], std::max(rightTop[1], std::max(leftTop[1], std::max(rightBottom[1], leftBottom[1])))),
						std::max(center[2], std::max(rightTop[2], std::max(leftTop[2], std::max(rightBottom[2], leftBottom[2]))))
					}
				};

				if (is_aabb_in_frustum(bounds, camFrust, light->GetEntity()->GetTransform().GetModelMatrix() * viewMat)) {
					visibleLights.emplace_back(light);
				}

				break;
			}

			case LightComponent::Type::Point: {
				auto const range{ light->GetRange() };
				auto const boundsOffset{ Normalized(Vector3{ 1, 1, 1 }) * range };

				AABB const bounds{
					.min = boundsOffset,
					.max = boundsOffset,
				};

				if (is_aabb_in_frustum(bounds, camFrust, light->GetEntity()->GetTransform().GetModelMatrix() * viewMat)) {
					visibleLights.emplace_back(light);
				}
				break;
			}
			}
		}

		D3D11_MAPPED_SUBRESOURCE mappedPerCamCBuf;
		mResources->context->Map(mResources->perCamCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerCamCBuf);
		auto const perCamCBufData{ static_cast<PerCamCBuffer*>(mappedPerCamCBuf.pData) };
		perCamCBufData->viewProjMat = viewProjMat;
		perCamCBufData->camPos = camPos;

		auto const lightCount{ std::min(MAX_LIGHT_COUNT, static_cast<int>(visibleLights.size())) };
		perCamCBufData->lightCount = lightCount;

		for (int i = 0; i < lightCount; i++) {
			perCamCBufData->lights[i].color = visibleLights[i]->GetColor();
			perCamCBufData->lights[i].intensity = visibleLights[i]->GetIntensity();
			perCamCBufData->lights[i].type = static_cast<int>(visibleLights[i]->GetType());
			perCamCBufData->lights[i].direction = visibleLights[i]->GetDirection();
			perCamCBufData->lights[i].isCastingShadow = false;
			perCamCBufData->lights[i].shadowNearPlane = visibleLights[i]->GetShadowNearPlane();
			perCamCBufData->lights[i].range = visibleLights[i]->GetRange();
			perCamCBufData->lights[i].innerAngleCos = std::cos(ToRadians(visibleLights[i]->GetInnerAngle()));
			perCamCBufData->lights[i].outerAngleCos = std::cos(ToRadians(visibleLights[i]->GetOuterAngle()));
			perCamCBufData->lights[i].position = visibleLights[i]->GetEntity()->GetTransform().GetWorldPosition();
		}

		for (int i = 0; i < 4; i++) {
			auto const cells{ mSpotPointShadowAtlasAlloc.GetQuadrantCells(i) };
			auto const quadrantRowColCount{ mSpotPointShadowAtlasAlloc.GetQuadrantRowColCount(i) };

			for (int j = 0; j < static_cast<int>(cells.size()); j++) {
				if (cells[j]) {
					perCamCBufData->lights[cells[j]->lightIdx].isCastingShadow = true;
					perCamCBufData->lights[cells[j]->lightIdx].lightSpaceMtx = cells[j]->lightViewProj;
					perCamCBufData->lights[cells[j]->lightIdx].shadowAtlasOffset = mSpotPointShadowAtlasAlloc.GetCellOffsetNormalized(i, j);
					perCamCBufData->lights[cells[j]->lightIdx].shadowAtlasScale = Vector2{ 0.5f / static_cast<float>(quadrantRowColCount) };
				}
			}
		}

		mResources->context->Unmap(mResources->perCamCB.Get(), 0);

		std::vector<StaticMeshComponent const*> static visibleMeshes;
		visibleMeshes.clear();

		for (auto const meshComponent : mStaticMeshComponents) {
			if (is_aabb_in_frustum(meshComponent->GetMesh().GetBounds(), camFrust, meshComponent->GetEntity()->GetTransform().GetModelMatrix() * viewMat)) {
				visibleMeshes.emplace_back(meshComponent);
			}
		}

		mResources->context->VSSetShader(mResources->meshVS.Get(), nullptr, 0);

		DrawShadowMaps(viewProjMat, visibleLights, visibleMeshes);

		mResources->context->VSSetShader(mResources->meshVS.Get(), nullptr, 0);
		mResources->context->PSSetShader(mResources->meshPbrPS.Get(), nullptr, 0);

		mResources->context->VSSetConstantBuffers(1, 1, mResources->perCamCB.GetAddressOf());
		mResources->context->PSSetConstantBuffers(1, 1, mResources->perCamCB.GetAddressOf());

		mResources->context->OMSetRenderTargets(1, &rtv, dsv);
		mResources->context->PSSetShaderResources(4, 1, mResources->spotPointShadowAtlas.srv.GetAddressOf());

		mResources->context->RSSetViewports(1, &viewport);

		DrawMeshes(visibleMeshes, true);
		DrawSkybox(viewMat, projMat);
	}

	DoToneMapGammaCorrectionStep(srv, outRtv);
}

auto Renderer::DrawShadowMaps(Matrix4 const& camViewProj, std::span<LightComponent const*> const camVisibleLights, std::span<StaticMeshComponent const* const> const camVisibleMeshes) -> void {
	std::array<std::vector<int>, 4> static lightsPerQuadrant{};

	for (auto& quadrantLights : lightsPerQuadrant) {
		quadrantLights.clear();
	}

	for (int i = 0; i < static_cast<int>(camVisibleLights.size()); i++) {
		if (auto const light{ camVisibleLights[i] }; light->IsCastingShadow()) {
			switch (light->GetType()) {
			case LightComponent::Type::Directional: {
				break;
			}

			case LightComponent::Type::Spot: {
				auto const boundXY{ std::tan(light->GetOuterAngle()) * light->GetRange() };
				Vector4 const center{ 0, 0, 0, 1 };
				Vector4 const rightTop{ boundXY, boundXY, light->GetRange(), 1 };
				Vector4 const leftTop{ -boundXY, boundXY, light->GetRange(), 1 };
				Vector4 const rightBottom{ boundXY, -boundXY, light->GetRange(), 1 };
				Vector4 const leftBottom{ -boundXY, -boundXY, light->GetRange(), 1 };

				auto const mvp{ light->GetEntity()->GetTransform().GetModelMatrix() * camViewProj };
				auto const centerMvp{ center * mvp };
				auto const rightTopMvp{ rightTop * mvp };
				auto const leftTopMvp{ leftTop * mvp };
				auto const rightBottomMvp{ rightBottom * mvp };
				auto const leftBottomMvp{ leftBottom * mvp };

				auto const centerMvpDiv{ centerMvp / centerMvp[3] };
				auto const rightTopMvpDiv{ rightTopMvp / rightTopMvp[3] };
				auto const leftTopMvpDiv{ leftTopMvp / leftTopMvp[3] };
				auto const rightBottomMvpDiv{ rightBottomMvp / rightBottomMvp[3] };
				auto const leftBottomMvpDiv{ leftBottomMvp / leftBottomMvp[3] };

				Vector3 const boundsMin{
					std::min(centerMvpDiv[0], std::min(rightTopMvpDiv[0], std::min(leftTopMvpDiv[0], std::min(leftBottomMvpDiv[0], rightBottomMvpDiv[0])))),
					std::min(centerMvpDiv[1], std::min(rightTopMvpDiv[1], std::min(leftTopMvpDiv[1], std::min(leftBottomMvpDiv[1], rightBottomMvpDiv[1])))),
					std::min(centerMvpDiv[2], std::min(rightTopMvpDiv[2], std::min(leftTopMvpDiv[2], std::min(leftBottomMvpDiv[2], rightBottomMvpDiv[2]))))
				};

				Vector3 const boundsMax{
					std::max(centerMvpDiv[0], std::max(rightTopMvpDiv[0], std::max(leftTopMvpDiv[0], std::max(leftBottomMvpDiv[0], rightBottomMvpDiv[0])))),
					std::max(centerMvpDiv[1], std::max(rightTopMvpDiv[1], std::max(leftTopMvpDiv[1], std::max(leftBottomMvpDiv[1], rightBottomMvpDiv[1])))),
					std::max(centerMvpDiv[2], std::max(rightTopMvpDiv[2], std::max(leftTopMvpDiv[2], std::max(leftBottomMvpDiv[2], rightBottomMvpDiv[2]))))
				};

				auto const boundsWidth{ boundsMax[0] - boundsMin[0] };
				auto const boundsHeight{ boundsMax[1] - boundsMin[0] };

				auto const boundsArea{ boundsMax[2] > 0 ? boundsWidth * boundsHeight : 0 };
				auto constexpr screenArea{ 4 };
				auto const boundsAreaRatio{ boundsArea / screenArea };

				std::optional<int> quadrantIdx;

				if (boundsAreaRatio >= 1) {
					quadrantIdx = 0;
				}
				else if (boundsAreaRatio >= 0.25f) {
					quadrantIdx = 1;
				}
				else if (boundsAreaRatio >= 0.0625f) {
					quadrantIdx = 2;
				}
				else if (boundsAreaRatio >= 0.015625f) {
					quadrantIdx = 3;
				}

				if (quadrantIdx) {
					lightsPerQuadrant[*quadrantIdx].emplace_back(i);
				}

				break;
			}

			case LightComponent::Type::Point: {
				break;
			}
			}
		}
	}

	ShadowAtlasAllocation newAlloc{};

	for (int i = 0; i < 4; i++) {
		for (auto& cell : newAlloc.GetQuadrantCells(i)) {
			if (lightsPerQuadrant[i].empty()) {
				break;
			}

			auto const lightViewMtx{
				Matrix4::LookToLH(mLights[lightsPerQuadrant[i].back()]->GetEntity()->GetTransform().GetWorldPosition(),
				                  mLights[lightsPerQuadrant[i].back()]->GetEntity()->GetTransform().GetForwardAxis(),
				                  Vector3::Up())
			};
			auto const lightProjMtx{
				Matrix4::PerspectiveAsymZLH(ToRadians(mLights[lightsPerQuadrant[i].back()]->GetOuterAngle() * 2),
				                            1.f,
				                            0.1f,
				                            mLights[lightsPerQuadrant[i].back()]->GetRange())
			};

			cell.emplace(lightViewMtx * lightProjMtx, lightsPerQuadrant[i].back());
			lightsPerQuadrant[i].pop_back();
		}

		if (i + 1 < 4) {
			std::ranges::copy(lightsPerQuadrant[i], std::back_inserter(lightsPerQuadrant[i + 1]));
		}
	}

	mSpotPointShadowAtlasAlloc = newAlloc;

	mResources->context->OMSetRenderTargets(0, nullptr, mResources->spotPointShadowAtlas.dsv.Get());
	mResources->context->ClearDepthStencilView(mResources->spotPointShadowAtlas.dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	mResources->context->VSSetShader(mResources->shadowVS.Get(), nullptr, 0);
	mResources->context->PSSetShader(nullptr, nullptr, 0);
	mResources->context->VSSetConstantBuffers(4, 1, mResources->shadowCB.GetAddressOf());

	for (int i = 0; i < 4; i++) {
		auto const cells{ mSpotPointShadowAtlasAlloc.GetQuadrantCells(i) };
		auto const cellSize{ mSpotPointShadowAtlasAlloc.GetQuadrantCellSizeNormalized(i) * static_cast<float>(mSpotPointShadowAtlasAlloc.GetSize()) };

		for (int j = 0; j < static_cast<int>(cells.size()); j++) {
			if (cells[j]) {
				auto const cellOffset{ mSpotPointShadowAtlasAlloc.GetCellOffsetNormalized(i, j) * static_cast<float>(mSpotPointShadowAtlasAlloc.GetSize()) };

				D3D11_VIEWPORT const viewport{
					.TopLeftX = cellOffset[0],
					.TopLeftY = cellOffset[1],
					.Width = static_cast<float>(cellSize),
					.Height = static_cast<float>(cellSize),
					.MinDepth = 0,
					.MaxDepth = 1
				};

				mResources->context->RSSetViewports(1, &viewport);

				D3D11_MAPPED_SUBRESOURCE mapped;
				mResources->context->Map(mResources->shadowCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				static_cast<ShadowCBData*>(mapped.pData)->lightVPMtx = cells[j]->lightViewProj;
				mResources->context->Unmap(mResources->shadowCB.Get(), 0);

				DrawMeshes(camVisibleMeshes, false);
			}
		}
	}
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

	mGameRes = gWindow.GetCurrentClientAreaSize();
	mGameAspect = static_cast<f32>(mGameRes.width) / static_cast<f32>(mGameRes.height);

	mSceneRes = mGameRes;
	mSceneAspect = mGameAspect;

	RecreateGameTexturesAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);
	RecreateSceneTexturesAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);

	CreateSwapChain(dxgiFactory2.Get());
	CreateInputLayouts();
	CreateShaders();
	CreateVertexAndIndexBuffers();
	CreateConstantBuffers();
	CreateRasterizerStates();
	CreateDepthStencilStates();
	CreateShadowAtlases();
	CreateSamplerStates();

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
	mResources->cubeMesh->SetSubMeshes(std::vector{ Mesh::SubMeshData{ 0, 0, static_cast<int>(CUBE_INDICES.size()) } });
	mResources->cubeMesh->ValidateAndUpdate();
}


auto Renderer::ShutDown() noexcept -> void {
	delete mResources;
	mResources = nullptr;
}


auto Renderer::DrawGame() noexcept -> void {
	DrawFullWithCameras(mGameRenderCameras, mResources->gameHdrTextureRtv.Get(), mResources->gameDSV.Get(), mResources->gameHdrTextureSrv.Get(), mResources->gameOutputTextureRtv.Get());
}

auto Renderer::DrawSceneView(RenderCamera const& cam) noexcept -> void {
	auto const camPtr{ &cam };
	DrawFullWithCameras(std::span{ &camPtr, 1 }, mResources->sceneHdrTextureRtv.Get(), mResources->sceneDSV.Get(), mResources->sceneHdrTextureSrv.Get(), mResources->sceneOutputTextureRtv.Get());
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
	FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
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


auto Renderer::RegisterStaticMesh(StaticMeshComponent const* const staticMesh) -> void {
	mStaticMeshComponents.emplace_back(staticMesh);
}


auto Renderer::UnregisterStaticMesh(StaticMeshComponent const* const staticMesh) -> void {
	std::erase(mStaticMeshComponents, staticMesh);
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

auto Renderer::RegisterSkybox(SkyboxComponent const* const skybox) -> void {
	mSkyboxes.emplace_back(skybox);
}

auto Renderer::UnregisterSkybox(SkyboxComponent const* const skybox) -> void {
	std::erase(mSkyboxes, skybox);
}

auto Renderer::RegisterGameCamera(RenderCamera const& cam) -> void {
	mGameRenderCameras.emplace_back(&cam);
}

auto Renderer::UnregisterGameCamera(RenderCamera const& cam) -> void {
	std::erase(mGameRenderCameras, &cam);
}
}
