#pragma once

#include "Core.hpp"
#include "StaticMeshComponent.hpp"
#include "LightComponents.hpp"
#include "Util.hpp"
#include "SkyboxComponent.hpp"
#include "RenderCamera.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>


namespace leopph::renderer {
LEOPPHAPI auto StartUp() -> void;
LEOPPHAPI auto ShutDown() noexcept -> void;

LEOPPHAPI auto DrawGame() noexcept -> void;
LEOPPHAPI auto DrawSceneView(RenderCamera const& cam) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetGameResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetGameResolution(Extent2D<u32> resolution) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetSceneResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetSceneResolution(Extent2D<u32> resolution) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetGameFrame() noexcept -> ID3D11ShaderResourceView*;
LEOPPHAPI [[nodiscard]] auto GetSceneFrame() noexcept -> ID3D11ShaderResourceView*;

LEOPPHAPI [[nodiscard]] auto GetGameAspectRatio() noexcept -> f32;
LEOPPHAPI [[nodiscard]] auto GetSceneAspectRatio() noexcept -> f32;

LEOPPHAPI auto BindAndClearSwapChain() noexcept -> void;

LEOPPHAPI auto Present() noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetSyncInterval() noexcept -> u32;
LEOPPHAPI auto SetSyncInterval(u32 interval) noexcept -> void;

LEOPPHAPI auto RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;
LEOPPHAPI auto UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;

LEOPPHAPI [[nodiscard]] auto GetDevice() noexcept -> ID3D11Device*;
LEOPPHAPI [[nodiscard]] auto GetImmediateContext() noexcept -> ID3D11DeviceContext*;

LEOPPHAPI auto RegisterLight(LightComponent const* light) -> void;
LEOPPHAPI auto UnregisterLight(LightComponent const* light) -> void;

LEOPPHAPI [[nodiscard]] auto GetDefaultMaterial() noexcept -> Material*;
LEOPPHAPI [[nodiscard]] auto GetCubeMesh() noexcept -> Mesh*;
LEOPPHAPI [[nodiscard]] auto GetPlaneMesh() noexcept -> Mesh*;

LEOPPHAPI [[nodiscard]] auto GetGamma() noexcept -> f32;
LEOPPHAPI auto SetGamma(f32 gamma) noexcept -> void;

LEOPPHAPI auto RegisterSkybox(SkyboxComponent const* skybox) -> void;
LEOPPHAPI auto UnregisterSkybox(SkyboxComponent const* skybox) -> void;

LEOPPHAPI auto RegisterGameCamera(RenderCamera const& cam) -> void;
LEOPPHAPI auto UnregisterGameCamera(RenderCamera const& cam) -> void;
}
