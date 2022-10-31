#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "Core.hpp"
#include "Components.hpp"
#include "Util.hpp"

#include <d3d11.h>
#include <dxgi1_2.h>

#include <wrl/client.h>

#include <memory>
#include <vector>

namespace leopph::rendering
{
	LEOPPHAPI [[nodiscard]] bool InitRenderer();
	LEOPPHAPI void CleanupRenderer();

	LEOPPHAPI void Present();
	LEOPPHAPI bool Render();

	[[nodiscard]] u32 GetSyncInterval();
	void SetSyncInterval(u32 interval);

	void RegisterCubeModel(CubeModel const* cubeModel);
	void UnregisterCubeModel(CubeModel const* cubeModel);

	LEOPPHAPI [[nodiscard]] ID3D11Device* GetDevice();
	LEOPPHAPI [[nodiscard]] ID3D11DeviceContext* GetImmediateContext();

	LEOPPHAPI [[nodiscard]] NormalizedViewport const& GetNormalizedViewport();
	LEOPPHAPI void SetNormalizedViewport(NormalizedViewport const& nvp);
}