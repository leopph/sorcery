#pragma once

#include "Core.hpp"
#include "StaticMeshComponent.hpp"
#include "LightComponents.hpp"
#include "Util.hpp"
#include "SkyboxComponent.hpp"
#include "shaders/ShadowFilteringModes.h"
#include "Camera.hpp"
#include "Visibility.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>

#include "RenderTarget.hpp"


namespace sorcery {
enum class ShadowFilteringMode {
  None        = SHADOW_FILTERING_NONE,
  HardwarePCF = SHADOW_FILTERING_HARDWARE_PCF,
  PCF3x3      = SHADOW_FILTERING_PCF_3x3,
  PCFTent3x3  = SHADOW_FILTERING_PCF_TENT_3x3,
  PCFTent5x5  = SHADOW_FILTERING_PCF_TENT_5x5,
  PCSS        = SHADOW_FILTERING_PCSS
};


class Renderer {
  class Impl;
  Impl* mImpl;

public:
  Renderer() = default;
  Renderer(Renderer const&) = delete;
  Renderer(Renderer&&) = delete;

  ~Renderer() = default;

  auto operator=(Renderer const&) -> void = delete;
  auto operator=(Renderer&&) -> void = delete;

  LEOPPHAPI auto StartUp() -> void;
  LEOPPHAPI auto ShutDown() -> void;

  LEOPPHAPI auto DrawCamera(Camera const& cam, RenderTarget* rt = nullptr) -> void;
  LEOPPHAPI auto DrawAllCameras(RenderTarget* rt = nullptr) -> void;
  LEOPPHAPI auto DrawGizmos(RenderTarget const* rt = nullptr) -> void;
  LEOPPHAPI auto BindAndClearSwapChain() noexcept -> void;
  LEOPPHAPI auto Present() noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSyncInterval() noexcept -> u32;
  LEOPPHAPI auto SetSyncInterval(u32 interval) noexcept -> void;

  LEOPPHAPI auto RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;
  LEOPPHAPI auto UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;

  [[nodiscard]] LEOPPHAPI auto GetDevice() noexcept -> ID3D11Device*;
  [[nodiscard]] LEOPPHAPI auto GetImmediateContext() noexcept -> ID3D11DeviceContext*;

  LEOPPHAPI auto RegisterLight(LightComponent const* light) -> void;
  LEOPPHAPI auto UnregisterLight(LightComponent const* light) -> void;

  [[nodiscard]] LEOPPHAPI auto GetDefaultMaterial() noexcept -> Material*;
  [[nodiscard]] LEOPPHAPI auto GetCubeMesh() noexcept -> Mesh*;
  [[nodiscard]] LEOPPHAPI auto GetPlaneMesh() noexcept -> Mesh*;

  [[nodiscard]] LEOPPHAPI auto GetGamma() noexcept -> f32;
  LEOPPHAPI auto SetGamma(f32 gamma) noexcept -> void;

  LEOPPHAPI auto RegisterSkybox(SkyboxComponent const* skybox) -> void;
  LEOPPHAPI auto UnregisterSkybox(SkyboxComponent const* skybox) -> void;

  LEOPPHAPI auto RegisterGameCamera(Camera const& cam) -> void;
  LEOPPHAPI auto UnregisterGameCamera(Camera const& cam) -> void;

  LEOPPHAPI auto CullLights(Frustum const& frustumWS, Visibility& visibility) -> void;
  LEOPPHAPI auto CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) -> void;

  LEOPPHAPI auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;

  LEOPPHAPI auto GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget&;

  [[nodiscard]] LEOPPHAPI auto GetShadowCascadeCount() noexcept -> int;
  LEOPPHAPI auto SetShadowCascadeCount(int cascadeCount) noexcept -> void;
  [[nodiscard]] LEOPPHAPI auto GetMaxShadowCascadeCount() noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetNormalizedShadowCascadeSplits() noexcept -> std::span<float const>;
  LEOPPHAPI auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;
  [[nodiscard]] LEOPPHAPI auto GetShadowDistance() noexcept -> float;
  LEOPPHAPI auto SetShadowDistance(float shadowDistance) noexcept -> void;
  [[nodiscard]] LEOPPHAPI auto IsVisualizingShadowCascades() noexcept -> bool;
  LEOPPHAPI auto VisualizeShadowCascades(bool visualize) noexcept -> void;
  [[nodiscard]] LEOPPHAPI auto IsUsingStableShadowCascadeProjection() noexcept -> bool;
  LEOPPHAPI auto UseStableShadowCascadeProjection(bool useStableProj) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetShadowFilteringMode() noexcept -> ShadowFilteringMode;
  LEOPPHAPI auto SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetInFlightFrameCount() noexcept -> int;
  LEOPPHAPI auto SetInFlightFrameCount(int count) -> void;
  constexpr static int MIN_IN_FLIGHT_FRAME_COUNT{ 1 };
  constexpr static int MAX_IN_FLIGHT_FRAME_COUNT{ 16 };
};
}
