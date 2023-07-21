#include "Renderer.hpp"

#include <dxgi1_5.h>

#include "Platform.hpp"
#include "Entity.hpp"
#include "Util.hpp"
#include "TransformComponent.hpp"
#include "Graphics.hpp"

#ifndef NDEBUG
#include "shaders/generated/MeshPbrPSBinDebug.h"
#include "shaders/generated/MeshVSBinDebug.h"
#include "shaders/generated/PostProcessPSBinDebug.h"
#include "shaders/generated/SkyboxPSBinDebug.h"
#include "shaders/generated/SkyboxVSBinDebug.h"
#include "shaders/generated/DepthOnlyVSBinDebug.h"
#include "shaders/generated/ScreenVSBinDebug.h"
#include "shaders/generated/GizmoPSBinDebug.h"
#include "shaders/generated/LineGizmoVSBinDebug.h"

#else
#include "shaders/generated/MeshPbrPSBin.h"
#include "shaders/generated/MeshVSBin.h"
#include "shaders/generated/PostProcessPSBin.h"
#include "shaders/generated/SkyboxPSBin.h"
#include "shaders/generated/SkyboxVSBin.h"
#include "shaders/generated/DepthOnlyVSBin.h"
#include "shaders/generated/ScreenVSBin.h"
#include "shaders/generated/GizmoPSBin.h"
#include "shaders/generated/LineGizmoVSBin.h"
#endif

#include "shaders/ShaderInterop.h"
#include "DirectionalShadowAtlas.hpp"
#include "PunctualShadowAtlas.hpp"
#include "RenderTarget.hpp"
#include "SwapChain.hpp"
#include "StructuredBuffer.hpp"
#include "ShadowCascadeBoundary.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <memory>

using Microsoft::WRL::ComPtr;


namespace sorcery {
Renderer gRenderer;


namespace {
std::vector const QUAD_POSITIONS{
  Vector3{ -1, 1, 0 }, Vector3{ -1, -1, 0 }, Vector3{ 1, -1, 0 }, Vector3{ 1, 1, 0 }
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
}


class Renderer::Impl {
  ComPtr<ID3D11Device> mDevice;
  ComPtr<ID3D11DeviceContext> mImmediateContext;

  ComPtr<IDXGIDevice1> mDxgiDevice;

  ComPtr<ID3D11PixelShader> mMeshPbrPs;
  ComPtr<ID3D11PixelShader> mPostProcessPs;
  ComPtr<ID3D11PixelShader> mSkyboxPs;
  ComPtr<ID3D11PixelShader> mGizmoPs;

  ComPtr<ID3D11VertexShader> mMeshVs;
  ComPtr<ID3D11VertexShader> mSkyboxVs;
  ComPtr<ID3D11VertexShader> mDepthOnlyVs;
  ComPtr<ID3D11VertexShader> mScreenVs;
  ComPtr<ID3D11VertexShader> mLineGizmoVs;

  ComPtr<ID3D11Buffer> mPerFrameCb;
  ComPtr<ID3D11Buffer> mPerCamCb;
  ComPtr<ID3D11Buffer> mPerDrawCb;
  ComPtr<ID3D11Buffer> mPostProcessCb;
  ComPtr<ID3D11Buffer> mSkyboxCb;
  ComPtr<ID3D11Buffer> mDepthOnlyCb;

  ComPtr<ID3D11InputLayout> mAllAttribsIl;
  ComPtr<ID3D11InputLayout> mPos3OnlyIl;

  ComPtr<ID3D11SamplerState> mCmpPcfGreaterEqualSs;
  ComPtr<ID3D11SamplerState> mCmpPcfLessEqualSs;
  ComPtr<ID3D11SamplerState> mCmpPointGreaterEqualSs;
  ComPtr<ID3D11SamplerState> mCmpPointLessEqualSs;
  ComPtr<ID3D11SamplerState> mAf16Ss;
  ComPtr<ID3D11SamplerState> mAf8Ss;
  ComPtr<ID3D11SamplerState> mAf4Ss;
  ComPtr<ID3D11SamplerState> mTrilinearSs;
  ComPtr<ID3D11SamplerState> mBilinearSs;
  ComPtr<ID3D11SamplerState> mPointSs;

  ComPtr<ID3D11RasterizerState> mSkyboxPassRs;
  ComPtr<ID3D11RasterizerState> mShadowPassRs;

  ComPtr<ID3D11DepthStencilState> mDepthTestGreaterWriteDss;
  ComPtr<ID3D11DepthStencilState> mDepthTestLessWriteDss;
  ComPtr<ID3D11DepthStencilState> mDepthTestLessEqualNoWriteDss;
  ComPtr<ID3D11DepthStencilState> mDepthTestGreaterEqualNoWriteDss;

  std::unique_ptr<Material> mDefaultMaterial;
  std::unique_ptr<Mesh> mCubeMesh;
  std::unique_ptr<Mesh> mPlaneMesh;

  std::unique_ptr<DirectionalShadowAtlas> mDirectionalShadowAtlas;
  std::unique_ptr<PunctualShadowAtlas> mPunctualShadowAtlas;

  std::unique_ptr<SwapChain> mSwapChain;

  std::unique_ptr<StructuredBuffer<ShaderLight>> mLightBuffer;
  std::unique_ptr<StructuredBuffer<Vector4>> mGizmoColorBuffer;
  std::unique_ptr<StructuredBuffer<ShaderLineGizmoVertexData>> mLineGizmoVertexDataBuffer;

  u32 mSyncInterval{ 0 };
  f32 mInvGamma{ 1.f / 2.2f };
  int mInFlightFrameCount{ 2 };

  std::vector<StaticMeshComponent const*> mStaticMeshComponents;
  std::vector<SkyboxComponent const*> mSkyboxes;
  std::vector<LightComponent const*> mLights;
  std::vector<Camera const*> mGameRenderCameras;

  std::vector<Vector4> mGizmoColors;
  std::vector<ShaderLineGizmoVertexData> mLineGizmoVertexData;

  // Normalized to [0, 1]
  std::array<float, MAX_CASCADE_COUNT - 1> mCascadeSplits{ 0.1f, 0.3f, 0.6f };
  int mCascadeCount{ 4 };
  float mShadowDistance{ 100 };
  bool mVisualizeShadowCascades{ false };
  bool mUseStableShadowCascadeProjection{ false };
  ShadowFilteringMode mShadowFilteringMode{ ShadowFilteringMode::PCFTent5x5 };


  struct TempRenderTargetRecord {
    std::unique_ptr<RenderTarget> rt;
    int ageInFrames;
  };


  constexpr static int MAX_TMP_RT_AGE{ 10 };
  std::vector<TempRenderTargetRecord> mTmpRenderTargets;

  [[nodiscard]] auto GetSceneDrawDssForReversedDepth() -> ComPtr<ID3D11DepthStencilState>&;
  [[nodiscard]] auto GetShadowDrawDssForReversedDepth() -> ComPtr<ID3D11DepthStencilState>&;
  [[nodiscard]] auto GetSkyboxDrawDssForReversedDepth() -> ComPtr<ID3D11DepthStencilState>&;
  [[nodiscard]] auto GetShadowPointSamplerForReversedDepth() -> ComPtr<ID3D11SamplerState>&;
  [[nodiscard]] auto GetShadowPcfSamplerForReversedDepth() -> ComPtr<ID3D11SamplerState>&;

  [[nodiscard]] auto CalculateCameraShadowCascadeBoundaries(Camera const& cam) const -> ShadowCascadeBoundaries;

  auto CreateDeviceAndContext() -> void;
  auto SetDebugBreaks() const -> void;
  auto CreateInputLayouts() -> void;
  auto CreateShaders() -> void;
  auto CreateConstantBuffers() -> void;
  auto CreateRasterizerStates() -> void;
  auto CreateDepthStencilStates() -> void;
  auto CreateShadowAtlases() -> void;
  auto CreateSamplerStates() -> void;
  auto CreateDefaultAssets() -> void;

