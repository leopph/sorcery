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


namespace sorcery {
class Renderer {
public:
  constexpr static int MIN_IN_FLIGHT_FRAME_COUNT{1};
  constexpr static int MAX_IN_FLIGHT_FRAME_COUNT{16};


  // Passing these enum values to shaders is valid
  enum class ShadowFilteringMode : int {
    None        = SHADOW_FILTERING_NONE,
    HardwarePCF = SHADOW_FILTERING_HARDWARE_PCF,
    PCF3x3      = SHADOW_FILTERING_PCF_3x3,
    PCFTent3x3  = SHADOW_FILTERING_PCF_TENT_3x3,
    PCFTent5x5  = SHADOW_FILTERING_PCF_TENT_5x5,
    PCSS        = SHADOW_FILTERING_PCSS
  };


  // Cast to int to get the sample count
  enum class MultisamplingMode : int {
    Off = 1,
    X2  = 2,
    X4  = 4,
    X8  = 8
  };

private:
  class Impl;
  Impl* mImpl{nullptr};

public:
  // LIFETIME FUNCTIONS

  LEOPPHAPI auto StartUp() -> void;
  LEOPPHAPI auto ShutDown() -> void;

  // FRAME RENDERING FUNCTIONS

  LEOPPHAPI auto DrawCamera(Camera const& cam, RenderTarget const* rt = nullptr) -> void;
  LEOPPHAPI auto DrawAllCameras(RenderTarget const* rt = nullptr) -> void;

  LEOPPHAPI auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;
  LEOPPHAPI auto DrawGizmos(RenderTarget const* rt = nullptr) -> void;

  LEOPPHAPI auto ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void;
  LEOPPHAPI auto BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void;

  LEOPPHAPI auto Present() noexcept -> void;

  // FUNCTIONS FOR CUSTOM EXTERNAL REQUESTS

  [[nodiscard]] LEOPPHAPI auto GetDevice() const noexcept -> ObserverPtr<ID3D11Device>;
  [[nodiscard]] LEOPPHAPI auto GetThreadContext() noexcept -> ObserverPtr<ID3D11DeviceContext>;

  LEOPPHAPI auto ExecuteCommandList(ObserverPtr<ID3D11CommandList> cmdList) noexcept -> void;

  LEOPPHAPI auto GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget&;

  // DEFAULT RENDERING RESOURCES

  [[nodiscard]] LEOPPHAPI auto GetDefaultMaterial() const noexcept -> ObserverPtr<Material>;
  [[nodiscard]] LEOPPHAPI auto GetCubeMesh() const noexcept -> ObserverPtr<Mesh>;
  [[nodiscard]] LEOPPHAPI auto GetPlaneMesh() const noexcept -> ObserverPtr<Mesh>;

  // RENDER STATE CONFIGURATION

  [[nodiscard]] LEOPPHAPI auto GetSyncInterval() const noexcept -> int;
  LEOPPHAPI auto SetSyncInterval(int interval) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetInFlightFrameCount() const noexcept -> int;
  LEOPPHAPI auto SetInFlightFrameCount(int count) -> void;

  [[nodiscard]] LEOPPHAPI auto GetMultisamplingMode() const noexcept -> MultisamplingMode;
  LEOPPHAPI auto SetMultisamplingMode(MultisamplingMode mode) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsDepthPrePassEnabled() const noexcept -> bool;
  LEOPPHAPI auto SetDepthPrePassEnabled(bool enabled) noexcept -> void;

  // SHADOWS

  [[nodiscard]] LEOPPHAPI auto GetShadowDistance() const noexcept -> float;
  LEOPPHAPI auto SetShadowDistance(float shadowDistance) noexcept -> void;

  [[nodiscard]] LEOPPHAPI static auto GetMaxShadowCascadeCount() noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetShadowCascadeCount() const noexcept -> int;
  LEOPPHAPI auto SetShadowCascadeCount(int cascadeCount) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const>;
  LEOPPHAPI auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsVisualizingShadowCascades() const noexcept -> bool;
  LEOPPHAPI auto VisualizeShadowCascades(bool visualize) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetShadowFilteringMode() const noexcept -> ShadowFilteringMode;
  LEOPPHAPI auto SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void;

  // LIGHTING

  [[nodiscard]] LEOPPHAPI auto GetAmbientLightColor() const noexcept -> Vector3 const&;
  LEOPPHAPI auto SetAmbientLightColor(Vector3 const& color) noexcept -> void;

  // POST-PROCESSING

  [[nodiscard]] LEOPPHAPI auto GetGamma() const noexcept -> float;
  LEOPPHAPI auto SetGamma(float gamma) noexcept -> void;

  // REGISTRATION FUNCTIONS

  LEOPPHAPI auto Register(StaticMeshComponent const& staticMeshComponent) noexcept -> void;
  LEOPPHAPI auto Unregister(StaticMeshComponent const& staticMeshComponent) noexcept -> void;

  LEOPPHAPI auto Register(LightComponent const& lightComponent) noexcept -> void;
  LEOPPHAPI auto Unregister(LightComponent const& lightComponent) noexcept -> void;

  LEOPPHAPI auto Register(SkyboxComponent const& skyboxComponent) noexcept -> void;
  LEOPPHAPI auto Unregister(SkyboxComponent const& skyboxComponent) noexcept -> void;

  LEOPPHAPI auto Register(Camera const& cam) noexcept -> void;
  LEOPPHAPI auto Unregister(Camera const& cam) noexcept -> void;
};


LEOPPHAPI extern Renderer gRenderer;
}
