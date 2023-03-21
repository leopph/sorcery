#include "Renderer.hpp"

#include <dxgi1_6.h>

#include "Platform.hpp"
#include "Entity.hpp"
#include "Systems.hpp"
#include "Util.hpp"
#include "TransformComponent.hpp"
#include "Mesh.hpp"

#ifndef NDEBUG
#include "shaders/generated/MeshBlinnPhongPSBinDebug.h"
#include "shaders/generated/MeshPbrPSBinDebug.h"
#include "shaders/generated/MeshVSBinDebug.h"
#include "shaders/generated/ToneMapGammaPSBinDebug.h"
#include "shaders/generated/SkyboxPSBinDebug.h"
#include "shaders/generated/SkyboxVSBinDebug.h"
#include "shaders/generated/ShadowVSBinDebug.h"
#include "shaders/generated/ScreenVSBinDebug.h"

#else
#include "shaders/generated/MeshBlinnPhongPSBin.h"
#include "shaders/generated/MeshPbrPSBin.h"
#include "shaders/generated/MeshVSBin.h"
#include "shaders/generated/ToneMapGammaPSBin.h"
#include "shaders/generated/SkyboxPSBin.h"
#include "shaders/generated/SkyboxVSBin.h"
#include "shaders/generated/ShadowVSBin.h"
#include "shaders/generated/ScreenVSBin.h"
#endif

#include "shaders/ShaderInterop.h"

#include <cassert>
#include <functional>

using Microsoft::WRL::ComPtr;


namespace leopph::renderer {
namespace {
constexpr int PUNCTUAL_SHADOW_ATLAS_SIZE{ 4096 };


struct ShadowAtlas {
	ComPtr<ID3D11Texture2D> tex;
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11DepthStencilView> dsv;
};


struct Resources {
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;

	ComPtr<IDXGISwapChain1> swapChain;
	ComPtr<ID3D11Texture2D> gameHdrTexture;
	ComPtr<ID3D11Texture2D> gameOutputTexture;
	ComPtr<ID3D11Texture2D> gameDSTex;
	ComPtr<ID3D11Texture2D> sceneHdrTexture;
	ComPtr<ID3D11Texture2D> sceneOutputTexture;
	ComPtr<ID3D11Texture2D> sceneDSTex;

	ComPtr<ID3D11RenderTargetView> swapChainRtv;
	ComPtr<ID3D11RenderTargetView> gameHdrTextureRtv;
	ComPtr<ID3D11RenderTargetView> gameOutputTextureRtv;
	ComPtr<ID3D11RenderTargetView> sceneHdrTextureRtv;
	ComPtr<ID3D11RenderTargetView> sceneOutputTextureRtv;

	ComPtr<ID3D11ShaderResourceView> gameHdrTextureSrv;
	ComPtr<ID3D11ShaderResourceView> gameOutputTextureSrv;
	ComPtr<ID3D11ShaderResourceView> sceneHdrTextureSrv;
	ComPtr<ID3D11ShaderResourceView> sceneOutputTextureSrv;
	ComPtr<ID3D11ShaderResourceView> lightSbSrv;

	ComPtr<ID3D11DepthStencilView> gameDSV;
	ComPtr<ID3D11DepthStencilView> sceneDSV;

	ComPtr<ID3D11PixelShader> meshBlinnPhongPS;
	ComPtr<ID3D11VertexShader> meshVS;
	ComPtr<ID3D11PixelShader> meshPbrPS;
	ComPtr<ID3D11PixelShader> toneMapGammaPS;
	ComPtr<ID3D11PixelShader> skyboxPS;
	ComPtr<ID3D11VertexShader> skyboxVS;
	ComPtr<ID3D11VertexShader> shadowVS;
	ComPtr<ID3D11VertexShader> screenVS;

	ComPtr<ID3D11Buffer> perFrameCB;
	ComPtr<ID3D11Buffer> perCamCB;
	ComPtr<ID3D11Buffer> perModelCB;
	ComPtr<ID3D11Buffer> toneMapGammaCB;
	ComPtr<ID3D11Buffer> skyboxCB;
	ComPtr<ID3D11Buffer> shadowCB;
	ComPtr<ID3D11Buffer> lightSB;

	ComPtr<ID3D11InputLayout> meshIL;
	ComPtr<ID3D11InputLayout> skyboxIL;

	ComPtr<ID3D11SamplerState> materialSS;
	ComPtr<ID3D11SamplerState> shadowSS;

	ComPtr<ID3D11RasterizerState> skyboxPassRS;

	ComPtr<ID3D11DepthStencilState> skyboxPassDSS;

	std::unique_ptr<Material> defaultMaterial;
	std::unique_ptr<Mesh> cubeMesh;
	std::unique_ptr<Mesh> planeMesh;

	ShadowAtlas punctualShadowAtlas;
};


struct ShadowAtlasCellData {
	Matrix4 lightViewMtx;
	Matrix4 lightViewProjMtx;
	// Index into the array of indices to the visible lights, use lights[visibleLights[visibleLightIdxIdx]] to get to the light
	int visibleLightIdxIdx;
};


#pragma warning(push)
#pragma warning(disable: 4324)
class ShadowAtlasAllocation {
	std::optional<ShadowAtlasCellData> mQuadrant0;
	std::array<std::optional<ShadowAtlasCellData>, 4> mQuadrant1;
	std::array<std::optional<ShadowAtlasCellData>, 16> mQuadrant2;
	std::array<std::optional<ShadowAtlasCellData>, 64> mQuadrant3;

public:
	[[nodiscard]] auto GetSize() const noexcept -> int {
		return PUNCTUAL_SHADOW_ATLAS_SIZE;
	}


	[[nodiscard]] auto GetQuadrantCells(int const idx) const -> std::span<std::optional<ShadowAtlasCellData> const> {
		return const_cast<ShadowAtlasAllocation*>(this)->GetQuadrantCells(idx);
	}


	[[nodiscard]] auto GetQuadrantCells(int const idx) -> std::span<std::optional<ShadowAtlasCellData>> {
		switch (idx) {
		case 0: {
			return std::span{ &mQuadrant0, 1 };
		}
		case 1: {
			return std::span{ mQuadrant1 };
		}
		case 2: {
			return std::span{ mQuadrant2 };
		}
		case 3: {
			return std::span{ mQuadrant3 };
		}
		default: {
			throw std::out_of_range{ "Shadow atlas quadrant index out of bounds." };
		}
		}
	}


	[[nodiscard]] auto GetQuadrantRowColCount(int const idx) const -> int {
		if (idx > 3 || idx < 0) {
			throw std::out_of_range{ "Shadow atlas quadrant index out of bounds." };
		}

		return Pow(2, idx);
	}


	[[nodiscard]] auto GetQuadrantCellSizeNormalized(int const idx) const -> float {
		return 0.5f / static_cast<float>(GetQuadrantRowColCount(idx));
	}


