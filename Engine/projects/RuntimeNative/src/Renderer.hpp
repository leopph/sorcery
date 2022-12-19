#pragma once

#include "Core.hpp"
#include "Components.hpp"
#include "Util.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>


namespace leopph::rendering
{
	[[nodiscard]] LEOPPHAPI bool InitRenderer();
	LEOPPHAPI void CleanupRenderer();
	
	[[nodiscard]] LEOPPHAPI bool DrawCamera(Camera const* cam);
	[[nodiscard]] LEOPPHAPI bool DrawGame();
	[[nodiscard]] LEOPPHAPI Extent2D<u32> GetGameResolution();
	LEOPPHAPI void SetGameResolution(Extent2D<u32> resolution);
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