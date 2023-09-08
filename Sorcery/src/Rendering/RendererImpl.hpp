#pragma once

#include "DirectionalShadowMapArray.hpp"
#include "Graphics.hpp"
#include "PunctualShadowAtlas.hpp"
#include "Renderer.hpp"
#include "SwapChain.hpp"
#include "StructuredBuffer.hpp"
#include "../Core.hpp"
#include "../Color.hpp"
#include "../Math.hpp"
#include "../shaders/ShaderInterop.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include <array>
#include <memory>
#include <mutex>
#include <vector>


namespace sorcery {
class Renderer::Impl {
  struct TempRenderTargetRecord {
    std::unique_ptr<RenderTarget> rt;
    int ageInFrames;
  };


  constexpr static int MAX_TMP_RT_AGE{10};
  inline static Guid const DEFAULT_MATERIAL_GUID{1, 0};
  inline static Guid const CUBE_MESH_GUID{2, 0};
  inline static Guid const PLANE_MESH_GUID{3, 0};
  constexpr static Vector3 DEFAULT_AMBIENT_LIGHT_COLOR{0.03};

  Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
  Microsoft::WRL::ComPtr<ID3D11DeviceContext> mImmediateContext;

  Microsoft::WRL::ComPtr<IDXGIDevice1> mDxgiDevice;

  Microsoft::WRL::ComPtr<ID3D11PixelShader> mMeshPbrPs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> mPostProcessPs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> mSkyboxPs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> mGizmoPs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> mDepthOnlyPs;

  Microsoft::WRL::ComPtr<ID3D11VertexShader> mMeshVs;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> mSkyboxVs;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> mDepthOnlyVs;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> mScreenVs;
  Microsoft::WRL::ComPtr<ID3D11VertexShader> mLineGizmoVs;

  Microsoft::WRL::ComPtr<ID3D11Buffer> mPerFrameCb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mPerCamCb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mPerDrawCb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mPostProcessCb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mSkyboxCb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> mDepthOnlyCb;

  Microsoft::WRL::ComPtr<ID3D11InputLayout> mAllAttribsIl;

  Microsoft::WRL::ComPtr<ID3D11SamplerState> mCmpPcfGreaterEqualSs;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mCmpPcfLessEqualSs;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mCmpPointGreaterEqualSs;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mCmpPointLessEqualSs;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mAf16Ss;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mAf8Ss;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mAf4Ss;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mTrilinearSs;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mBilinearSs;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> mPointSs;

  Microsoft::WRL::ComPtr<ID3D11RasterizerState> mSkyboxPassRs;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState> mShadowPassRs;

  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthTestGreaterWriteDss;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthTestLessWriteDss;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthTestLessEqualNoWriteDss;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthTestGreaterEqualNoWriteDss;

  ObserverPtr<Material> mDefaultMaterial{nullptr};
  ObserverPtr<Mesh> mCubeMesh{nullptr};
  ObserverPtr<Mesh> mPlaneMesh{nullptr};

  //std::unique_ptr<DirectionalShadowAtlas> mDirectionalShadowAtlas;
  std::unique_ptr<DirectionalShadowMapArray> mDirShadowMapArr;
  std::unique_ptr<PunctualShadowAtlas> mPunctualShadowAtlas;
  std::unique_ptr<SwapChain> mSwapChain;
  std::unique_ptr<StructuredBuffer<ShaderLight>> mLightBuffer;
  std::unique_ptr<StructuredBuffer<Vector4>> mGizmoColorBuffer;
  std::unique_ptr<StructuredBuffer<ShaderLineGizmoVertexData>> mLineGizmoVertexDataBuffer;
  std::unique_ptr<RenderTarget> mMainRt;

  std::vector<StaticMeshComponent const*> mStaticMeshComponents;
  std::vector<SkyboxComponent const*> mSkyboxes;
  std::vector<LightComponent const*> mLights;
  std::vector<Camera const*> mGameRenderCameras;

