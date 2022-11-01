#pragma once

#include "Core.hpp"
#include "Components.hpp"
#include "Util.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>


namespace leopph::rendering
{
	LEOPPHAPI [[nodiscard]] bool InitRenderer();
	LEOPPHAPI void CleanupRenderer();
	
	LEOPPHAPI [[nodiscard]] bool DrawGame();
	LEOPPHAPI [[nodiscard]] Extent2D<u32> GetGameResolution();
	LEOPPHAPI void SetGameResolution(Extent2D<u32> resolution);
	LEOPPHAPI [[nodiscard]] ID3D11ShaderResourceView* GetGameFrame();
	LEOPPHAPI [[nodiscard]] f32 GetGameAspectRatio();

	LEOPPHAPI void BindAndClearSwapChain();

	LEOPPHAPI void Present();

	[[nodiscard]] u32 GetSyncInterval();
	void SetSyncInterval(u32 interval);

	void RegisterCubeModel(CubeModel const* cubeModel);
	void UnregisterCubeModel(CubeModel const* cubeModel);

	LEOPPHAPI [[nodiscard]] ID3D11Device* GetDevice();
	LEOPPHAPI [[nodiscard]] ID3D11DeviceContext* GetImmediateContext();
}