  auto DrawShadowMaps(ShadowAtlas const& atlas) -> void;
  auto DrawMeshes(std::span<int const> meshComponentIndices, bool useMaterials) noexcept -> void;
  auto DrawSkybox(Matrix4 const& camViewMtx, Matrix4 const& camProjMtx) const noexcept -> void;
  auto PostProcess(ID3D11ShaderResourceView* src, ID3D11RenderTargetView* dst) noexcept -> void;

  auto ClearGizmoDrawQueue() noexcept -> void;
  auto ReleaseTempRenderTargets() noexcept -> void;

  static auto OnWindowSize(Impl* self, Extent2D<u32> size) -> void;

public:
  auto StartUp() -> void;
  auto ShutDown() -> void;

  auto DrawCamera(Camera const& cam, RenderTarget const* rt) -> void;
  auto DrawAllCameras(RenderTarget const* rt) -> void;
  auto DrawGizmos(RenderTarget const* rt) -> void;
  auto BindAndClearSwapChain() noexcept -> void;
  auto Present() noexcept -> void;

  auto GetSyncInterval() noexcept -> u32;
  auto SetSyncInterval(u32 interval) noexcept -> void;

  auto RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;
  auto UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;

  auto GetDevice() noexcept -> ID3D11Device*;
  auto GetImmediateContext() noexcept -> ID3D11DeviceContext*;

  auto RegisterLight(LightComponent const* light) -> void;
  auto UnregisterLight(LightComponent const* light) -> void;

  auto GetDefaultMaterial() noexcept -> Material*;
  auto GetCubeMesh() noexcept -> Mesh*;
  auto GetPlaneMesh() noexcept -> Mesh*;

  auto GetGamma() noexcept -> f32;
  auto SetGamma(f32 gamma) noexcept -> void;

  auto RegisterSkybox(SkyboxComponent const* skybox) -> void;
  auto UnregisterSkybox(SkyboxComponent const* skybox) -> void;

  auto RegisterGameCamera(Camera const& cam) -> void;
  auto UnregisterGameCamera(Camera const& cam) -> void;

  auto CullLights(Frustum const& frustumWS, Visibility& visibility) -> void;
  auto CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) -> void;

  auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;

  auto GetShadowCascadeCount() noexcept -> int;
  auto SetShadowCascadeCount(int cascadeCount) noexcept -> void;

  auto GetMaxShadowCascadeCount() noexcept -> int;

  auto GetNormalizedShadowCascadeSplits() noexcept -> std::span<float const>;
  auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;

  auto GetShadowDistance() noexcept -> float;
  auto SetShadowDistance(float shadowDistance) noexcept -> void;

  auto IsVisualizingShadowCascades() noexcept -> bool;
  auto VisualizeShadowCascades(bool visualize) noexcept -> void;

  auto IsUsingStableShadowCascadeProjection() noexcept -> bool;
  auto UseStableShadowCascadeProjection(bool useStableProj) noexcept -> void;

  auto GetShadowFilteringMode() noexcept -> ShadowFilteringMode;
  auto SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void;

  auto GetInFlightFrameCount() noexcept -> int;
  auto SetInFlightFrameCount(int count) -> void;

  auto GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget&;
};


auto Renderer::Impl::GetSceneDrawDssForReversedDepth() -> ComPtr<ID3D11DepthStencilState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mDepthTestGreaterEqualNoWriteDss;
  } else {
    return mDepthTestLessEqualNoWriteDss;
  }
}


auto Renderer::Impl::GetShadowDrawDssForReversedDepth() -> ComPtr<ID3D11DepthStencilState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mDepthTestGreaterWriteDss;
  } else {
    return mDepthTestLessWriteDss;
  }
}


auto Renderer::Impl::GetSkyboxDrawDssForReversedDepth() -> ComPtr<ID3D11DepthStencilState>& {
  return GetShadowDrawDssForReversedDepth();
}


auto Renderer::Impl::GetShadowPointSamplerForReversedDepth() -> ComPtr<ID3D11SamplerState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mCmpPointGreaterEqualSs;
  } else {
    return mCmpPointLessEqualSs;
  }
}


auto Renderer::Impl::GetShadowPcfSamplerForReversedDepth() -> ComPtr<ID3D11SamplerState>& {
  if constexpr (Graphics::IsDepthBufferReversed()) {
    return mCmpPcfGreaterEqualSs;
  } else {
    return mCmpPcfLessEqualSs;
  }
}


auto Renderer::Impl::CalculateCameraShadowCascadeBoundaries(Camera const& cam) const -> ShadowCascadeBoundaries {
  auto const camNear{ cam.GetNearClipPlane() };
  auto const shadowDistance{ std::min(cam.GetFarClipPlane(), mShadowDistance) };
  auto const shadowedFrustumDepth{ shadowDistance - camNear };

  ShadowCascadeBoundaries boundaries;

  boundaries[0].nearClip = camNear;

  for (int i = 0; i < mCascadeCount - 1; i++) {
    boundaries[i + 1].nearClip = camNear + mCascadeSplits[i] * shadowedFrustumDepth;
    boundaries[i].farClip = boundaries[i + 1].nearClip * 1.005f;
  }

  boundaries[mCascadeCount - 1].farClip = shadowDistance;

  for (int i = mCascadeCount; i < MAX_CASCADE_COUNT; i++) {
    boundaries[i].nearClip = std::numeric_limits<float>::infinity();
    boundaries[i].farClip = std::numeric_limits<float>::infinity();
  }

  return boundaries;
}


auto Renderer::Impl::CreateDeviceAndContext() -> void {
  UINT creationFlags{ 0 };
  D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{ D3D_FEATURE_LEVEL_11_0 };

#ifndef NDEBUG
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, mDevice.GetAddressOf(), nullptr, mImmediateContext.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create D3D device." };
  }
}


auto Renderer::Impl::SetDebugBreaks() const -> void {
  ComPtr<ID3D11Debug> d3dDebug;
  if (FAILED(mDevice.As(&d3dDebug))) {
    throw std::runtime_error{ "Failed to get ID3D11Debug interface." };
  }

  ComPtr<ID3D11InfoQueue> d3dInfoQueue;
  if (FAILED(d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue))) {
    throw std::runtime_error{ "Failed to get ID3D11InfoQueue interface." };
  }

  d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
  d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
}