  std::vector<Vector4> mGizmoColors;
  std::vector<ShaderLineGizmoVertexData> mLineGizmoVertexData;

  // Normalized to [0, 1]
  std::array<float, MAX_CASCADE_COUNT - 1> mCascadeSplits{0.1f, 0.3f, 0.6f};
  int mCascadeCount{4};
  float mShadowDistance{100};
  bool mVisualizeShadowCascades{false};
  ShadowFilteringMode mShadowFilteringMode{ShadowFilteringMode::PCFTent5x5};
  MultisamplingMode mMsaaMode{MultisamplingMode::X8};
  int mSyncInterval{0};
  float mInvGamma{1.f / 2.2f};
  int mInFlightFrameCount{2};

  std::unordered_map<std::thread::id, Microsoft::WRL::ComPtr<ID3D11DeviceContext>> mPerThreadCtx;

  std::mutex mPerThreadCtxMutex;
  std::mutex mImmediateCtxMutex;
  std::mutex mStaticMeshMutex;
  std::mutex mLightMutex;
  std::mutex mSkyboxMutex;
  std::mutex mGameCameraMutex;

  Vector3 mAmbientLightColor{DEFAULT_AMBIENT_LIGHT_COLOR};

  std::vector<TempRenderTargetRecord> mTmpRenderTargets;

  [[nodiscard]] constexpr auto GetSceneDrawDss() -> Microsoft::WRL::ComPtr<ID3D11DepthStencilState>&;
  [[nodiscard]] constexpr auto GetShadowDrawDss() -> Microsoft::WRL::ComPtr<ID3D11DepthStencilState>&;
  [[nodiscard]] constexpr auto GetSkyboxDrawDss() -> Microsoft::WRL::ComPtr<ID3D11DepthStencilState>&;
  [[nodiscard]] constexpr auto GetShadowPointSampler() -> Microsoft::WRL::ComPtr<ID3D11SamplerState>&;
  [[nodiscard]] constexpr auto GetShadowPcfSampler() -> Microsoft::WRL::ComPtr<ID3D11SamplerState>&;

  [[nodiscard]] auto CalculateCameraShadowCascadeBoundaries(Camera const& cam) const -> ShadowCascadeBoundaries;

  auto CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) const -> void;
  auto CullLights(Frustum const& frustumWS, Visibility& visibility) const -> void;

  auto DrawShadowMaps(ShadowAtlas const& atlas) -> void;
  auto DrawMeshes(std::span<StaticMeshSubmeshIndex const> culledIndices) noexcept -> void;
  auto DrawSkybox(Matrix4 const& camViewMtx, Matrix4 const& camProjMtx) const noexcept -> void;
  auto PostProcess(ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) noexcept -> void;

  auto ClearGizmoDrawQueue() noexcept -> void;
  auto ReleaseTempRenderTargets() noexcept -> void;

  static auto OnWindowSize(Impl* self, Extent2D<std::uint32_t> size) -> void;

public:
  // LIFETIME FUNCTIONS

  auto StartUp() -> void;
  auto ShutDown() -> void;

  // FRAME RENDERING FUNCTIONS

  auto DrawCamera(Camera const& cam, RenderTarget const* rt = nullptr) -> void;
  auto DrawAllCameras(RenderTarget const* rt = nullptr) -> void;

  auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;
  auto DrawGizmos(RenderTarget const* rt = nullptr) -> void;

  auto ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void;
  auto BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void;

  auto Present() noexcept -> void;

  // FUNCTIONS FOR CUSTOM EXTERNAL REQUESTS

  [[nodiscard]] auto GetDevice() const noexcept -> ObserverPtr<ID3D11Device>;
  [[nodiscard]] auto GetThreadContext() noexcept -> ObserverPtr<ID3D11DeviceContext>;

  auto ExecuteCommandList(ObserverPtr<ID3D11CommandList> cmdList) noexcept -> void;

