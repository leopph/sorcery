#pragma once

#include "Camera.hpp"
#include "RenderTarget.hpp"
#include "Visibility.hpp"
#include "../Core.hpp"
#include "../Util.hpp"
#include "../SceneObjects/StaticMeshComponent.hpp"
#include "../SceneObjects/LightComponents.hpp"
#include "../SceneObjects/SkyboxComponent.hpp"
#include "../shaders/ShadowFilteringModes.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>


namespace sorcery {
enum class ShadowFilteringMode : int {
  None        = SHADOW_FILTERING_NONE,
  HardwarePCF = SHADOW_FILTERING_HARDWARE_PCF,
  PCF3x3      = SHADOW_FILTERING_PCF_3x3,
  PCFTent3x3  = SHADOW_FILTERING_PCF_TENT_3x3,
  PCFTent5x5  = SHADOW_FILTERING_PCF_TENT_5x5,
  PCSS        = SHADOW_FILTERING_PCSS
};


enum class MSAAMode : int {
  Off = 0,
  X2  = 2,
  X4  = 4,
  X8  = 8
};


class Renderer {
  class Impl;
  Impl* mImpl{nullptr};

public:
  LEOPPHAPI auto StartUp() -> void;
  LEOPPHAPI auto ShutDown() -> void;

  LEOPPHAPI auto DrawCamera(Camera const& cam, RenderTarget const* rt = nullptr) -> void;
  LEOPPHAPI auto DrawAllCameras(RenderTarget const* rt = nullptr) -> void;
  LEOPPHAPI auto DrawGizmos(RenderTarget const* rt = nullptr) -> void;
  LEOPPHAPI auto ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> ctx) noexcept -> void;
  LEOPPHAPI auto BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> ctx) noexcept -> void;
  LEOPPHAPI auto Present() noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSyncInterval() noexcept -> u32;
  LEOPPHAPI auto SetSyncInterval(u32 interval) noexcept -> void;

  LEOPPHAPI auto RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;
  LEOPPHAPI auto UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;

  [[nodiscard]] LEOPPHAPI auto GetDevice() noexcept -> ID3D11Device*;
  [[nodiscard]] LEOPPHAPI auto GetThreadContext() noexcept -> ObserverPtr<ID3D11DeviceContext>;
  LEOPPHAPI auto ExecuteCommandList(ObserverPtr<ID3D11CommandList> cmdList) noexcept -> void;

  LEOPPHAPI auto RegisterLight(LightComponent const* light) -> void;
  LEOPPHAPI auto UnregisterLight(LightComponent const* light) -> void;

  [[nodiscard]] LEOPPHAPI auto GetDefaultMaterial() const noexcept -> ObserverPtr<Material>;
  [[nodiscard]] LEOPPHAPI auto GetCubeMesh() const noexcept -> ObserverPtr<Mesh>;
  [[nodiscard]] LEOPPHAPI auto GetPlaneMesh() const noexcept -> ObserverPtr<Mesh>;

  [[nodiscard]] LEOPPHAPI auto GetGamma() const noexcept -> f32;
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

  [[nodiscard]] LEOPPHAPI auto GetShadowFilteringMode() noexcept -> ShadowFilteringMode;
  LEOPPHAPI auto SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetInFlightFrameCount() noexcept -> int;
  LEOPPHAPI auto SetInFlightFrameCount(int count) -> void;
  constexpr static int MIN_IN_FLIGHT_FRAME_COUNT{1};
  constexpr static int MAX_IN_FLIGHT_FRAME_COUNT{16};

  [[nodiscard]] LEOPPHAPI auto GetMsaaMode() const noexcept -> MSAAMode;
  LEOPPHAPI auto SetMsaaMode(MSAAMode mode) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAmbientLightColor() const noexcept -> Vector3 const&;
  LEOPPHAPI auto SetAmbientLightColor(Vector3 const& color) noexcept -> void;
};


LEOPPHAPI extern Renderer gRenderer;
}