	[[nodiscard]] auto GetQuadrantOffsetNormalized(int const idx) const -> Vector2 {
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


	[[nodiscard]] auto GetCellOffsetNormalized(int const quadrantIdx, int const cellIdx) const -> Vector2 {
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
};
#pragma warning(pop)


std::vector const QUAD_POSITIONS{
	Vector3{ -1, 1, 0 }, Vector3{ -1, -1, 0 }, Vector3{ 1, -1, 0 }, Vector3{ 1, 1, 0 }
};


std::vector const QUAD_NORMALS{
	Vector3::Backward(), Vector3::Backward(), Vector3::Backward(), Vector3::Backward()
};


std::vector const QUAD_UVS{
	Vector2{ 0, 0 }, Vector2{ 0, 1 }, Vector2{ 1, 1 }, Vector2{ 1, 0 }
};


std::vector<unsigned> const QUAD_INDICES{
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


Resources* gResources{ nullptr };
UINT gPresentFlags{ 0 };
UINT gSwapChainFlags{ 0 };
Extent2D<u32> gGameRes;
f32 gGameAspect;
Extent2D<u32> gSceneRes;
f32 gSceneAspect;
u32 gSyncInterval{ 0 };
std::vector<StaticMeshComponent const*> gStaticMeshComponents;
std::vector<LightComponent const*> gLights;
f32 gInvGamma{ 1.f / 2.2f };
std::vector<SkyboxComponent const*> gSkyboxes;
std::vector<Camera const*> gGameRenderCameras;
ShadowAtlasAllocation gPunctualShadowAtlasAlloc;


auto RecreateGameTexturesAndViews(u32 const width, u32 const height) -> void {
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

	if (FAILED(gResources->device->CreateTexture2D(&hdrTexDesc, nullptr, gResources->gameHdrTexture.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateRenderTargetView(gResources->gameHdrTexture.Get(), &hdrRtvDesc, gResources->gameHdrTextureRtv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateShaderResourceView(gResources->gameHdrTexture.Get(), &hdrSrvDesc, gResources->gameHdrTextureSrv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateTexture2D(&outputTexDesc, nullptr, gResources->gameOutputTexture.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateRenderTargetView(gResources->gameOutputTexture.Get(), &outputRtvDesc, gResources->gameOutputTextureRtv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateShaderResourceView(gResources->gameOutputTexture.Get(), &outputSrvDesc, gResources->gameOutputTextureSrv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateTexture2D(&dsTexDesc, nullptr, gResources->gameDSTex.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view depth-stencil texture." };
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC const dsDsvDesc{
		.Format = dsTexDesc.Format,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0,
		.Texture2D = { .MipSlice = 0 }
	};

	if (FAILED(gResources->device->CreateDepthStencilView(gResources->gameDSTex.Get(), &dsDsvDesc, gResources->gameDSV.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate game view depth-stencil texture dsv." };
	}
}


auto RecreateSceneTexturesAndViews(u32 const width, u32 const height) -> void {
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

	if (FAILED(gResources->device->CreateTexture2D(&hdrTexDesc, nullptr, gResources->sceneHdrTexture.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateRenderTargetView(gResources->sceneHdrTexture.Get(), &hdrRtvDesc, gResources->sceneHdrTextureRtv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateShaderResourceView(gResources->sceneHdrTexture.Get(), &hdrSrvDesc, gResources->sceneHdrTextureSrv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateTexture2D(&outputTexDesc, nullptr, gResources->sceneOutputTexture.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateRenderTargetView(gResources->sceneOutputTexture.Get(), &outputRtvDesc, gResources->sceneOutputTextureRtv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateShaderResourceView(gResources->sceneOutputTexture.Get(), &outputSrvDesc, gResources->sceneOutputTextureSrv.ReleaseAndGetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateTexture2D(&dsTexDesc, nullptr, gResources->sceneDSTex.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view depth-stencil texture." };
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC const dsDsvDesc{
		.Format = dsTexDesc.Format,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0,
		.Texture2D = { .MipSlice = 0 }
	};

	if (FAILED(gResources->device->CreateDepthStencilView(gResources->sceneDSTex.Get(), &dsDsvDesc, gResources->sceneDSV.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to recreate scene view depth-stencil texture dsv." };
	}
}


auto RecreateSwapChainRtv() -> void {
	ComPtr<ID3D11Texture2D> backBuf;
	if (FAILED(gResources->swapChain->GetBuffer(0, IID_PPV_ARGS(backBuf.GetAddressOf())))) {
		throw std::runtime_error{ "Failed to get swapchain backbuffer." };
	}

	D3D11_RENDER_TARGET_VIEW_DESC constexpr rtvDesc{
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
		.Texture2D = { .MipSlice = 0 }
	};

	if (FAILED(gResources->device->CreateRenderTargetView(backBuf.Get(), &rtvDesc, gResources->swapChainRtv.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create swapchain backbuffer render target view." };
	}
}


auto on_window_resize(Extent2D<u32> const size) -> void {
	if (size.width == 0 || size.height == 0) {
		return;
	}

	gResources->swapChainRtv.Reset();
	gResources->swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, gSwapChainFlags);

	RecreateSwapChainRtv();
}


auto CreateDeviceAndContext() -> void {
	UINT creationFlags{ 0 };
	D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{ D3D_FEATURE_LEVEL_11_0 };

#ifndef NDEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, gResources->device.GetAddressOf(), nullptr, gResources->context.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create D3D device." };
	}
}


auto SetDebugBreaks() -> void {
	ComPtr<ID3D11Debug> d3dDebug;
	if (FAILED(gResources->device.As(&d3dDebug))) {
		throw std::runtime_error{ "Failed to get ID3D11Debug interface." };
	}

	ComPtr<ID3D11InfoQueue> d3dInfoQueue;
	if (FAILED(d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue))) {
		throw std::runtime_error{ "Failed to get ID3D11InfoQueue interface." };
	}

	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
	d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
}


auto CheckTearingSupport(IDXGIFactory2* factory2) -> void {
	if (ComPtr<IDXGIFactory5> dxgiFactory5; SUCCEEDED(factory2->QueryInterface(IID_PPV_ARGS(dxgiFactory5.GetAddressOf())))) {
		BOOL allowTearing{};
		dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof allowTearing);

		if (allowTearing) {
			gSwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			gPresentFlags |= DXGI_PRESENT_ALLOW_TEARING;
		}
	}
}


auto CreateInputLayouts() -> void {
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

	if (FAILED(gResources->device->CreateInputLayout(meshInputDesc, ARRAYSIZE(meshInputDesc), gMeshVSBin, ARRAYSIZE(gMeshVSBin), gResources->meshIL.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh input layout." };
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

	if (FAILED(gResources->device->CreateInputLayout(&skyboxInputDesc, 1, gSkyboxVSBin, ARRAYSIZE(gSkyboxVSBin), gResources->skyboxIL.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass input layout." };
	}
}


auto CreateShaders() -> void {
	if (FAILED(gResources->device->CreateVertexShader(gMeshVSBin, ARRAYSIZE(gMeshVSBin), nullptr, gResources->meshVS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh vertex shader." };
	}

	if (FAILED(gResources->device->CreatePixelShader(gMeshBlinnPhongPSBin, ARRAYSIZE(gMeshBlinnPhongPSBin), nullptr, gResources->meshBlinnPhongPS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh blinn-phong pixel shader." };
	}

	if (FAILED(gResources->device->CreatePixelShader(gMeshPbrPSBin, ARRAYSIZE(gMeshPbrPSBin), nullptr, gResources->meshPbrPS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create mesh pbr pixel shader." };
	}

	if (FAILED(gResources->device->CreatePixelShader(gToneMapGammaPSBin, ARRAYSIZE(gToneMapGammaPSBin), nullptr, gResources->toneMapGammaPS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create textured tonemap-gamma pixel shader." };
	}

	if (FAILED(gResources->device->CreateVertexShader(gSkyboxVSBin, ARRAYSIZE(gSkyboxVSBin), nullptr, gResources->skyboxVS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox vertex shader." };
	}

	if (FAILED(gResources->device->CreatePixelShader(gSkyboxPSBin, ARRAYSIZE(gSkyboxPSBin), nullptr, gResources->skyboxPS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pixel shader." };
	}

	if (FAILED(gResources->device->CreateVertexShader(gShadowVSBin, ARRAYSIZE(gShadowVSBin), nullptr, gResources->shadowVS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create shadow vertex shader." };
	}

	if (FAILED(gResources->device->CreateVertexShader(gScreenVSBin, ARRAYSIZE(gScreenVSBin), nullptr, gResources->screenVS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create screen vertex shader." };
	}
}


auto CreateSwapChain(IDXGIFactory2* const factory2) -> void {
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
		.Flags = gSwapChainFlags
	};

	if (FAILED(factory2->CreateSwapChainForHwnd(gResources->device.Get(), gWindow.GetHandle(), &swapChainDesc1, nullptr, nullptr, gResources->swapChain.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create swapchain." };
	}

	RecreateSwapChainRtv();
}


auto CreateConstantBuffers() -> void {
	D3D11_BUFFER_DESC constexpr perFrameCbDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerFrameCB), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(gResources->device->CreateBuffer(&perFrameCbDesc, nullptr, gResources->perFrameCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create light constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr perCamCbDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerCameraCB), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(gResources->device->CreateBuffer(&perCamCbDesc, nullptr, gResources->perCamCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create camera constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr perModelCbDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(PerModelCB), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(gResources->device->CreateBuffer(&perModelCbDesc, nullptr, gResources->perModelCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create model constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr toneMapGammaCBDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(ToneMapGammaCB), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(gResources->device->CreateBuffer(&toneMapGammaCBDesc, nullptr, gResources->toneMapGammaCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create tonemap-gamma constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr skyboxCBDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(SkyboxCB), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(gResources->device->CreateBuffer(&skyboxCBDesc, nullptr, gResources->skyboxCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass constant buffer." };
	}

	D3D11_BUFFER_DESC constexpr shadowCBDesc{
		.ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(ShadowCB), 16)),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = 0,
		.StructureByteStride = 0
	};

	if (FAILED(gResources->device->CreateBuffer(&shadowCBDesc, nullptr, gResources->shadowCB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create shadow constant buffer." };
	}
}


auto CreateRasterizerStates() -> void {
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

	if (FAILED(gResources->device->CreateRasterizerState(&skyboxPassRasterizerDesc, gResources->skyboxPassRS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass rasterizer state." };
	}
}


auto CreateDepthStencilStates() -> void {
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

	if (FAILED(gResources->device->CreateDepthStencilState(&skyboxPassDepthStencilDesc, gResources->skyboxPassDSS.ReleaseAndGetAddressOf()))) {
		throw std::runtime_error{ "Failed to create skybox pass depth-stencil state." };
	}
}


auto CreateShadowAtlases() -> void {
	D3D11_TEXTURE2D_DESC constexpr punctAtlasDesc{
		.Width = PUNCTUAL_SHADOW_ATLAS_SIZE,
		.Height = PUNCTUAL_SHADOW_ATLAS_SIZE,
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

	if (FAILED(gResources->device->CreateTexture2D(&punctAtlasDesc, nullptr, gResources->punctualShadowAtlas.tex.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create spot/point shadow atlas texture." };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC constexpr punctSrvDesc{
		.Format = DXGI_FORMAT_R16_UNORM,
		.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
		.Texture2D = {
			.MostDetailedMip = 0,
			.MipLevels = 1
		}
	};

	if (FAILED(gResources->device->CreateShaderResourceView(gResources->punctualShadowAtlas.tex.Get(), &punctSrvDesc, gResources->punctualShadowAtlas.srv.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create spot/point shadow atlas srv." };
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC constexpr puntDsvDesc{
		.Format = DXGI_FORMAT_D16_UNORM,
		.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
		.Flags = 0,
		.Texture2D = {
			.MipSlice = 0
		}
	};

	if (FAILED(gResources->device->CreateDepthStencilView(gResources->punctualShadowAtlas.tex.Get(), &puntDsvDesc, gResources->punctualShadowAtlas.dsv.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create spot/point shadow atlas dsv." };
	}
}


auto CreateSamplerStates() -> void {
	D3D11_SAMPLER_DESC constexpr shadowSamplerDesc{
		.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MipLODBias = 0,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,
		.BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
		.MinLOD = -FLT_MAX,
		.MaxLOD = FLT_MAX
	};

	if (FAILED(gResources->device->CreateSamplerState(&shadowSamplerDesc, gResources->shadowSS.GetAddressOf()))) {
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

	if (FAILED(gResources->device->CreateSamplerState(&materialSamplerDesc, gResources->materialSS.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create material sampler state." };
	}
}


auto CreateDefaultAssets() -> void {
	gResources->defaultMaterial = std::make_unique<Material>();
	gResources->defaultMaterial->SetName("Default Material");

	gResources->cubeMesh = std::make_unique<Mesh>();
	gResources->cubeMesh->SetGuid(Guid{ 0, 0 });
	gResources->cubeMesh->SetName("Cube");
	gResources->cubeMesh->SetPositions(CUBE_POSITIONS);
	gResources->cubeMesh->SetNormals(CUBE_NORMALS);
	gResources->cubeMesh->SetUVs(CUBE_UVS);
	gResources->cubeMesh->SetIndices(CUBE_INDICES);
	gResources->cubeMesh->SetSubMeshes(std::vector{ Mesh::SubMeshData{ 0, 0, static_cast<int>(CUBE_INDICES.size()) } });
	gResources->cubeMesh->ValidateAndUpdate();

	gResources->planeMesh = std::make_unique<Mesh>();
	gResources->planeMesh->SetGuid(Guid{ 0, 1 });
	gResources->planeMesh->SetName("Plane");
	gResources->planeMesh->SetPositions(QUAD_POSITIONS);
	gResources->planeMesh->SetNormals(QUAD_NORMALS);
	gResources->planeMesh->SetUVs(QUAD_UVS);
	gResources->planeMesh->SetIndices(QUAD_INDICES);
	gResources->planeMesh->SetSubMeshes(std::vector{ Mesh::SubMeshData{ 0, 0, static_cast<int>(QUAD_INDICES.size()) } });
	gResources->planeMesh->ValidateAndUpdate();
}


auto CreateStructuredBuffers() -> void {
	D3D11_BUFFER_DESC constexpr lightSbDesc{
		.ByteWidth = MAX_LIGHT_COUNT * sizeof(ShaderLight),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
		.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		.StructureByteStride = sizeof(ShaderLight)
	};

	if (FAILED(gResources->device->CreateBuffer(&lightSbDesc, nullptr, gResources->lightSB.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create light structured buffer." };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC constexpr lightSbSrvDesc{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D11_SRV_DIMENSION_BUFFER,
		.Buffer = {
			.FirstElement = 0,
			.NumElements = MAX_LIGHT_COUNT
		}
	};

	if (FAILED(gResources->device->CreateShaderResourceView(gResources->lightSB.Get(), &lightSbSrvDesc, gResources->lightSbSrv.GetAddressOf()))) {
		throw std::runtime_error{ "Failed to create light SB SRV." };
	}
}


auto DrawMeshes(std::span<int const> const meshComponentIndices, bool const useMaterials) noexcept -> void {
	for (auto const meshComponentIdx : meshComponentIndices) {
		auto const meshComponent{ gStaticMeshComponents[meshComponentIdx] };
		auto const& mesh{ meshComponent->GetMesh() };

		ID3D11Buffer* vertexBuffers[]{ mesh.GetPositionBuffer().Get(), mesh.GetNormalBuffer().Get(), mesh.GetUVBuffer().Get() };
		UINT constexpr strides[]{ sizeof(Vector3), sizeof(Vector3), sizeof(Vector2) };
		UINT constexpr offsets[]{ 0, 0, 0 };
		gResources->context->IASetVertexBuffers(0, 3, vertexBuffers, strides, offsets);
		gResources->context->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
		gResources->context->IASetInputLayout(gResources->meshIL.Get());

		D3D11_MAPPED_SUBRESOURCE mappedPerModelCBuf;
		gResources->context->Map(gResources->perModelCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerModelCBuf);
		auto& [modelMatData, normalMatData]{ *static_cast<PerModelCB*>(mappedPerModelCBuf.pData) };
		modelMatData = meshComponent->GetEntity()->GetTransform().GetModelMatrix();
		normalMatData = Matrix4{ meshComponent->GetEntity()->GetTransform().GetNormalMatrix() };
		gResources->context->Unmap(gResources->perModelCB.Get(), 0);

		gResources->context->VSSetConstantBuffers(CB_SLOT_PER_MODEL, 1, gResources->perModelCB.GetAddressOf());
		gResources->context->PSSetConstantBuffers(CB_SLOT_PER_MODEL, 1, gResources->perModelCB.GetAddressOf());

		auto const subMeshes{ mesh.GetSubMeshes() };
		auto const& materials{ meshComponent->GetMaterials() };

		for (int i = 0; i < static_cast<int>(subMeshes.size()); i++) {
			auto const& [baseVertex, firstIndex, indexCount]{ subMeshes[i] };

			if (useMaterials) {
				auto const& mtl{ static_cast<int>(materials.size()) > i ? *materials[i] : *gResources->defaultMaterial };

				auto const mtlBuffer{ mtl.GetBuffer() };
				gResources->context->VSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);
				gResources->context->PSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);

				auto const albedoSrv{ mtl.GetAlbedoMap() ? mtl.GetAlbedoMap()->GetSrv() : nullptr };
				gResources->context->PSSetShaderResources(TEX_SLOT_ALBEDO_MAP, 1, &albedoSrv);

				auto const metallicSrv{ mtl.GetMetallicMap() ? mtl.GetMetallicMap()->GetSrv() : nullptr };
				gResources->context->PSSetShaderResources(TEX_SLOT_METALLIC_MAP, 1, &metallicSrv);

				auto const roughnessSrv{ mtl.GetRoughnessMap() ? mtl.GetRoughnessMap()->GetSrv() : nullptr };
				gResources->context->PSSetShaderResources(TEX_SLOT_ROUGHNESS_MAP, 1, &roughnessSrv);

				auto const aoSrv{ mtl.GetAoMap() ? mtl.GetAoMap()->GetSrv() : nullptr };
				gResources->context->PSSetShaderResources(TEX_SLOT_AO_MAP, 1, &aoSrv);
			}

			gResources->context->DrawIndexed(indexCount, firstIndex, baseVertex);
		}
	}
}


auto UpdatePerFrameCB() noexcept -> void {
	/*D3D11_MAPPED_SUBRESOURCE mappedPerFrameCB;
	gResources->context->Map(gResources->perFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerFrameCB);

	auto const perFrameCBData{ static_cast<PerFrameCBuffer*>(mappedPerFrameCB.pData) };
	gResources->context->Unmap(gResources->perFrameCB.Get(), 0);*/
}


auto DoToneMapGammaCorrectionStep(ID3D11ShaderResourceView* const src, ID3D11RenderTargetView* const dst) noexcept -> void {
	// Back up old views to restore later.

	ComPtr<ID3D11RenderTargetView> rtvBackup;
	ComPtr<ID3D11DepthStencilView> dsvBackup;
	ComPtr<ID3D11ShaderResourceView> srvBackup;
	gResources->context->OMGetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.GetAddressOf());
	gResources->context->PSGetShaderResources(0, 1, srvBackup.GetAddressOf());

	// Do the step

	D3D11_MAPPED_SUBRESOURCE mappedToneMapGammaCB;
	gResources->context->Map(gResources->toneMapGammaCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedToneMapGammaCB);
	auto const toneMapGammaCBData{ static_cast<ToneMapGammaCB*>(mappedToneMapGammaCB.pData) };
	toneMapGammaCBData->invGamma = gInvGamma;
	gResources->context->Unmap(gResources->toneMapGammaCB.Get(), 0);

	gResources->context->VSSetShader(gResources->screenVS.Get(), nullptr, 0);
	gResources->context->PSSetShader(gResources->toneMapGammaPS.Get(), nullptr, 0);

	gResources->context->OMSetRenderTargets(1, &dst, nullptr);

	gResources->context->PSSetConstantBuffers(CB_SLOT_TONE_MAP_GAMMA, 1, gResources->toneMapGammaCB.GetAddressOf());
	gResources->context->PSSetShaderResources(TEX_SLOT_TONE_MAP_SRC, 1, &src);

	gResources->context->Draw(6, 0);

	// Restore old view bindings to that we don't leave any input/output conflicts behind.

	gResources->context->PSSetShaderResources(TEX_SLOT_TONE_MAP_SRC, 1, srvBackup.GetAddressOf());
	gResources->context->OMSetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.Get());
}


auto DrawSkybox(Matrix4 const& camViewMtx, Matrix4 const& camProjMtx) noexcept -> void {
	if (gSkyboxes.empty()) {
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mappedSkyboxCB;
	gResources->context->Map(gResources->skyboxCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSkyboxCB);
	auto const skyboxCBData{ static_cast<SkyboxCB*>(mappedSkyboxCB.pData) };
	skyboxCBData->skyboxViewProjMtx = Matrix4{ Matrix3{ camViewMtx } } * camProjMtx;
	gResources->context->Unmap(gResources->skyboxCB.Get(), 0);

	ID3D11Buffer* const vertexBuffer{ gResources->cubeMesh->GetPositionBuffer().Get() };
	UINT constexpr stride{ sizeof(Vector3) };
	UINT constexpr offset{ 0 };
	gResources->context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	gResources->context->IASetIndexBuffer(gResources->cubeMesh->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
	gResources->context->IASetInputLayout(gResources->skyboxIL.Get());

	gResources->context->VSSetShader(gResources->skyboxVS.Get(), nullptr, 0);
	gResources->context->PSSetShader(gResources->skyboxPS.Get(), nullptr, 0);

	auto const cubemapSrv{ gSkyboxes[0]->GetCubemap()->GetSrv() };
	gResources->context->PSSetShaderResources(TEX_SLOT_SKYBOX_CUBEMAP, 1, &cubemapSrv);

	auto const cb{ gResources->skyboxCB.Get() };
	gResources->context->VSSetConstantBuffers(CB_SLOT_SKYBOX_PASS, 1, &cb);

	gResources->context->OMSetDepthStencilState(gResources->skyboxPassDSS.Get(), 0);
	gResources->context->RSSetState(gResources->skyboxPassRS.Get());

	gResources->context->DrawIndexed(clamp_cast<UINT>(CUBE_INDICES.size()), 0, 0);

	// Restore state
	gResources->context->OMSetDepthStencilState(nullptr, 0);
	gResources->context->RSSetState(nullptr);
}


auto DrawShadowMaps(Visibility const& visibility, ShadowAtlasAllocation const& alloc) -> void {
	gResources->context->OMSetRenderTargets(0, nullptr, gResources->punctualShadowAtlas.dsv.Get());
	gResources->context->ClearDepthStencilView(gResources->punctualShadowAtlas.dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	gResources->context->VSSetShader(gResources->shadowVS.Get(), nullptr, 0);
	gResources->context->PSSetShader(nullptr, nullptr, 0);
	gResources->context->VSSetConstantBuffers(CB_SLOT_SHADOW_PASS, 1, gResources->shadowCB.GetAddressOf());

	for (int i = 0; i < 4; i++) {
		auto const cells{ alloc.GetQuadrantCells(i) };
		auto const cellSize{ alloc.GetQuadrantCellSizeNormalized(i) * static_cast<float>(alloc.GetSize()) };

		for (int j = 0; j < static_cast<int>(cells.size()); j++) {
			if (cells[j]) {
				auto const cellOffset{ alloc.GetCellOffsetNormalized(i, j) * static_cast<float>(alloc.GetSize()) };

				D3D11_VIEWPORT const viewport{
					.TopLeftX = cellOffset[0],
					.TopLeftY = cellOffset[1],
					.Width = static_cast<float>(cellSize),
					.Height = static_cast<float>(cellSize),
					.MinDepth = 0,
					.MaxDepth = 1
				};

				gResources->context->RSSetViewports(1, &viewport);

				D3D11_MAPPED_SUBRESOURCE mapped;
				gResources->context->Map(gResources->shadowCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
				static_cast<ShadowCB*>(mapped.pData)->lightViewProjMtx = cells[j]->lightViewProjMtx;
				gResources->context->Unmap(gResources->shadowCB.Get(), 0);

				auto const lightFrustum{
					[](LightComponent const* const light) -> Frustum {
						switch (light->GetType()) {
						case LightComponent::Type::Spot: {
							float constexpr nearClipPlane{ 0.1f };
							float const farClipPlane{ light->GetRange() };
							auto const radius{ std::tan(light->GetOuterAngle()) * light->GetRange() };

							return Frustum{
								.rightTopNear = Vector3{ radius, radius, nearClipPlane },
								.leftTopNear = Vector3{ -radius, radius, nearClipPlane },
								.leftBottomNear = Vector3{ -radius, -radius, nearClipPlane },
								.rightBottomNear = Vector3{ radius, -radius, nearClipPlane },
								.rightTopFar = Vector3{ radius, radius, farClipPlane },
								.leftTopFar = Vector3{ -radius, radius, farClipPlane },
								.leftBottomFar = Vector3{ -radius, -radius, farClipPlane },
								.rightBottomFar = Vector3{ radius, -radius, farClipPlane }
							};
						}

						default: {
							return {};
						}
						}
					}(gLights[visibility.lightIndices[cells[j]->visibleLightIdxIdx]])
				};

				Visibility static perLightVisibility;

				CullStaticMeshComponents(lightFrustum, cells[j]->lightViewMtx, perLightVisibility);
				DrawMeshes(perLightVisibility.staticMeshIndices, false);
			}
		}
	}
}


auto CalculatePunctualShadowAtlasAllocation(Visibility const& visibility, Vector3 const& camPos, Matrix4 const& camViewProjMtx, ShadowAtlasAllocation& alloc) -> void {
	std::array<std::vector<int>, 4> static lightIndexIndicesInQuadrant{};

	for (auto& quadrantLights : lightIndexIndicesInQuadrant) {
		quadrantLights.clear();
	}

	for (int i = 0; i < static_cast<int>(visibility.lightIndices.size()); i++) {
		if (auto const light{ gLights[visibility.lightIndices[i]] }; light->IsCastingShadow()) {
			switch (light->GetType()) {
			case LightComponent::Type::Directional: {
				break;
			}

			case LightComponent::Type::Spot: {
				auto const range{ light->GetRange() };
				auto const boundXY{ std::tan(ToRadians(light->GetOuterAngle())) * range };
				std::array boundVertices{
					Vector4{ boundXY, boundXY, 0.1f, 1 },
					Vector4{ -boundXY, boundXY, 0.1f, 1 },
					Vector4{ boundXY, -boundXY, 0.1f, 1 },
					Vector4{ -boundXY, -boundXY, 0.1f, 1 },
					Vector4{ boundXY, boundXY, range, 1 },
					Vector4{ -boundXY, boundXY, range, 1 },
					Vector4{ boundXY, -boundXY, range, 1 },
					Vector4{ -boundXY, -boundXY, range, 1 },
				};

				auto const mvp{ light->GetEntity()->GetTransform().GetModelMatrix() * camViewProjMtx };
				Vector2 const bottomLeft{ -1, -1 };
				Vector2 const topRight{ 1, 1 };

				Vector2 min{ bottomLeft };
				Vector2 max{ topRight };

				for (auto& boundVertex : boundVertices) {
					boundVertex *= mvp;
					auto const projected{ Vector2{ boundVertex } / boundVertex[3] };
					min = Min(min, projected);
					max = Max(max, projected);
					min = Clamp(min, bottomLeft, topRight);
					max = Clamp(max, bottomLeft, topRight);
				}

				auto const width{ max[0] - min[0] };
				auto const height{ max[1] - min[1] };

				auto const area{ width * height };
				auto const coverage{ area / 4 };

				std::optional<int> quadrantIdx;

				if (coverage >= 1) {
					quadrantIdx = 0;
				}
				else if (coverage >= 0.25f) {
					quadrantIdx = 1;
				}
				else if (coverage >= 0.0625f) {
					quadrantIdx = 2;
				}
				else if (coverage >= 0.015625f) {
					quadrantIdx = 3;
				}

				if (quadrantIdx) {
					lightIndexIndicesInQuadrant[*quadrantIdx].emplace_back(i);
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
		std::ranges::sort(lightIndexIndicesInQuadrant[i], [&visibility, &camPos](int const leftIdxIdx, int const rightIdxIdx) {
			auto const leftLight{ gLights[visibility.lightIndices[leftIdxIdx]] };
			auto const rightLight{ gLights[visibility.lightIndices[rightIdxIdx]] };

			auto const leftLightPos{ leftLight->GetEntity()->GetTransform().GetWorldPosition() };
			auto const rightLightPos{ rightLight->GetEntity()->GetTransform().GetWorldPosition() };

			auto const leftDist{ Distance(leftLightPos, camPos) };
			auto const rightDist{ Distance(rightLightPos, camPos) };

			return leftDist > rightDist;
		});

		for (auto& cell : newAlloc.GetQuadrantCells(i)) {
			if (lightIndexIndicesInQuadrant[i].empty()) {
				break;
			}

			auto const lightIdxIdx{ lightIndexIndicesInQuadrant[i].back() };
			auto const light{ gLights[visibility.lightIndices[lightIdxIdx]] };
			lightIndexIndicesInQuadrant[i].pop_back();

			auto const lightViewMtx{
				Matrix4::LookToLH(light->GetEntity()->GetTransform().GetWorldPosition(),
				                  light->GetEntity()->GetTransform().GetForwardAxis(),
				                  Vector3::Up())
			};
			auto const lightProjMtx{
				Matrix4::PerspectiveAsymZLH(ToRadians(light->GetOuterAngle() * 2),
				                            1.f,
				                            0.1f,
				                            light->GetRange())
			};

			cell.emplace(lightViewMtx, lightViewMtx * lightProjMtx, lightIdxIdx);
		}

		if (i + 1 < 4) {
			std::ranges::copy(lightIndexIndicesInQuadrant[i], std::back_inserter(lightIndexIndicesInQuadrant[i + 1]));
		}
	}

	alloc = newAlloc;
}


auto DrawFullWithCameras(std::span<Camera const* const> const cameras, ID3D11RenderTargetView* const rtv, ID3D11DepthStencilView* const dsv, ID3D11ShaderResourceView* const srv, ID3D11RenderTargetView* const outRtv) noexcept -> void {
	FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
	gResources->context->ClearRenderTargetView(rtv, clearColor);
	gResources->context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1, 0);

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

	gResources->context->PSSetSamplers(SAMPLER_SLOT_MATERIAL, 1, gResources->materialSS.GetAddressOf());
	gResources->context->PSSetSamplers(SAMPLER_SLOT_SHADOW, 1, gResources->shadowSS.GetAddressOf());

	UpdatePerFrameCB();
	gResources->context->VSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, gResources->perFrameCB.GetAddressOf());
	gResources->context->PSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, gResources->perFrameCB.GetAddressOf());

	for (auto const cam : cameras) {
		auto const camPos{ cam->GetPosition() };
		auto const camViewMtx{ cam->CalculateViewMatrix() };
		auto const camProjMtx{ cam->CalculateProjectionMatrix(aspectRatio) };
		auto const camViewProjMtx{ camViewMtx * camProjMtx };
		auto const camFrust{ cam->CalculateFrustum(aspectRatio) };

		Visibility static visibility;
		CullLights(camFrust, camViewMtx, visibility);
		auto const lightCount{ std::min(MAX_LIGHT_COUNT, static_cast<int>(visibility.lightIndices.size())) };

		CalculatePunctualShadowAtlasAllocation(visibility, camPos, camViewProjMtx, gPunctualShadowAtlasAlloc);
		ID3D11ShaderResourceView* const nullSrv{ nullptr };
		gResources->context->PSSetShaderResources(TEX_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, &nullSrv);
		DrawShadowMaps(visibility, gPunctualShadowAtlasAlloc);

		CullStaticMeshComponents(camFrust, camViewMtx, visibility);

		gResources->context->VSSetShader(gResources->meshVS.Get(), nullptr, 0);

		D3D11_MAPPED_SUBRESOURCE mappedPerCamCBuf;
		gResources->context->Map(gResources->perCamCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerCamCBuf);
		auto const perCamCBufData{ static_cast<PerCameraCB*>(mappedPerCamCBuf.pData) };
		perCamCBufData->viewProjMtx = camViewProjMtx;
		perCamCBufData->camPos = camPos;
		perCamCBufData->lightCount = lightCount;
		gResources->context->Unmap(gResources->perCamCB.Get(), 0);

		D3D11_MAPPED_SUBRESOURCE mappedLightSB;
		gResources->context->Map(gResources->lightSB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedLightSB);
		auto const mappedLightSBData{ static_cast<ShaderLight*>(mappedLightSB.pData) };

		for (int i = 0; i < lightCount; i++) {
			mappedLightSBData[i].color = gLights[visibility.lightIndices[i]]->GetColor();
			mappedLightSBData[i].intensity = gLights[visibility.lightIndices[i]]->GetIntensity();
			mappedLightSBData[i].type = static_cast<int>(gLights[visibility.lightIndices[i]]->GetType());
			mappedLightSBData[i].direction = gLights[visibility.lightIndices[i]]->GetDirection();
			mappedLightSBData[i].isCastingShadow = false;
			mappedLightSBData[i].shadowNearPlane = gLights[visibility.lightIndices[i]]->GetShadowNearPlane();
			mappedLightSBData[i].range = gLights[visibility.lightIndices[i]]->GetRange();
			mappedLightSBData[i].innerAngleCos = std::cos(ToRadians(gLights[visibility.lightIndices[i]]->GetInnerAngle()));
			mappedLightSBData[i].outerAngleCos = std::cos(ToRadians(gLights[visibility.lightIndices[i]]->GetOuterAngle()));
			mappedLightSBData[i].position = gLights[visibility.lightIndices[i]]->GetEntity()->GetTransform().GetWorldPosition();
		}

		for (int i = 0; i < 4; i++) {
			auto const cells{ gPunctualShadowAtlasAlloc.GetQuadrantCells(i) };
			for (int j = 0; j < static_cast<int>(cells.size()); j++) {
				if (cells[j]) {
					mappedLightSBData[cells[j]->visibleLightIdxIdx].isCastingShadow = true;
					mappedLightSBData[cells[j]->visibleLightIdxIdx].lightViewProjMtx = cells[j]->lightViewProjMtx;
					mappedLightSBData[cells[j]->visibleLightIdxIdx].atlasQuadrantIdx = i;
					mappedLightSBData[cells[j]->visibleLightIdxIdx].atlasCellIdx = j;
				}
			}
		}

		gResources->context->Unmap(gResources->lightSB.Get(), 0);
		gResources->context->PSSetShaderResources(SB_SLOT_LIGHTS, 1, gResources->lightSbSrv.GetAddressOf());


		gResources->context->VSSetShader(gResources->meshVS.Get(), nullptr, 0);
		gResources->context->PSSetShader(gResources->meshPbrPS.Get(), nullptr, 0);

		gResources->context->VSSetConstantBuffers(CB_SLOT_PER_CAM, 1, gResources->perCamCB.GetAddressOf());
		gResources->context->PSSetConstantBuffers(CB_SLOT_PER_CAM, 1, gResources->perCamCB.GetAddressOf());

		gResources->context->OMSetRenderTargets(1, &rtv, dsv);
		gResources->context->PSSetShaderResources(TEX_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, gResources->punctualShadowAtlas.srv.GetAddressOf());

		gResources->context->RSSetViewports(1, &viewport);

		DrawMeshes(visibility.staticMeshIndices, true);
		DrawSkybox(camViewMtx, camProjMtx);
	}

	gResources->context->RSSetViewports(1, &viewport);
	DoToneMapGammaCorrectionStep(srv, outRtv);
}
}


auto Camera::GetNearClipPlane() const noexcept -> float {
	return mNear;
}


auto Camera::SetNearClipPlane(float const nearClipPlane) noexcept -> void {
	if (GetType() == Type::Perspective) {
		mNear = std::max(nearClipPlane, MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE);
	}
	else {
		mNear = nearClipPlane;
	}
}


auto Camera::GetFarClipPlane() const noexcept -> float {
	return mFar;
}


auto Camera::SetFarClipPlane(float const farClipPlane) noexcept -> void {
	if (GetType() == Type::Perspective) {
		mFar = std::max(farClipPlane, mNear + 0.1f);
	}
	else {
		mFar = farClipPlane;
	}
}


auto Camera::GetType() const noexcept -> Type {
	return mType;
}


auto Camera::SetType(Type const type) noexcept -> void {
	if (type == Type::Perspective) {
		mNear = std::max(mNear, MINIMUM_PERSPECTIVE_NEAR_CLIP_PLANE);
		mFar = std::max(mFar, mNear + MINIMUM_PERSPECTIVE_FAR_CLIP_PLANE_OFFSET);
	}

	mType = type;
}


auto Camera::GetHorizontalPerspectiveFov() const -> float {
	return mPerspFovHorizDeg;
}


auto Camera::SetHorizontalPerspectiveFov(float degrees) -> void {
	degrees = std::max(degrees, MINIMUM_PERSPECTIVE_HORIZONTAL_FOV);
	mPerspFovHorizDeg = degrees;
}


auto Camera::GetHorizontalOrthographicSize() const -> float {
	return mOrthoSizeHoriz;
}


auto Camera::SetHorizontalOrthographicSize(float size) -> void {
	size = std::max(size, MINIMUM_ORTHOGRAPHIC_HORIZONTAL_SIZE);
	mOrthoSizeHoriz = size;
}


auto Camera::CalculateFrustum(float const aspectRatio) const noexcept -> Frustum {
	auto const nearClipPlane{ GetNearClipPlane() };
	auto const farClipPlane{ GetFarClipPlane() };

	switch (GetType()) {
	case Type::Perspective: {
		auto const horizFov{ GetHorizontalPerspectiveFov() };


		auto const tanHalfHorizFov{ std::tan(ToRadians(horizFov) / 2.0f) };
		auto const tanHalfVertFov{ std::tan(ToRadians(HorizontalPerspectiveFovToVertical(horizFov, aspectRatio)) / 2.0f) };

		auto const xn = nearClipPlane * tanHalfHorizFov;
		auto const xf = farClipPlane * tanHalfHorizFov;
		auto const yn = nearClipPlane * tanHalfVertFov;
		auto const yf = farClipPlane * tanHalfVertFov;

		return Frustum
		{
			.rightTopNear = Vector3{ xn, yn, nearClipPlane },
			.leftTopNear = Vector3{ -xn, yn, nearClipPlane },
			.leftBottomNear = Vector3{ -xn, -yn, nearClipPlane },
			.rightBottomNear = Vector3{ xn, -yn, nearClipPlane },
			.rightTopFar = Vector3{ xf, yf, farClipPlane },
			.leftTopFar = Vector3{ -xf, yf, farClipPlane },
			.leftBottomFar = Vector3{ -xf, -yf, farClipPlane },
			.rightBottomFar = Vector3{ xf, -yf, farClipPlane },
		};
	}

	case Type::Orthographic: {
		auto static constexpr half = 1.f / 2.f;
		auto const horizSize{ GetHorizontalOrthographicSize() };
		auto const x = horizSize * half;
		auto const y = horizSize / aspectRatio * half;
		return Frustum
		{
			.rightTopNear = Vector3{ x, y, nearClipPlane },
			.leftTopNear = Vector3{ -x, y, nearClipPlane },
			.leftBottomNear = Vector3{ -x, -y, nearClipPlane },
			.rightBottomNear = Vector3{ x, -y, nearClipPlane },
			.rightTopFar = Vector3{ x, y, farClipPlane },
			.leftTopFar = Vector3{ -x, y, farClipPlane },
			.leftBottomFar = Vector3{ -x, -y, farClipPlane },
			.rightBottomFar = Vector3{ x, -y, farClipPlane },
		};
	}
	}

	return {};
}


auto Camera::CalculateViewMatrix() const noexcept -> Matrix4 {
	return Matrix4::LookToLH(GetPosition(), GetForwardAxis(), Vector3::Up());
}


auto Camera::CalculateProjectionMatrix(float const aspectRatio) const noexcept -> Matrix4 {
	switch (GetType()) {
	case Type::Perspective:
		return Matrix4::PerspectiveAsymZLH(ToRadians(Camera::HorizontalPerspectiveFovToVertical(GetHorizontalPerspectiveFov(), aspectRatio)), aspectRatio, GetNearClipPlane(), GetFarClipPlane());

	case Type::Orthographic:
		return Matrix4::OrthographicAsymZLH(GetHorizontalOrthographicSize(), GetHorizontalOrthographicSize() / aspectRatio, GetNearClipPlane(), GetFarClipPlane());
	}

	return Matrix4{};
}


auto Camera::HorizontalPerspectiveFovToVertical(float const fovDegrees, float const aspectRatio) noexcept -> float {
	return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fovDegrees) / 2.0f) / aspectRatio));
}


auto Camera::VerticalPerspectiveFovToHorizontal(float const fovDegrees, float const aspectRatio) noexcept -> float {
	return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fovDegrees) / 2.0f) * aspectRatio));
}


auto StartUp() -> void {
	gResources = new Resources{};

	CreateDeviceAndContext();

#ifndef NDEBUG
	SetDebugBreaks();
#endif

	ComPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(gResources->device.As(&dxgiDevice))) {
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

	gGameRes = gWindow.GetCurrentClientAreaSize();
	gGameAspect = static_cast<f32>(gGameRes.width) / static_cast<f32>(gGameRes.height);

	gSceneRes = gGameRes;
	gSceneAspect = gGameAspect;

	RecreateGameTexturesAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);
	RecreateSceneTexturesAndViews(gWindow.GetCurrentClientAreaSize().width, gWindow.GetCurrentClientAreaSize().height);

	CreateSwapChain(dxgiFactory2.Get());
	CreateInputLayouts();
	CreateShaders();
	CreateConstantBuffers();
	CreateRasterizerStates();
	CreateDepthStencilStates();
	CreateShadowAtlases();
	CreateSamplerStates();
	CreateDefaultAssets();
	CreateStructuredBuffers();

	gWindow.OnWindowSize.add_handler(&on_window_resize);

	dxgiFactory2->MakeWindowAssociation(gWindow.GetHandle(), DXGI_MWA_NO_WINDOW_CHANGES);

	gResources->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


auto ShutDown() noexcept -> void {
	delete gResources;
	gResources = nullptr;
}


auto DrawGame() noexcept -> void {
	DrawFullWithCameras(gGameRenderCameras, gResources->gameHdrTextureRtv.Get(), gResources->gameDSV.Get(), gResources->gameHdrTextureSrv.Get(), gResources->gameOutputTextureRtv.Get());
}


auto DrawSceneView(Camera const& cam) noexcept -> void {
	auto const camPtr{ &cam };
	DrawFullWithCameras(std::span{ &camPtr, 1 }, gResources->sceneHdrTextureRtv.Get(), gResources->sceneDSV.Get(), gResources->sceneHdrTextureSrv.Get(), gResources->sceneOutputTextureRtv.Get());
}


auto GetGameResolution() noexcept -> Extent2D<u32> {
	return gGameRes;
}


auto SetGameResolution(Extent2D<u32> const resolution) noexcept -> void {
	gGameRes = resolution;
	gGameAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
	RecreateGameTexturesAndViews(gGameRes.width, gGameRes.height);
}


auto GetSceneResolution() noexcept -> Extent2D<u32> {
	return gSceneRes;
}


auto SetSceneResolution(Extent2D<u32> const resolution) noexcept -> void {
	gSceneRes = resolution;
	gSceneAspect = static_cast<f32>(resolution.width) / static_cast<f32>(resolution.height);
	RecreateSceneTexturesAndViews(gSceneRes.width, gSceneRes.height);
}


auto GetGameFrame() noexcept -> ID3D11ShaderResourceView* {
	return gResources->gameOutputTextureSrv.Get();
}


auto GetSceneFrame() noexcept -> ID3D11ShaderResourceView* {
	return gResources->sceneOutputTextureSrv.Get();
}


auto GetGameAspectRatio() noexcept -> f32 {
	return gGameAspect;
}


auto GetSceneAspectRatio() noexcept -> f32 {
	return gSceneAspect;
}


auto BindAndClearSwapChain() noexcept -> void {
	FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
	gResources->context->ClearRenderTargetView(gResources->swapChainRtv.Get(), clearColor);
	gResources->context->OMSetRenderTargets(1, gResources->swapChainRtv.GetAddressOf(), nullptr);
}


auto Present() noexcept -> void {
	gResources->swapChain->Present(gSyncInterval, gSyncInterval ? gPresentFlags & ~DXGI_PRESENT_ALLOW_TEARING : gPresentFlags);
}


auto GetSyncInterval() noexcept -> u32 {
	return gSyncInterval;
}


auto SetSyncInterval(u32 const interval) noexcept -> void {
	gSyncInterval = interval;
}


auto RegisterStaticMesh(StaticMeshComponent const* const staticMesh) -> void {
	gStaticMeshComponents.emplace_back(staticMesh);
}


auto UnregisterStaticMesh(StaticMeshComponent const* const staticMesh) -> void {
	std::erase(gStaticMeshComponents, staticMesh);
}


auto GetDevice() noexcept -> ID3D11Device* {
	return gResources->device.Get();
}


auto GetImmediateContext() noexcept -> ID3D11DeviceContext* {
	return gResources->context.Get();
}


auto RegisterLight(LightComponent const* light) -> void {
	gLights.emplace_back(light);
}


auto UnregisterLight(LightComponent const* light) -> void {
	std::erase(gLights, light);
}


auto GetDefaultMaterial() noexcept -> Material* {
	return gResources->defaultMaterial.get();
}


auto GetCubeMesh() noexcept -> Mesh* {
	return gResources->cubeMesh.get();
}


auto GetPlaneMesh() noexcept -> Mesh* {
	return gResources->planeMesh.get();
}


auto GetGamma() noexcept -> f32 {
	return 1.f / gInvGamma;
}


auto SetGamma(f32 const gamma) noexcept -> void {
	gInvGamma = 1.f / gamma;
}


auto RegisterSkybox(SkyboxComponent const* const skybox) -> void {
	gSkyboxes.emplace_back(skybox);
}


auto UnregisterSkybox(SkyboxComponent const* const skybox) -> void {
	std::erase(gSkyboxes, skybox);
}


auto RegisterGameCamera(Camera const& cam) -> void {
	gGameRenderCameras.emplace_back(&cam);
}


auto UnregisterGameCamera(Camera const& cam) -> void {
	std::erase(gGameRenderCameras, &cam);
}


auto CullLights(Frustum const& frust, Matrix4 const& viewMtx, Visibility& visibility) -> void {
	visibility.lightIndices.clear();

	for (int lightIdx = 0; lightIdx < static_cast<int>(gLights.size()); lightIdx++) {
		switch (auto const light{ gLights[lightIdx] }; light->GetType()) {
		case LightComponent::Type::Directional: {
			visibility.lightIndices.emplace_back(lightIdx);
			break;
		}

		case LightComponent::Type::Spot: {
			auto const range{ light->GetRange() };
			auto const boundXY{ std::tan(ToRadians(light->GetOuterAngle())) * range };
			std::array const boundVertices{
				Vector3{ 0, 0, 0.1f },
				Vector3{ boundXY, boundXY, range },
				Vector3{ -boundXY, boundXY, range },
				Vector3{ boundXY, -boundXY, range },
				Vector3{ -boundXY, -boundXY, range },
			};

			auto const bounds{
				[](std::span<Vector3 const> const vertices) {
					AABB ret{
						.min = Vector3{ std::numeric_limits<float>::max() },
						.max = Vector3{ std::numeric_limits<float>::lowest() }
					};

					for (auto const& vertex : vertices) {
						ret.min = Min(ret.min, vertex);
						ret.max = Max(ret.max, vertex);
					}

					return ret;
				}(boundVertices)
			};

			if (is_aabb_in_frustum(bounds, frust, light->GetEntity()->GetTransform().GetModelMatrix() * viewMtx)) {
				visibility.lightIndices.emplace_back(lightIdx);
			}

			break;
		}

		case LightComponent::Type::Point: {
			auto const range{ light->GetRange() };
			auto const boundsOffset{ Normalized(Vector3{ 1, 1, 1 }) * range };

			AABB const bounds{
				.min = -boundsOffset,
				.max = boundsOffset,
			};

			if (is_aabb_in_frustum(bounds, frust, light->GetEntity()->GetTransform().GetModelMatrix() * viewMtx)) {
				visibility.lightIndices.emplace_back(lightIdx);
			}
			break;
		}
		}
	}
}


auto CullStaticMeshComponents(Frustum const& frust, Matrix4 const& viewMtx, Visibility& visibility) -> void {
	visibility.staticMeshIndices.clear();

	for (int i = 0; i < static_cast<int>(gStaticMeshComponents.size()); i++) {
		if (is_aabb_in_frustum(gStaticMeshComponents[i]->GetMesh().GetBounds(), frust, gStaticMeshComponents[i]->GetEntity()->GetTransform().GetModelMatrix() * viewMtx)) {
			visibility.staticMeshIndices.emplace_back(i);
		}
	}
}
}