  auto GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget&;

  // DEFAULT RENDERING RESOURCES

  [[nodiscard]] auto GetDefaultMaterial() const noexcept -> ObserverPtr<Material>;
  [[nodiscard]] auto GetCubeMesh() const noexcept -> ObserverPtr<Mesh>;
  [[nodiscard]] auto GetPlaneMesh() const noexcept -> ObserverPtr<Mesh>;

  // RENDER STATE CONFIGURATION

  [[nodiscard]] auto GetSyncInterval() const noexcept -> int;
  auto SetSyncInterval(int interval) noexcept -> void;

  [[nodiscard]] auto GetInFlightFrameCount() const noexcept -> int;
  auto SetInFlightFrameCount(int count) -> void;

  [[nodiscard]] auto GetMultisamplingMode() const noexcept -> MultisamplingMode;
  auto SetMultisamplingMode(MultisamplingMode mode) noexcept -> void;

  // SHADOWS

  [[nodiscard]] auto GetShadowDistance() const noexcept -> float;
  auto SetShadowDistance(float shadowDistance) noexcept -> void;

  [[nodiscard]] constexpr static auto GetMaxShadowCascadeCount() noexcept -> int;
  [[nodiscard]] auto GetShadowCascadeCount() const noexcept -> int;
  auto SetShadowCascadeCount(int cascadeCount) noexcept -> void;

  [[nodiscard]] auto GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const>;
  auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;

  [[nodiscard]] auto IsVisualizingShadowCascades() const noexcept -> bool;
  auto VisualizeShadowCascades(bool visualize) noexcept -> void;

  [[nodiscard]] auto GetShadowFilteringMode() const noexcept -> ShadowFilteringMode;
  auto SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void;

  // LIGHTING

  [[nodiscard]] auto GetAmbientLightColor() const noexcept -> Vector3 const&;
  auto SetAmbientLightColor(Vector3 const& color) noexcept -> void;

  // POST-PROCESSING

  [[nodiscard]] auto GetGamma() const noexcept -> float;
  auto SetGamma(float gamma) noexcept -> void;

  // REGISTRATION FUNCTIONS

  auto Register(StaticMeshComponent const& staticMeshComponent) noexcept -> void;
  auto Unregister(StaticMeshComponent const& staticMeshComponent) noexcept -> void;

  auto Register(LightComponent const& lightComponent) noexcept -> void;
  auto Unregister(LightComponent const& lightComponent) noexcept -> void;

  auto Register(SkyboxComponent const& skyboxComponent) noexcept -> void;
  auto Unregister(SkyboxComponent const& skyboxComponent) noexcept -> void;

  auto Register(Camera const& cam) noexcept -> void;
  auto Unregister(Camera const& cam) noexcept -> void;
};


constexpr auto Renderer::Impl::GetSceneDrawDss() -> Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mDepthTestGreaterEqualNoWriteDss;
  } else {
    return mDepthTestLessEqualNoWriteDss;
  }
}


constexpr auto Renderer::Impl::GetShadowDrawDss() -> Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mDepthTestGreaterWriteDss;
  } else {
    return mDepthTestLessWriteDss;
  }
}


constexpr auto Renderer::Impl::GetSkyboxDrawDss() -> Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& {
  return GetShadowDrawDss();
}


constexpr auto Renderer::Impl::GetShadowPointSampler() -> Microsoft::WRL::ComPtr<ID3D11SamplerState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mCmpPointGreaterEqualSs;
  } else {
    return mCmpPointLessEqualSs;
  }
}


constexpr auto Renderer::Impl::GetShadowPcfSampler() -> Microsoft::WRL::ComPtr<ID3D11SamplerState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mCmpPcfGreaterEqualSs;
  } else {
    return mCmpPcfLessEqualSs;
  }
}


constexpr auto Renderer::Impl::GetMaxShadowCascadeCount() noexcept -> int {
  return MAX_CASCADE_COUNT;
}
}