auto Renderer::Impl::CreateInputLayouts() -> void {
  D3D11_INPUT_ELEMENT_DESC constexpr inputDescs[]{
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
    },
    {
      .SemanticName = "TANGENT",
      .SemanticIndex = 0,
      .Format = DXGI_FORMAT_R32G32B32_FLOAT,
      .InputSlot = 3,
      .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
      .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
      .InstanceDataStepRate = 0
    }
  };

  if (FAILED(mDevice->CreateInputLayout(inputDescs, ARRAYSIZE(inputDescs), gMeshVSBin, ARRAYSIZE(gMeshVSBin), mAllAttribsIl.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create all-attributes input layout." };
  }

  if (FAILED(mDevice->CreateInputLayout(&inputDescs[0], 1, gDepthOnlyVSBin, ARRAYSIZE(gDepthOnlyVSBin), mPos3OnlyIl.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create pos3d-only input layout." };
  }
}


auto Renderer::Impl::CreateShaders() -> void {
  if (FAILED(mDevice->CreateVertexShader(gMeshVSBin, ARRAYSIZE(gMeshVSBin), nullptr, mMeshVs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create mesh vertex shader." };
  }

  if (FAILED(mDevice->CreatePixelShader(gMeshPbrPSBin, ARRAYSIZE(gMeshPbrPSBin), nullptr, mMeshPbrPs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create mesh pbr pixel shader." };
  }

  if (FAILED(mDevice->CreatePixelShader(gPostProcessPSBin, ARRAYSIZE(gPostProcessPSBin), nullptr, mPostProcessPs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create textured post process pixel shader." };
  }

  if (FAILED(mDevice->CreateVertexShader(gSkyboxVSBin, ARRAYSIZE(gSkyboxVSBin), nullptr, mSkyboxVs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create skybox vertex shader." };
  }

  if (FAILED(mDevice->CreatePixelShader(gSkyboxPSBin, ARRAYSIZE(gSkyboxPSBin), nullptr, mSkyboxPs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create skybox pixel shader." };
  }

  if (FAILED(mDevice->CreateVertexShader(gDepthOnlyVSBin, ARRAYSIZE(gDepthOnlyVSBin), nullptr, mDepthOnlyVs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create depth-only vertex shader." };
  }

  if (FAILED(mDevice->CreateVertexShader(gScreenVSBin, ARRAYSIZE(gScreenVSBin), nullptr, mScreenVs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create screen vertex shader." };
  }

  if (FAILED(mDevice->CreateVertexShader(gLineGizmoVSBin, ARRAYSIZE(gLineGizmoVSBin), nullptr, mLineGizmoVs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create line gizmo vertex shader." };
  }

  if (FAILED(mDevice->CreatePixelShader(gGizmoPSBin, ARRAYSIZE(gGizmoPSBin), nullptr, mGizmoPs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create gizmo pixel shader." };
  }
}


auto Renderer::Impl::CreateConstantBuffers() -> void {
  D3D11_BUFFER_DESC constexpr perFrameCbDesc{
    .ByteWidth = sizeof(PerFrameCB),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&perFrameCbDesc, nullptr, mPerFrameCb.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create per frame CB." };
  }

  D3D11_BUFFER_DESC constexpr perCamCbDesc{
    .ByteWidth = sizeof(PerCameraCB),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&perCamCbDesc, nullptr, mPerCamCb.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create per camera CB." };
  }

  D3D11_BUFFER_DESC constexpr perDrawCbDesc{
    .ByteWidth = sizeof(PerDrawCB),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&perDrawCbDesc, nullptr, mPerDrawCb.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create per draw CB." };
  }

  D3D11_BUFFER_DESC constexpr postProcessCbDesc{
    .ByteWidth = sizeof(PostProcessCB),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&postProcessCbDesc, nullptr, mPostProcessCb.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create post-process CB." };
  }

  D3D11_BUFFER_DESC constexpr skyboxCbDesc{
    .ByteWidth = sizeof(SkyboxCB),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&skyboxCbDesc, nullptr, mSkyboxCb.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create skybox pass CB." };
  }

  D3D11_BUFFER_DESC constexpr depthOnlyCbDesc{
    .ByteWidth = sizeof(DepthOnlyCB),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&depthOnlyCbDesc, nullptr, mDepthOnlyCb.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create depth-only pass CB." };
  }
}


auto Renderer::Impl::CreateRasterizerStates() -> void {
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

  if (FAILED(mDevice->CreateRasterizerState(&skyboxPassRasterizerDesc, mSkyboxPassRs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create skybox pass rasterizer state." };
  }

  D3D11_RASTERIZER_DESC constexpr shadowPassRasterizerDesc{
    .FillMode = D3D11_FILL_SOLID,
    .CullMode = D3D11_CULL_FRONT,
    .FrontCounterClockwise = FALSE,
    .DepthBias = 0,
    .DepthBiasClamp = 0,
    .SlopeScaledDepthBias = 0,
    .DepthClipEnable = TRUE,
    .ScissorEnable = FALSE,
    .MultisampleEnable = FALSE,
    .AntialiasedLineEnable = FALSE
  };

  if (FAILED(mDevice->CreateRasterizerState(&shadowPassRasterizerDesc, mShadowPassRs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create shadow pass rasterizer state." };
  }
}


auto Renderer::Impl::CreateDepthStencilStates() -> void {
  D3D11_DEPTH_STENCIL_DESC constexpr depthTestGreaterWrite{
    .DepthEnable = TRUE,
    .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
    .DepthFunc = D3D11_COMPARISON_GREATER,
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

  if (FAILED(mDevice->CreateDepthStencilState(&depthTestGreaterWrite, mDepthTestGreaterWriteDss.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create depth-test greater write depth-stencil state." };
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestLessrWrite{
    .DepthEnable = TRUE,
    .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
    .DepthFunc = D3D11_COMPARISON_LESS,
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

  if (FAILED(mDevice->CreateDepthStencilState(&depthTestLessrWrite, mDepthTestLessWriteDss.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create depth-test less write depth-stencil state." };
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestLessEqualNoWriteDesc{
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

  if (FAILED(mDevice->CreateDepthStencilState(&depthTestLessEqualNoWriteDesc, mDepthTestLessEqualNoWriteDss.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create depth-test less-or-equal no-write DSS." };
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestGreaterEqualNoWriteDesc{
    .DepthEnable = TRUE,
    .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO,
    .DepthFunc = D3D11_COMPARISON_GREATER_EQUAL,
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

  if (FAILED(mDevice->CreateDepthStencilState(&depthTestGreaterEqualNoWriteDesc, mDepthTestGreaterEqualNoWriteDss.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create depth-test greater-or-equal no-write DSS." };
  }
}


auto Renderer::Impl::CreateShadowAtlases() -> void {
  mDirectionalShadowAtlas = std::make_unique<DirectionalShadowAtlas>(mDevice.Get(), 4096);
  mPunctualShadowAtlas = std::make_unique<PunctualShadowAtlas>(mDevice.Get(), 4096);
}


auto Renderer::Impl::CreateSamplerStates() -> void {
  D3D11_SAMPLER_DESC constexpr cmpPcfGreaterEqual{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL,
    .BorderColor = { 0, 0, 0, 0 },
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPcfGreaterEqual, mCmpPcfGreaterEqualSs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create PCF greater-equal comparison sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr cmpPcfLessEqual{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,
    .BorderColor = { 0, 0, 0, 0 },
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPcfLessEqual, mCmpPcfLessEqualSs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create PCF less-equal comparison sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr cmpPointGreaterEqualDesc{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL,
    .BorderColor = { 0, 0, 0, 0 },
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPointGreaterEqualDesc, mCmpPointGreaterEqualSs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create point-filter greater-equal comparison sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr cmpPointLessEqualDesc{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,
    .BorderColor = { 0, 0, 0, 0 },
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPointLessEqualDesc, mCmpPointLessEqualSs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create point-filter less-equal comparison sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr af16Desc{
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

  if (FAILED(mDevice->CreateSamplerState(&af16Desc, mAf16Ss.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create AF16 sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr af8Desc{
    .Filter = D3D11_FILTER_ANISOTROPIC,
    .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
    .MipLODBias = 0,
    .MaxAnisotropy = 8,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af8Desc, mAf8Ss.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create AF8 sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr af4Desc{
    .Filter = D3D11_FILTER_ANISOTROPIC,
    .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
    .MipLODBias = 0,
    .MaxAnisotropy = 4,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af4Desc, mAf4Ss.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create AF4 sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr trilinearDesc{
    .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
    .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&trilinearDesc, mTrilinearSs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create trilinear sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr bilinearDesc{
    .Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&bilinearDesc, mBilinearSs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create bilinear sampler state." };
  }

  D3D11_SAMPLER_DESC constexpr pointDesc{
    .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f },
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&pointDesc, mPointSs.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create point-filter sampler state." };
  }
}


auto Renderer::Impl::CreateDefaultAssets() -> void {
  mDefaultMaterial = std::make_unique<Material>();
  mDefaultMaterial->SetName("Default Material");

  std::vector<Vector3> cubeNormals;
  CalculateNormals(CUBE_POSITIONS, CUBE_INDICES, cubeNormals);

  std::vector<Vector3> cubeTangents;
  CalculateTangents(CUBE_POSITIONS, CUBE_UVS, CUBE_INDICES, cubeTangents);

  std::vector<Vector3> quadNormals;
  CalculateNormals(QUAD_POSITIONS, QUAD_INDICES, quadNormals);

  std::vector<Vector3> quadTangents;
  CalculateTangents(QUAD_POSITIONS, QUAD_UVS, QUAD_INDICES, quadTangents);

  mCubeMesh = std::make_unique<Mesh>();
  mCubeMesh->SetGuid(Guid{ 0, 1 });
  mCubeMesh->SetName("Cube");
  mCubeMesh->SetPositions(CUBE_POSITIONS);
  mCubeMesh->SetNormals(std::move(cubeNormals));
  mCubeMesh->SetUVs(CUBE_UVS);
  mCubeMesh->SetTangents(std::move(cubeTangents));
  mCubeMesh->SetIndices(CUBE_INDICES);
  mCubeMesh->SetSubMeshes(std::vector{ Mesh::SubMeshData{ 0, 0, static_cast<int>(CUBE_INDICES.size()), "Material" } });
  mCubeMesh->ValidateAndUpdate();
  mCubeMesh->ReleaseCPUMemory();

  mPlaneMesh = std::make_unique<Mesh>();
  mPlaneMesh->SetGuid(Guid{ 0, 2 });
  mPlaneMesh->SetName("Plane");
  mPlaneMesh->SetPositions(QUAD_POSITIONS);
  mPlaneMesh->SetNormals(std::move(quadNormals));
  mPlaneMesh->SetUVs(QUAD_UVS);
  mPlaneMesh->SetTangents(std::move(quadTangents));
  mPlaneMesh->SetIndices(QUAD_INDICES);
  mPlaneMesh->SetSubMeshes(std::vector{ Mesh::SubMeshData{ 0, 0, static_cast<int>(QUAD_INDICES.size()), "Material" } });
  mPlaneMesh->ValidateAndUpdate();
  mPlaneMesh->ReleaseCPUMemory();
}


auto Renderer::Impl::DrawMeshes(std::span<int const> const meshComponentIndices, bool const useMaterials) noexcept -> void {
  mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_DRAW, 1, mPerDrawCb.GetAddressOf());
  mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_DRAW, 1, mPerDrawCb.GetAddressOf());

  for (auto const meshComponentIdx : meshComponentIndices) {
    auto const meshComponent{ mStaticMeshComponents[meshComponentIdx] };
    auto const& mesh{ meshComponent->GetMesh() };

    std::array const vertexBuffers{ mesh.GetPositionBuffer().Get(), mesh.GetNormalBuffer().Get(), mesh.GetUVBuffer().Get(), mesh.GetTangentBuffer().Get() };
    UINT constexpr strides[]{ sizeof(Vector3), sizeof(Vector3), sizeof(Vector2), sizeof(Vector3) };
    UINT constexpr offsets[]{ 0, 0, 0, 0 };
    mImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), strides, offsets);
    mImmediateContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

    D3D11_MAPPED_SUBRESOURCE mappedPerDrawCb;
    mImmediateContext->Map(mPerDrawCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerDrawCb);
    auto const perDrawCbData{ static_cast<PerDrawCB*>(mappedPerDrawCb.pData) };
    perDrawCbData->gPerDrawConstants.modelMtx = meshComponent->GetEntity().GetTransform().GetModelMatrix();
    perDrawCbData->gPerDrawConstants.normalMtx = Matrix4{ meshComponent->GetEntity().GetTransform().GetNormalMatrix() };
    mImmediateContext->Unmap(mPerDrawCb.Get(), 0);

    auto const subMeshes{ mesh.GetSubMeshes() };
    auto const& materials{ meshComponent->GetMaterials() };

    for (int i = 0; i < static_cast<int>(subMeshes.size()); i++) {
      auto const& [baseVertex, firstIndex, indexCount, mtlSlotName]{ subMeshes[i] };

      if (useMaterials) {
        auto const& mtl{
          static_cast<int>(materials.size()) > i
            ? *materials[i]
            : *mDefaultMaterial
        };

        auto const mtlBuffer{ mtl.GetBuffer() };
        mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);
        mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);

        auto const albedoSrv{
          [&mtl] {
            auto const albedoMap{ mtl.GetAlbedoMap().lock() };
            return albedoMap
                     ? albedoMap->GetSrv()
                     : nullptr;
          }()
        };
        mImmediateContext->PSSetShaderResources(RES_SLOT_ALBEDO_MAP, 1, &albedoSrv);

        auto const metallicSrv{
          [&mtl] {
            auto const metallicMap{ mtl.GetMetallicMap().lock() };
            return metallicMap
                     ? metallicMap->GetSrv()
                     : nullptr;
          }()
        };
        mImmediateContext->PSSetShaderResources(RES_SLOT_METALLIC_MAP, 1, &metallicSrv);

        auto const roughnessSrv{
          [&mtl] {
            auto const roughnessMap{ mtl.GetRoughnessMap().lock() };
            return roughnessMap
                     ? roughnessMap->GetSrv()
                     : nullptr;
          }()
        };
        mImmediateContext->PSSetShaderResources(RES_SLOT_ROUGHNESS_MAP, 1, &roughnessSrv);

        auto const aoSrv{
          [&mtl] {
            auto const aoMap{ mtl.GetAoMap().lock() };
            return aoMap
                     ? aoMap->GetSrv()
                     : nullptr;
          }()
        };
        mImmediateContext->PSSetShaderResources(RES_SLOT_AO_MAP, 1, &aoSrv);

        auto const normalSrv{
          [&mtl] {
            auto const normalMap{ mtl.GetNormalMap().lock() };
            return normalMap
                     ? normalMap->GetSrv()
                     : nullptr;
          }()
        };
        mImmediateContext->PSSetShaderResources(RES_SLOT_NORMAL_MAP, 1, &normalSrv);
      }

      mImmediateContext->DrawIndexed(indexCount, firstIndex, baseVertex);
    }
  }
}


auto Renderer::Impl::PostProcess(ID3D11ShaderResourceView* const src, ID3D11RenderTargetView* const dst) noexcept -> void {
  // Back up old views to restore later.

  ComPtr<ID3D11RenderTargetView> rtvBackup;
  ComPtr<ID3D11DepthStencilView> dsvBackup;
  ComPtr<ID3D11ShaderResourceView> srvBackup;
  mImmediateContext->OMGetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.GetAddressOf());
  mImmediateContext->PSGetShaderResources(0, 1, srvBackup.GetAddressOf());

  // Do the step

  D3D11_MAPPED_SUBRESOURCE mappedCb;
  mImmediateContext->Map(mPostProcessCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCb);
  auto const cbData{ static_cast<PostProcessCB*>(mappedCb.pData) };
  cbData->invGamma = mInvGamma;
  mImmediateContext->Unmap(mPostProcessCb.Get(), 0);

  mImmediateContext->VSSetShader(mScreenVs.Get(), nullptr, 0);
  mImmediateContext->PSSetShader(mPostProcessPs.Get(), nullptr, 0);

  mImmediateContext->OMSetRenderTargets(1, &dst, nullptr);

  mImmediateContext->PSSetConstantBuffers(CB_SLOT_POST_PROCESS, 1, mPostProcessCb.GetAddressOf());
  mImmediateContext->PSSetShaderResources(RES_SLOT_POST_PROCESS_SRC, 1, &src);

  mImmediateContext->IASetInputLayout(nullptr);
  mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  mImmediateContext->Draw(6, 0);

  // Restore old view bindings to that we don't leave any input/output conflicts behind.

  mImmediateContext->PSSetShaderResources(RES_SLOT_POST_PROCESS_SRC, 1, srvBackup.GetAddressOf());
  mImmediateContext->OMSetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.Get());
}


auto Renderer::Impl::DrawSkybox(Matrix4 const& camViewMtx, Matrix4 const& camProjMtx) const noexcept -> void {
  if (mSkyboxes.empty()) {
    return;
  }

  D3D11_MAPPED_SUBRESOURCE mappedSkyboxCB;
  mImmediateContext->Map(mSkyboxCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSkyboxCB);
  auto const skyboxCBData{ static_cast<SkyboxCB*>(mappedSkyboxCB.pData) };
  skyboxCBData->skyboxViewProjMtx = Matrix4{ Matrix3{ camViewMtx } } * camProjMtx;
  mImmediateContext->Unmap(mSkyboxCb.Get(), 0);

  ID3D11Buffer* const vertexBuffer{ mCubeMesh->GetPositionBuffer().Get() };
  UINT constexpr stride{ sizeof(Vector3) };
  UINT constexpr offset{ 0 };
  mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
  mImmediateContext->IASetIndexBuffer(mCubeMesh->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
  mImmediateContext->IASetInputLayout(mPos3OnlyIl.Get());
  mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  mImmediateContext->VSSetShader(mSkyboxVs.Get(), nullptr, 0);
  mImmediateContext->PSSetShader(mSkyboxPs.Get(), nullptr, 0);

  auto const cubemapSrv{ mSkyboxes[0]->GetCubemap()->GetSrv() };
  mImmediateContext->PSSetShaderResources(RES_SLOT_SKYBOX_CUBEMAP, 1, &cubemapSrv);

  auto const cb{ mSkyboxCb.Get() };
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_SKYBOX_PASS, 1, &cb);

  mImmediateContext->OMSetDepthStencilState(mDepthTestLessEqualNoWriteDss.Get(), 0);
  mImmediateContext->RSSetState(mSkyboxPassRs.Get());

  mImmediateContext->DrawIndexed(clamp_cast<UINT>(CUBE_INDICES.size()), 0, 0);

  // Restore state
  mImmediateContext->OMSetDepthStencilState(nullptr, 0);
  mImmediateContext->RSSetState(nullptr);
}


auto Renderer::Impl::DrawShadowMaps(ShadowAtlas const& atlas) -> void {
  mImmediateContext->OMSetRenderTargets(0, nullptr, atlas.GetDsv());
  mImmediateContext->ClearDepthStencilView(atlas.GetDsv(), D3D11_CLEAR_DEPTH, Graphics::GetDepthClearValueForReversedDepth(), 0);
  mImmediateContext->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);
  mImmediateContext->PSSetShader(nullptr, nullptr, 0);
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_DEPTH_ONLY_PASS, 1, mDepthOnlyCb.GetAddressOf());
  mImmediateContext->OMSetDepthStencilState(GetShadowDrawDssForReversedDepth().Get(), 0);
  mImmediateContext->RSSetState(mShadowPassRs.Get());

  auto const cellSizeNorm{ atlas.GetNormalizedElementSize() };

  for (auto i = 0; i < atlas.GetElementCount(); i++) {
    auto const& cell{ atlas.GetCell(i) };
    auto const cellOffsetNorm{ atlas.GetNormalizedElementOffset(i) };
    auto const subcellSize{ cellSizeNorm * cell.GetNormalizedElementSize() * static_cast<float>(atlas.GetSize()) };

    for (auto j = 0; j < cell.GetElementCount(); j++) {
      if (auto const& subcell{ cell.GetSubcell(j) }) {
        auto const subcellOffset{ (cellOffsetNorm + cell.GetNormalizedElementOffset(j) * cellSizeNorm) * static_cast<float>(atlas.GetSize()) };

        D3D11_VIEWPORT const viewport{
          .TopLeftX = subcellOffset[0],
          .TopLeftY = subcellOffset[1],
          .Width = subcellSize,
          .Height = subcellSize,
          .MinDepth = 0,
          .MaxDepth = 1
        };

        mImmediateContext->RSSetViewports(1, &viewport);

        D3D11_MAPPED_SUBRESOURCE mapped;
        mImmediateContext->Map(mDepthOnlyCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        auto const depthOnlyCbData{ static_cast<DepthOnlyCB*>(mapped.pData) };
        depthOnlyCbData->gDepthOnlyViewProjMtx = subcell->shadowViewProjMtx;
        mImmediateContext->Unmap(mDepthOnlyCb.Get(), 0);

        Frustum const shadowFrustumWS{ subcell->shadowViewProjMtx };

        Visibility static perLightVisibility;

        CullStaticMeshComponents(shadowFrustumWS, perLightVisibility);

        mImmediateContext->IASetInputLayout(mPos3OnlyIl.Get());

        DrawMeshes(perLightVisibility.staticMeshIndices, false);
      }
    }
  }
}


auto Renderer::Impl::DrawGizmos(RenderTarget const* const rt) -> void {
  mImmediateContext->OMSetRenderTargets(1, std::array{
                                          rt
                                            ? rt->GetRtv()
                                            : mSwapChain->GetRtv()
                                        }.data(), nullptr);

  mGizmoColorBuffer->Resize(static_cast<int>(std::ssize(mGizmoColors)));
  std::ranges::copy(mGizmoColors, std::begin(mGizmoColorBuffer->Map()));
  mGizmoColorBuffer->Unmap();

  mLineGizmoVertexDataBuffer->Resize(static_cast<int>(std::ssize(mLineGizmoVertexData)));
  std::ranges::copy(mLineGizmoVertexData, std::begin(mLineGizmoVertexDataBuffer->Map()));
  mLineGizmoVertexDataBuffer->Unmap();

  mImmediateContext->PSSetShader(mGizmoPs.Get(), nullptr, 0);
  mImmediateContext->PSSetShaderResources(RES_SLOT_GIZMO_COLOR, 1, std::array{ mGizmoColorBuffer->GetSrv() }.data());

  if (!mLineGizmoVertexData.empty()) {
    mImmediateContext->VSSetShader(mLineGizmoVs.Get(), nullptr, 0);
    mImmediateContext->VSSetShaderResources(RES_SLOT_LINE_GIZMO_VERTEX, 1, std::array{ mLineGizmoVertexDataBuffer->GetSrv() }.data());
    mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    mImmediateContext->DrawInstanced(2, static_cast<UINT>(mLineGizmoVertexData.size()), 0, 0);
  }
}


auto Renderer::Impl::ClearGizmoDrawQueue() noexcept -> void {
  mGizmoColors.clear();
  mLineGizmoVertexData.clear();
}


auto Renderer::Impl::ReleaseTempRenderTargets() noexcept -> void {
  std::erase_if(mTmpRenderTargets, [](TempRenderTargetRecord& tmpRtRecord) {
    tmpRtRecord.ageInFrames += 1;
    return tmpRtRecord.ageInFrames >= MAX_TMP_RT_AGE;
  });
}


auto Renderer::Impl::OnWindowSize(Impl* const self, Extent2D<u32> const size) -> void {
  self->mSwapChain->Resize(size.width, size.height);
}


auto Renderer::Impl::StartUp() -> void {
  CreateDeviceAndContext();

#ifndef NDEBUG
  SetDebugBreaks();
#endif

  if (FAILED(mDevice.As(&mDxgiDevice))) {
    throw std::runtime_error{ "Failed to query IDXGIDevice interface." };
  }

  SetInFlightFrameCount(2);

  ComPtr<IDXGIAdapter> dxgiAdapter;
  if (FAILED(mDxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to get IDXGIAdapter." };
  }

  ComPtr<IDXGIFactory2> dxgiFactory2;
  if (FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory2.GetAddressOf())))) {
    throw std::runtime_error{ "Failed to query IDXGIFactory2 interface." };
  }

  mSwapChain = std::make_unique<SwapChain>(mDevice, dxgiFactory2.Get());

  mLightBuffer = std::make_unique<StructuredBuffer<ShaderLight>>(mDevice, mImmediateContext);
  mGizmoColorBuffer = std::make_unique<StructuredBuffer<Vector4>>(mDevice, mImmediateContext);
  mLineGizmoVertexDataBuffer = std::make_unique<StructuredBuffer<ShaderLineGizmoVertexData>>(mDevice, mImmediateContext);

  CreateInputLayouts();
  CreateShaders();
  CreateConstantBuffers();
  CreateRasterizerStates();
  CreateDepthStencilStates();
  CreateShadowAtlases();
  CreateSamplerStates();
  CreateDefaultAssets();

  gWindow.OnWindowSize.add_handler(this, &OnWindowSize);

  dxgiFactory2->MakeWindowAssociation(gWindow.GetHandle(), DXGI_MWA_NO_WINDOW_CHANGES);
}


auto Renderer::Impl::ShutDown() -> void {
  gWindow.OnWindowSize.remove_handler(this, &OnWindowSize);
}


auto Renderer::Impl::DrawCamera(Camera const& cam, RenderTarget const* const rt) -> void {
  auto const rtWidth{
    rt
      ? rt->GetDesc().width
      : gWindow.GetCurrentClientAreaSize().width
  };
  auto const rtHeight{
    rt
      ? rt->GetDesc().height
      : gWindow.GetCurrentClientAreaSize().height
  };
  auto const rtAspect{ static_cast<float>(rtWidth) / static_cast<float>(rtHeight) };

  RenderTarget::Desc const hdrRtDesc{
    .width = rtWidth,
    .height = rtHeight,
    .colorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT,
    .depthBufferBitCount = 32,
    .stencilBufferBitCount = 0,
    .debugName = "Camera HDR RenderTarget"
  };

  auto const& hdrRt{ GetTemporaryRenderTarget(hdrRtDesc) };

  FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
  mImmediateContext->ClearRenderTargetView(hdrRt.GetRtv(), clearColor);


  D3D11_VIEWPORT const viewport{
    .TopLeftX = 0,
    .TopLeftY = 0,
    .Width = static_cast<FLOAT>(rtWidth),
    .Height = static_cast<FLOAT>(rtHeight),
    .MinDepth = 0,
    .MaxDepth = 1
  };

  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_CMP_PCF, 1, GetShadowPcfSamplerForReversedDepth().GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_CMP_POINT, 1, GetShadowPointSamplerForReversedDepth().GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_AF16, 1, mAf16Ss.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_AF8, 1, mAf8Ss.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_AF4, 1, mAf4Ss.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_TRI, 1, mTrilinearSs.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_BI, 1, mBilinearSs.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_POINT, 1, mPointSs.GetAddressOf());

  D3D11_MAPPED_SUBRESOURCE mappedPerFrameCB;
  if (FAILED(mImmediateContext->Map(mPerFrameCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerFrameCB))) {
    throw std::runtime_error{ "Failed to map per frame CB." };
  }

  auto const perFrameCbData{ static_cast<PerFrameCB*>(mappedPerFrameCB.pData) };
  perFrameCbData->gPerFrameConstants.shadowCascadeCount = mCascadeCount;
  perFrameCbData->gPerFrameConstants.visualizeShadowCascades = mVisualizeShadowCascades;
  perFrameCbData->gPerFrameConstants.shadowFilteringMode = static_cast<int>(mShadowFilteringMode);

  mImmediateContext->Unmap(mPerFrameCb.Get(), 0);

  mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, mPerFrameCb.GetAddressOf());
  mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, mPerFrameCb.GetAddressOf());

  auto const camPos{ cam.GetPosition() };
  auto const camViewMtx{ cam.CalculateViewMatrix() };
  auto const camProjMtx{ cam.CalculateProjectionMatrix(rtAspect) };
  auto const camViewProjMtx{ camViewMtx * camProjMtx };
  Frustum const camFrustWS{ camViewProjMtx };

  Visibility static visibility;
  CullLights(camFrustWS, visibility);

  // Shadow pass

  auto const shadowCascadeBoundaries{ CalculateCameraShadowCascadeBoundaries(cam) };

  ID3D11ShaderResourceView* const nullSrv{ nullptr };
  mImmediateContext->PSSetShaderResources(RES_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, &nullSrv);
  mImmediateContext->PSSetShaderResources(RES_SLOT_DIR_SHADOW_ATLAS, 1, &nullSrv);
  mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  mDirectionalShadowAtlas->Update(mLights, visibility, cam, shadowCascadeBoundaries, rtAspect, mCascadeCount, mUseStableShadowCascadeProjection);
  DrawShadowMaps(*mDirectionalShadowAtlas);

  mPunctualShadowAtlas->Update(mLights, visibility, cam, camViewProjMtx, mShadowDistance);
  DrawShadowMaps(*mPunctualShadowAtlas);

  CullStaticMeshComponents(camFrustWS, visibility);

  // Depth pre-pass

  mImmediateContext->OMSetRenderTargets(0, nullptr, hdrRt.GetDsv());
  mImmediateContext->OMSetDepthStencilState(mDepthTestLessWriteDss.Get(), 0);

  mImmediateContext->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_DEPTH_ONLY_PASS, 1, mDepthOnlyCb.GetAddressOf());

  mImmediateContext->RSSetViewports(1, &viewport);
  mImmediateContext->RSSetState(nullptr);

  mImmediateContext->IASetInputLayout(mPos3OnlyIl.Get());

  mImmediateContext->ClearDepthStencilView(hdrRt.GetDsv(), D3D11_CLEAR_DEPTH, 1.0f, 0);

  D3D11_MAPPED_SUBRESOURCE mappedCb;
  if (FAILED(mImmediateContext->Map(mDepthOnlyCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCb))) {
    throw std::runtime_error{ "Failed to map CB for depth pre-pass." };
  }
  static_cast<DepthOnlyCB*>(mappedCb.pData)->gDepthOnlyViewProjMtx = camViewProjMtx;
  mImmediateContext->Unmap(mDepthOnlyCb.Get(), 0);

  DrawMeshes(visibility.staticMeshIndices, false);

  // Full forward lighting pass
  mImmediateContext->VSSetShader(mMeshVs.Get(), nullptr, 0);

  D3D11_MAPPED_SUBRESOURCE mappedPerCamCB;
  if (FAILED(mImmediateContext->Map(mPerCamCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerCamCB))) {
    throw std::runtime_error{ "Failed to map per camera CB." };
  }

  auto const perCamCbData{ static_cast<PerCameraCB*>(mappedPerCamCB.pData) };
  perCamCbData->gPerCamConstants.viewProjMtx = camViewProjMtx;
  perCamCbData->gPerCamConstants.camPos = camPos;

  for (int i = 0; i < MAX_CASCADE_COUNT; i++) {
    perCamCbData->gPerCamConstants.shadowCascadeSplitDistances[i] = shadowCascadeBoundaries[i].farClip;
  }

  mImmediateContext->Unmap(mPerCamCb.Get(), 0);

  auto const lightCount{ std::ssize(visibility.lightIndices) };
  mLightBuffer->Resize(static_cast<int>(lightCount));
  auto const lightBufferData{ mLightBuffer->Map() };

  for (int i = 0; i < lightCount; i++) {
    lightBufferData[i].color = mLights[visibility.lightIndices[i]]->GetColor();
    lightBufferData[i].intensity = mLights[visibility.lightIndices[i]]->GetIntensity();
    lightBufferData[i].type = static_cast<int>(mLights[visibility.lightIndices[i]]->GetType());
    lightBufferData[i].direction = mLights[visibility.lightIndices[i]]->GetDirection();
    lightBufferData[i].isCastingShadow = FALSE;
    lightBufferData[i].range = mLights[visibility.lightIndices[i]]->GetRange();
    lightBufferData[i].halfInnerAngleCos = std::cos(ToRadians(mLights[visibility.lightIndices[i]]->GetInnerAngle() / 2.0f));
    lightBufferData[i].halfOuterAngleCos = std::cos(ToRadians(mLights[visibility.lightIndices[i]]->GetOuterAngle() / 2.0f));
    lightBufferData[i].position = mLights[visibility.lightIndices[i]]->GetEntity().GetTransform().GetWorldPosition();
    lightBufferData[i].depthBias = mLights[visibility.lightIndices[i]]->GetShadowDepthBias();
    lightBufferData[i].normalBias = mLights[visibility.lightIndices[i]]->GetShadowNormalBias();

    for (auto& sample : lightBufferData[i].sampleShadowMap) {
      sample = FALSE;
    }
  }

  mDirectionalShadowAtlas->SetLookUpInfo(lightBufferData);
  mPunctualShadowAtlas->SetLookUpInfo(lightBufferData);

  mLightBuffer->Unmap();

  mImmediateContext->VSSetShader(mMeshVs.Get(), nullptr, 0);
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_CAM, 1, mPerCamCb.GetAddressOf());

  mImmediateContext->OMSetRenderTargets(1, std::array{ hdrRt.GetRtv() }.data(), hdrRt.GetDsv());
  mImmediateContext->OMSetDepthStencilState(mDepthTestLessEqualNoWriteDss.Get(), 0);

  mImmediateContext->PSSetShader(mMeshPbrPs.Get(), nullptr, 0);
  mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_CAM, 1, mPerCamCb.GetAddressOf());
  mImmediateContext->PSSetShaderResources(RES_SLOT_LIGHTS, 1, std::array{ mLightBuffer->GetSrv() }.data());
  mImmediateContext->PSSetShaderResources(RES_SLOT_DIR_SHADOW_ATLAS, 1, std::array{ mDirectionalShadowAtlas->GetSrv() }.data());
  mImmediateContext->PSSetShaderResources(RES_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, std::array{ mPunctualShadowAtlas->GetSrv() }.data());

  mImmediateContext->RSSetViewports(1, &viewport);
  mImmediateContext->RSSetState(nullptr);

  mImmediateContext->IASetInputLayout(mAllAttribsIl.Get());

  DrawMeshes(visibility.staticMeshIndices, true);
  DrawSkybox(camViewMtx, camProjMtx);

  mImmediateContext->RSSetViewports(1, &viewport);
  PostProcess(hdrRt.GetColorSrv(), rt
                                     ? rt->GetRtv()
                                     : mSwapChain->GetRtv());
}


auto Renderer::Impl::DrawAllCameras(RenderTarget const* const rt) -> void {
  for (auto const& cam : mGameRenderCameras) {
    DrawCamera(*cam, rt);
  }
}


auto Renderer::Impl::BindAndClearSwapChain() noexcept -> void {
  FLOAT constexpr clearColor[]{ 0, 0, 0, 1 };
  auto const rtv{ mSwapChain->GetRtv() };
  mImmediateContext->ClearRenderTargetView(rtv, clearColor);
  mImmediateContext->OMSetRenderTargets(1, &rtv, nullptr);
}


auto Renderer::Impl::Present() noexcept -> void {
  mSwapChain->Present(mSyncInterval);

  ClearGizmoDrawQueue();
  ReleaseTempRenderTargets();
}


auto Renderer::Impl::GetSyncInterval() noexcept -> u32 {
  return mSyncInterval;
}


auto Renderer::Impl::SetSyncInterval(u32 const interval) noexcept -> void {
  mSyncInterval = interval;
}


auto Renderer::Impl::RegisterStaticMesh(StaticMeshComponent const* const staticMesh) -> void {
  mStaticMeshComponents.emplace_back(staticMesh);
}


auto Renderer::Impl::UnregisterStaticMesh(StaticMeshComponent const* const staticMesh) -> void {
  std::erase(mStaticMeshComponents, staticMesh);
}


auto Renderer::Impl::GetDevice() noexcept -> ID3D11Device* {
  return mDevice.Get();
}


auto Renderer::Impl::GetImmediateContext() noexcept -> ID3D11DeviceContext* {
  return mImmediateContext.Get();
}


auto Renderer::Impl::RegisterLight(LightComponent const* light) -> void {
  mLights.emplace_back(light);
}


auto Renderer::Impl::UnregisterLight(LightComponent const* light) -> void {
  std::erase(mLights, light);
}


auto Renderer::Impl::GetDefaultMaterial() noexcept -> Material* {
  return mDefaultMaterial.get();
}


auto Renderer::Impl::GetCubeMesh() noexcept -> Mesh* {
  return mCubeMesh.get();
}


auto Renderer::Impl::GetPlaneMesh() noexcept -> Mesh* {
  return mPlaneMesh.get();
}


auto Renderer::Impl::GetGamma() noexcept -> f32 {
  return 1.f / mInvGamma;
}


auto Renderer::Impl::SetGamma(f32 const gamma) noexcept -> void {
  mInvGamma = 1.f / gamma;
}


auto Renderer::Impl::RegisterSkybox(SkyboxComponent const* const skybox) -> void {
  mSkyboxes.emplace_back(skybox);
}


auto Renderer::Impl::UnregisterSkybox(SkyboxComponent const* const skybox) -> void {
  std::erase(mSkyboxes, skybox);
}


auto Renderer::Impl::RegisterGameCamera(Camera const& cam) -> void {
  mGameRenderCameras.emplace_back(&cam);
}


auto Renderer::Impl::UnregisterGameCamera(Camera const& cam) -> void {
  std::erase(mGameRenderCameras, &cam);
}


auto Renderer::Impl::CullLights(Frustum const& frustumWS, Visibility& visibility) -> void {
  visibility.lightIndices.clear();

  for (int lightIdx = 0; lightIdx < static_cast<int>(mLights.size()); lightIdx++) {
    switch (auto const light{ mLights[lightIdx] }; light->GetType()) {
      case LightComponent::Type::Directional: {
        visibility.lightIndices.emplace_back(lightIdx);
        break;
      }

      case LightComponent::Type::Spot: {
        auto const lightVerticesWS{
          [light] {
            auto vertices{ CalculateSpotLightLocalVertices(*light) };

            for (auto const modelMtxNoScale{ CalculateModelMatrixNoScale(light->GetEntity().GetTransform()) };
                 auto& vertex : vertices) {
              vertex = Vector3{ Vector4{ vertex, 1 } * modelMtxNoScale };
            }

            return vertices;
          }()
        };

        if (frustumWS.Intersects(AABB::FromVertices(lightVerticesWS))) {
          visibility.lightIndices.emplace_back(lightIdx);
        }

        break;
      }

      case LightComponent::Type::Point: {
        BoundingSphere const boundsWS{
          .center = Vector3{ light->GetEntity().GetTransform().GetWorldPosition() },
          .radius = light->GetRange()
        };

        if (frustumWS.Intersects(boundsWS)) {
          visibility.lightIndices.emplace_back(lightIdx);
        }
        break;
      }
    }
  }
}


auto Renderer::Impl::CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) -> void {
  visibility.staticMeshIndices.clear();

  for (int i = 0; i < static_cast<int>(mStaticMeshComponents.size()); i++) {
    if (frustumWS.Intersects(mStaticMeshComponents[i]->CalculateBounds())) {
      visibility.staticMeshIndices.emplace_back(i);
    }
  }
}


auto Renderer::Impl::DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void {
  mGizmoColors.emplace_back(color);
  mLineGizmoVertexData.emplace_back(from, static_cast<std::uint32_t>(mGizmoColors.size() - 1), to, 0.0f);
}


auto Renderer::Impl::GetShadowCascadeCount() noexcept -> int {
  return mCascadeCount;
}


auto Renderer::Impl::SetShadowCascadeCount(int const cascadeCount) noexcept -> void {
  mCascadeCount = std::clamp(cascadeCount, 1, MAX_CASCADE_COUNT);
  int const splitCount{ mCascadeCount - 1 };

  for (int i = 1; i < splitCount; i++) {
    mCascadeSplits[i] = std::max(mCascadeSplits[i - 1], mCascadeSplits[i]);
  }
}


auto Renderer::Impl::GetMaxShadowCascadeCount() noexcept -> int {
  return MAX_CASCADE_COUNT;
}


auto Renderer::Impl::GetNormalizedShadowCascadeSplits() noexcept -> std::span<float const> {
  return { std::begin(mCascadeSplits), static_cast<std::size_t>(mCascadeCount - 1) };
}


auto Renderer::Impl::SetNormalizedShadowCascadeSplit(int const idx, float const split) noexcept -> void {
  auto const splitCount{ mCascadeCount - 1 };

  if (idx < 0 || idx >= splitCount) {
    return;
  }

  float const clampMin{
    idx == 0
      ? 0.0f
      : mCascadeSplits[idx - 1]
  };
  float const clampMax{
    idx == splitCount - 1
      ? 1.0f
      : mCascadeSplits[idx + 1]
  };

  mCascadeSplits[idx] = std::clamp(split, clampMin, clampMax);
}


auto Renderer::Impl::GetShadowDistance() noexcept -> float {
  return mShadowDistance;
}


auto Renderer::Impl::SetShadowDistance(float const shadowDistance) noexcept -> void {
  mShadowDistance = std::max(shadowDistance, 0.0f);
}


auto Renderer::Impl::IsVisualizingShadowCascades() noexcept -> bool {
  return mVisualizeShadowCascades;
}


auto Renderer::Impl::VisualizeShadowCascades(bool const visualize) noexcept -> void {
  mVisualizeShadowCascades = visualize;
}


auto Renderer::Impl::IsUsingStableShadowCascadeProjection() noexcept -> bool {
  return mUseStableShadowCascadeProjection;
}


auto Renderer::Impl::UseStableShadowCascadeProjection(bool const useStableProj) noexcept -> void {
  mUseStableShadowCascadeProjection = useStableProj;
}


auto Renderer::Impl::GetShadowFilteringMode() noexcept -> ShadowFilteringMode {
  return mShadowFilteringMode;
}


auto Renderer::Impl::SetShadowFilteringMode(ShadowFilteringMode const filteringMode) noexcept -> void {
  mShadowFilteringMode = filteringMode;
}


auto Renderer::Impl::GetInFlightFrameCount() noexcept -> int {
  return mInFlightFrameCount;
}


auto Renderer::Impl::SetInFlightFrameCount(int const count) -> void {
  mInFlightFrameCount = std::clamp(count, MIN_IN_FLIGHT_FRAME_COUNT, MAX_IN_FLIGHT_FRAME_COUNT);

  if (FAILED(mDxgiDevice->SetMaximumFrameLatency(mInFlightFrameCount))) {
    throw std::runtime_error{ "Failed to set in-flight frame count." };
  }
}


auto Renderer::Impl::GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget& {
  for (auto& [rt, lastUseInFrames] : mTmpRenderTargets) {
    if (rt->GetDesc() == desc) {
      lastUseInFrames = 0;
      return *rt;
    }
  }

  return *mTmpRenderTargets.emplace_back(std::make_unique<RenderTarget>(desc), 0).rt;
}


auto Renderer::StartUp() -> void {
  mImpl = new Impl{};
  mImpl->StartUp();
}


auto Renderer::ShutDown() -> void {
  mImpl->ShutDown();
  delete mImpl;
}


auto Renderer::DrawCamera(Camera const& cam, RenderTarget const* rt) -> void { mImpl->DrawCamera(cam, rt); }
auto Renderer::DrawAllCameras(RenderTarget const* rt) -> void { mImpl->DrawAllCameras(rt); }
auto Renderer::DrawGizmos(RenderTarget const* const rt) -> void { mImpl->DrawGizmos(rt); }
auto Renderer::BindAndClearSwapChain() noexcept -> void { mImpl->BindAndClearSwapChain(); }
auto Renderer::Present() noexcept -> void { mImpl->Present(); }
auto Renderer::GetSyncInterval() noexcept -> u32 { return mImpl->GetSyncInterval(); }
auto Renderer::SetSyncInterval(u32 const interval) noexcept -> void { mImpl->SetSyncInterval(interval); }
auto Renderer::RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void { mImpl->RegisterStaticMesh(staticMesh); }
auto Renderer::UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void { mImpl->UnregisterStaticMesh(staticMesh); }
auto Renderer::GetDevice() noexcept -> ID3D11Device* { return mImpl->GetDevice(); }
auto Renderer::GetImmediateContext() noexcept -> ID3D11DeviceContext* { return mImpl->GetImmediateContext(); }
auto Renderer::RegisterLight(LightComponent const* light) -> void { mImpl->RegisterLight(light); }
auto Renderer::UnregisterLight(LightComponent const* light) -> void { mImpl->UnregisterLight(light); }
auto Renderer::GetDefaultMaterial() noexcept -> Material* { return mImpl->GetDefaultMaterial(); }
auto Renderer::GetCubeMesh() noexcept -> Mesh* { return mImpl->GetCubeMesh(); }
auto Renderer::GetPlaneMesh() noexcept -> Mesh* { return mImpl->GetPlaneMesh(); }
auto Renderer::GetGamma() noexcept -> f32 { return mImpl->GetGamma(); }
auto Renderer::SetGamma(f32 const gamma) noexcept -> void { mImpl->SetGamma(gamma); }
auto Renderer::RegisterSkybox(SkyboxComponent const* skybox) -> void { mImpl->RegisterSkybox(skybox); }
auto Renderer::UnregisterSkybox(SkyboxComponent const* skybox) -> void { mImpl->UnregisterSkybox(skybox); }
auto Renderer::RegisterGameCamera(Camera const& cam) -> void { mImpl->RegisterGameCamera(cam); }
auto Renderer::UnregisterGameCamera(Camera const& cam) -> void { mImpl->UnregisterGameCamera(cam); }
auto Renderer::CullLights(Frustum const& frustumWS, Visibility& visibility) -> void { mImpl->CullLights(frustumWS, visibility); }
auto Renderer::CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) -> void { mImpl->CullStaticMeshComponents(frustumWS, visibility); }
auto Renderer::DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void { mImpl->DrawLineAtNextRender(from, to, color); }
auto Renderer::GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget& { return mImpl->GetTemporaryRenderTarget(desc); }
auto Renderer::GetShadowCascadeCount() noexcept -> int { return mImpl->GetShadowCascadeCount(); }
auto Renderer::SetShadowCascadeCount(int const cascadeCount) noexcept -> void { mImpl->SetShadowCascadeCount(cascadeCount); }
auto Renderer::GetMaxShadowCascadeCount() noexcept -> int { return mImpl->GetMaxShadowCascadeCount(); }
auto Renderer::GetNormalizedShadowCascadeSplits() noexcept -> std::span<float const> { return mImpl->GetNormalizedShadowCascadeSplits(); }
auto Renderer::SetNormalizedShadowCascadeSplit(int const idx, float const split) noexcept -> void { mImpl->SetNormalizedShadowCascadeSplit(idx, split); }
auto Renderer::GetShadowDistance() noexcept -> float { return mImpl->GetShadowDistance(); }
auto Renderer::SetShadowDistance(float const shadowDistance) noexcept -> void { mImpl->SetShadowDistance(shadowDistance); }
auto Renderer::IsVisualizingShadowCascades() noexcept -> bool { return mImpl->IsVisualizingShadowCascades(); }
auto Renderer::VisualizeShadowCascades(bool visualize) noexcept -> void { mImpl->VisualizeShadowCascades(visualize); }
auto Renderer::IsUsingStableShadowCascadeProjection() noexcept -> bool { return mImpl->IsUsingStableShadowCascadeProjection(); }
auto Renderer::UseStableShadowCascadeProjection(bool const useStableProj) noexcept -> void { mImpl->UseStableShadowCascadeProjection(useStableProj); }
auto Renderer::GetShadowFilteringMode() noexcept -> ShadowFilteringMode { return mImpl->GetShadowFilteringMode(); }
auto Renderer::SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void { mImpl->SetShadowFilteringMode(filteringMode); }
auto Renderer::GetInFlightFrameCount() noexcept -> int { return mImpl->GetInFlightFrameCount(); }
auto Renderer::SetInFlightFrameCount(int const count) -> void { mImpl->SetInFlightFrameCount(count); }
}
