#pragma once

#include "Core.hpp"
#include "Components.hpp"
#include "Util.hpp"
#include "Platform.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>


namespace leopph::rendering
{
	LEOPPHAPI auto InitRenderer(std::shared_ptr<Window> window) -> void;
	LEOPPHAPI auto CleanupRenderer() noexcept -> void;
	
	LEOPPHAPI auto DrawCamera(Camera const* cam) -> void;
	LEOPPHAPI auto DrawGame() -> void;
	[[nodiscard]] LEOPPHAPI auto GetGameResolution() noexcept -> Extent2D<u32>;
	LEOPPHAPI auto SetGameResolution(Extent2D<u32> resolution) noexcept -> void;
	[[nodiscard]] LEOPPHAPI ID3D11ShaderResourceView* GetGameFrame();
	[[nodiscard]] LEOPPHAPI f32 GetGameAspectRatio();

	LEOPPHAPI void BindAndClearSwapChain();

	LEOPPHAPI void Present();

	[[nodiscard]] LEOPPHAPI u32 GetSyncInterval();
	LEOPPHAPI void SetSyncInterval(u32 interval);

	LEOPPHAPI void RegisterCubeModel(CubeModel const* cubeModel);
	LEOPPHAPI void UnregisterCubeModel(CubeModel const* cubeModel);

	[[nodiscard]] LEOPPHAPI ID3D11Device* GetDevice();
	[[nodiscard]] LEOPPHAPI ID3D11DeviceContext* GetImmediateContext();

	LEOPPHAPI auto RegisterDirLight(DirectionalLight const* dirLight) -> void;
	LEOPPHAPI auto UnregisterDirLight(DirectionalLight const* dirLight) -> void;

	LEOPPHAPI auto RegisterSpotLight(SpotLight const* spotLight) -> void;
	LEOPPHAPI auto UnregisterSpotLight(SpotLight const* spotLight) -> void;

	LEOPPHAPI auto RegisterPointLight(PointLight const* pointLight) -> void;
	LEOPPHAPI auto UnregisterPointLight(PointLight const* pointLight) -> void;
}