#include "RendererImpl.hpp"

#include "Graphics.hpp"
#include "ShadowCascadeBoundary.hpp"
#include "../MemoryAllocation.hpp"
#include "../Platform.hpp"
#include "../SceneObjects/Entity.hpp"
#include "../SceneObjects/TransformComponent.hpp"
#include "../shaders/ShaderInterop.h"


#ifndef NDEBUG
#include "../shaders/generated/DepthOnlyVSBinDebug.h"
#include "../shaders/generated/DepthOnlyPSBinDebug.h"
#include "../shaders/generated/GizmoPSBinDebug.h"
#include "../shaders/generated/LineGizmoVSBinDebug.h"
#include "../shaders/generated/MeshVSBinDebug.h"
#include "../shaders/generated/MeshPbrPSBinDebug.h"
#include "../shaders/generated/PostProcessPSBinDebug.h"
#include "../shaders/generated/ScreenVSBinDebug.h"
#include "../shaders/generated/SkyboxVSBinDebug.h"
#include "../shaders/generated/SkyboxPSBinDebug.h"

#else
#include "../shaders/generated/DepthOnlyPSBin.h"
#include "../shaders/generated/DepthOnlyVSBin.h"
#include "../shaders/generated/GizmoPSBin.h"
#include "../shaders/generated/LineGizmoVSBin.h"
#include "../shaders/generated/MeshPbrPSBin.h"
#include "../shaders/generated/MeshVSBin.h"
#include "../shaders/generated/PostProcessPSBin.h"
#include "../shaders/generated/ScreenVSBin.h"
#include "../shaders/generated/SkyboxPSBin.h"
#include "../shaders/generated/SkyboxVSBin.h"
#endif

using Microsoft::WRL::ComPtr;


namespace sorcery {
namespace {
std::vector const QUAD_POSITIONS{
  Vector3{-1, 1, 0}, Vector3{-1, -1, 0}, Vector3{1, -1, 0}, Vector3{1, 1, 0}
};


std::vector const QUAD_UVS{
  Vector2{0, 0}, Vector2{0, 1}, Vector2{1, 1}, Vector2{1, 0}
};


std::vector<std::uint32_t> const QUAD_INDICES{
  2, 1, 0,
  0, 3, 2
};


std::vector const CUBE_POSITIONS{
  Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f},
  Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f},
  Vector3{-0.5f, 0.5f, -0.5f}, Vector3{-0.5f, 0.5f, -0.5f}, Vector3{-0.5f, 0.5f, -0.5f},
  Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f},
  Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f},
  Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f},
  Vector3{-0.5f, -0.5f, -0.5f}, Vector3{-0.5f, -0.5f, -0.5f}, Vector3{-0.5f, -0.5f, -0.5f},
  Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f},
};


std::vector const CUBE_UVS{
  Vector2{1, 0}, Vector2{1, 0}, Vector2{0, 0},
  Vector2{0, 0}, Vector2{0, 0}, Vector2{1, 0},
  Vector2{1, 0}, Vector2{0, 1}, Vector2{0, 0},
  Vector2{0, 0}, Vector2{1, 1}, Vector2{1, 0},
  Vector2{1, 1}, Vector2{1, 1}, Vector2{0, 1},
  Vector2{0, 1}, Vector2{0, 1}, Vector2{1, 1},
  Vector2{1, 1}, Vector2{0, 0}, Vector2{0, 1},
  Vector2{0, 1}, Vector2{1, 0}, Vector2{1, 1}
};


std::vector<std::uint32_t> const CUBE_INDICES{
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


auto Renderer::Impl::CalculateCameraShadowCascadeBoundaries(Camera const& cam) const -> ShadowCascadeBoundaries {
  auto const camNear{cam.GetNearClipPlane()};
  auto const shadowDistance{std::min(cam.GetFarClipPlane(), mShadowDistance)};
  auto const shadowedFrustumDepth{shadowDistance - camNear};

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


auto Renderer::Impl::CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) const -> void {
  visibility.staticMeshIndices.clear();

  for (int i = 0; i < static_cast<int>(mStaticMeshComponents.size()); i++) {
    if (frustumWS.Intersects(mStaticMeshComponents[i]->CalculateBounds())) {
      auto const submeshes{mStaticMeshComponents[i]->GetMesh()->GetSubMeshes()};
      for (int j{0}; j < std::ssize(submeshes); j++) {
        auto const& modelMtx{mStaticMeshComponents[i]->GetEntity().GetTransform().GetModelMatrix()};
        auto submeshBounds{submeshes[j].bounds};
        submeshBounds.min = Vector3{Vector4{submeshBounds.min, 1} * modelMtx};
        submeshBounds.max = Vector3{Vector4{submeshBounds.max, 1} * modelMtx};

        if (frustumWS.Intersects(submeshBounds)) {
          visibility.staticMeshIndices.emplace_back(i, j);
        }
      }
    }
  }
}


auto Renderer::Impl::CullLights(Frustum const& frustumWS, Visibility& visibility) const -> void {
  visibility.lightIndices.clear();

  for (int lightIdx = 0; lightIdx < static_cast<int>(mLights.size()); lightIdx++) {
    switch (auto const light{mLights[lightIdx]}; light->GetType()) {
      case LightComponent::Type::Directional: {
        visibility.lightIndices.emplace_back(lightIdx);
        break;
      }

      case LightComponent::Type::Spot: {
        auto const lightVerticesWS{
          [light] {
            auto vertices{CalculateSpotLightLocalVertices(*light)};

            for (auto const modelMtxNoScale{CalculateModelMatrixNoScale(light->GetEntity().GetTransform())};
                 auto& vertex : vertices) {
              vertex = Vector3{Vector4{vertex, 1} * modelMtxNoScale};
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
          .center = Vector3{light->GetEntity().GetTransform().GetWorldPosition()},
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


auto Renderer::Impl::DrawShadowMaps(ShadowAtlas const& atlas) -> void {
  mImmediateContext->OMSetRenderTargets(0, nullptr, atlas.GetDsv());
  mImmediateContext->OMSetDepthStencilState(GetShadowDrawDss().Get(), 0);

  mImmediateContext->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_DEPTH_ONLY_PASS, 1, mDepthOnlyCb.GetAddressOf());

  mImmediateContext->PSSetShader(mDepthOnlyPs.Get(), nullptr, 0);

  mImmediateContext->ClearDepthStencilView(atlas.GetDsv(), D3D11_CLEAR_DEPTH, Graphics::GetDepthClearValueForReversedDepth(), 0);

  mImmediateContext->RSSetState(mShadowPassRs.Get());

  mImmediateContext->IASetInputLayout(mAllAttribsIl.Get());

  auto const cellSizeNorm{atlas.GetNormalizedElementSize()};

  for (auto i = 0; i < atlas.GetElementCount(); i++) {
    auto const& cell{atlas.GetCell(i)};
    auto const cellOffsetNorm{atlas.GetNormalizedElementOffset(i)};
    auto const subcellSize{cellSizeNorm * cell.GetNormalizedElementSize() * static_cast<float>(atlas.GetSize())};

    for (auto j = 0; j < cell.GetElementCount(); j++) {
      if (auto const& subcell{cell.GetSubcell(j)}) {
        auto const subcellOffset{(cellOffsetNorm + cell.GetNormalizedElementOffset(j) * cellSizeNorm) * static_cast<float>(atlas.GetSize())};

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
        auto const depthOnlyCbData{static_cast<DepthOnlyCB*>(mapped.pData)};
        depthOnlyCbData->gDepthOnlyViewProjMtx = subcell->shadowViewProjMtx;
        mImmediateContext->Unmap(mDepthOnlyCb.Get(), 0);

        Frustum const shadowFrustumWS{subcell->shadowViewProjMtx};

        Visibility perLightVisibility{
          .lightIndices = std::pmr::vector<int>{&GetTmpMemRes()},
          .staticMeshIndices = std::pmr::vector<StaticMeshSubmeshIndex>{&GetTmpMemRes()}
        };

        CullStaticMeshComponents(shadowFrustumWS, perLightVisibility);
        DrawMeshes(perLightVisibility.staticMeshIndices);
      }
    }
  }
}


auto Renderer::Impl::DrawMeshes(std::span<StaticMeshSubmeshIndex const> const culledIndices) noexcept -> void {
  mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_DRAW, 1, mPerDrawCb.GetAddressOf());
  mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_DRAW, 1, mPerDrawCb.GetAddressOf());

  for (auto const& [meshIdx, submeshIdx] : culledIndices) {
    auto const component{mStaticMeshComponents[meshIdx]};
    auto const mesh{component->GetMesh()};

    if (!mesh) {
      continue;
    }

    std::array const vertexBuffers{mesh->GetPositionBuffer().Get(), mesh->GetNormalBuffer().Get(), mesh->GetUVBuffer().Get(), mesh->GetTangentBuffer().Get()};
    UINT constexpr strides[]{sizeof(Vector3), sizeof(Vector3), sizeof(Vector2), sizeof(Vector3)};
    UINT constexpr offsets[]{0, 0, 0, 0};
    mImmediateContext->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), strides, offsets);
    mImmediateContext->IASetIndexBuffer(mesh->GetIndexBuffer().Get(), mesh->GetIndexFormat(), 0);

    D3D11_MAPPED_SUBRESOURCE mappedPerDrawCb;
    mImmediateContext->Map(mPerDrawCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerDrawCb);
    auto const perDrawCbData{static_cast<PerDrawCB*>(mappedPerDrawCb.pData)};
    perDrawCbData->gPerDrawConstants.modelMtx = component->GetEntity().GetTransform().GetModelMatrix();
    perDrawCbData->gPerDrawConstants.normalMtx = Matrix4{component->GetEntity().GetTransform().GetNormalMatrix()};
    mImmediateContext->Unmap(mPerDrawCb.Get(), 0);

    auto const submesh{mesh->GetSubMeshes()[submeshIdx]};
    auto const mtl{component->GetMaterials()[submesh.materialIndex]};

    if (!mtl) {
      continue;
    }

    auto const mtlBuffer{mtl->GetBuffer()};
    mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);
    mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);

    auto const albedoSrv{
      [mtl] {
        auto const albedoMap{mtl->GetAlbedoMap()};
        return albedoMap ? albedoMap->GetSrv() : nullptr;
      }()
    };
    mImmediateContext->PSSetShaderResources(RES_SLOT_ALBEDO_MAP, 1, &albedoSrv);

    auto const metallicSrv{
      [mtl] {
        auto const metallicMap{mtl->GetMetallicMap()};
        return metallicMap ? metallicMap->GetSrv() : nullptr;
      }()
    };
    mImmediateContext->PSSetShaderResources(RES_SLOT_METALLIC_MAP, 1, &metallicSrv);

    auto const roughnessSrv{
      [mtl] {
        auto const roughnessMap{mtl->GetRoughnessMap()};
        return roughnessMap ? roughnessMap->GetSrv() : nullptr;
      }()
    };
    mImmediateContext->PSSetShaderResources(RES_SLOT_ROUGHNESS_MAP, 1, &roughnessSrv);

    auto const aoSrv{
      [mtl] {
        auto const aoMap{mtl->GetAoMap()};
        return aoMap ? aoMap->GetSrv() : nullptr;
      }()
    };
    mImmediateContext->PSSetShaderResources(RES_SLOT_AO_MAP, 1, &aoSrv);

    auto const normalSrv{
      [&mtl] {
        auto const normalMap{mtl->GetNormalMap()};
        return normalMap ? normalMap->GetSrv() : nullptr;
      }()
    };
    mImmediateContext->PSSetShaderResources(RES_SLOT_NORMAL_MAP, 1, &normalSrv);

    auto const opacitySrv{
      [&mtl] {
        auto const opacityMask{mtl->GetOpacityMask()};
        return opacityMask ? opacityMask->GetSrv() : nullptr;
      }()
    };
    mImmediateContext->PSSetShaderResources(RES_SLOT_OPACITY_MASK, 1, &opacitySrv);


    mImmediateContext->DrawIndexed(submesh.indexCount, submesh.firstIndex, submesh.baseVertex);
  }
}


auto Renderer::Impl::DrawSkybox(Matrix4 const& camViewMtx, Matrix4 const& camProjMtx) const noexcept -> void {
  if (mSkyboxes.empty()) {
    return;
  }

  D3D11_MAPPED_SUBRESOURCE mappedSkyboxCB;
  mImmediateContext->Map(mSkyboxCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSkyboxCB);
  auto const skyboxCBData{static_cast<SkyboxCB*>(mappedSkyboxCB.pData)};
  skyboxCBData->skyboxViewProjMtx = Matrix4{Matrix3{camViewMtx}} * camProjMtx;
  mImmediateContext->Unmap(mSkyboxCb.Get(), 0);

  ID3D11Buffer* const vertexBuffer{mCubeMesh->GetPositionBuffer().Get()};
  UINT constexpr stride{sizeof(Vector3)};
  UINT constexpr offset{0};
  mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
  mImmediateContext->IASetIndexBuffer(mCubeMesh->GetIndexBuffer().Get(), mCubeMesh->GetIndexFormat(), 0);
  mImmediateContext->IASetInputLayout(mAllAttribsIl.Get());
  mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  mImmediateContext->VSSetShader(mSkyboxVs.Get(), nullptr, 0);
  mImmediateContext->PSSetShader(mSkyboxPs.Get(), nullptr, 0);

  auto const cubemapSrv{mSkyboxes[0]->GetCubemap()->GetSrv()};
  mImmediateContext->PSSetShaderResources(RES_SLOT_SKYBOX_CUBEMAP, 1, &cubemapSrv);

  auto const cb{mSkyboxCb.Get()};
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_SKYBOX_PASS, 1, &cb);

  mImmediateContext->OMSetDepthStencilState(mDepthTestLessEqualNoWriteDss.Get(), 0);
  mImmediateContext->RSSetState(mSkyboxPassRs.Get());

  mImmediateContext->DrawIndexed(clamp_cast<UINT>(CUBE_INDICES.size()), 0, 0);

  // Restore state
  mImmediateContext->OMSetDepthStencilState(nullptr, 0);
  mImmediateContext->RSSetState(nullptr);
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
  auto const cbData{static_cast<PostProcessCB*>(mappedCb.pData)};
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


auto Renderer::Impl::OnWindowSize(Impl* const self, Extent2D<std::uint32_t> const size) -> void {
  self->mSwapChain->Resize(size.width, size.height);

  if (size.width != 0 && size.height != 0) {
    RenderTarget::Desc desc{self->mMainRt->GetDesc()};
    desc.width = size.width;
    desc.height = size.height;
    self->mMainRt = std::make_unique<RenderTarget>(desc);
  }
}


auto Renderer::Impl::StartUp() -> void {
  // CREATE DEVICE AND IMMEDIATE CONTEXT

  UINT creationFlags{0};
  D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{D3D_FEATURE_LEVEL_11_0};

#ifndef NDEBUG
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, requestedFeatureLevels, 1, D3D11_SDK_VERSION, mDevice.GetAddressOf(), nullptr, mImmediateContext.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create D3D device."};
  }

  // SET DEBUG BREAKS

#ifndef NDEBUG
  ComPtr<ID3D11Debug> d3dDebug;
  if (FAILED(mDevice.As(&d3dDebug))) {
    throw std::runtime_error{"Failed to get ID3D11Debug interface."};
  }

  ComPtr<ID3D11InfoQueue> d3dInfoQueue;
  if (FAILED(d3dDebug.As<ID3D11InfoQueue>(&d3dInfoQueue))) {
    throw std::runtime_error{"Failed to get ID3D11InfoQueue interface."};
  }

  d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
  d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

  if (FAILED(mDevice.As(&mDxgiDevice))) {
    throw std::runtime_error{"Failed to query IDXGIDevice interface."};
  }

  SetInFlightFrameCount(2);

  ComPtr<IDXGIAdapter> dxgiAdapter;
  if (FAILED(mDxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()))) {
    throw std::runtime_error{"Failed to get IDXGIAdapter."};
  }

  ComPtr<IDXGIFactory2> dxgiFactory2;
  if (FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory2.GetAddressOf())))) {
    throw std::runtime_error{"Failed to query IDXGIFactory2 interface."};
  }

  mSwapChain = std::make_unique<SwapChain>(mDevice, dxgiFactory2.Get());

  mLightBuffer = std::make_unique<StructuredBuffer<ShaderLight>>(mDevice, mImmediateContext);
  mGizmoColorBuffer = std::make_unique<StructuredBuffer<Vector4>>(mDevice, mImmediateContext);
  mLineGizmoVertexDataBuffer = std::make_unique<StructuredBuffer<ShaderLineGizmoVertexData>>(mDevice, mImmediateContext);
  mMainRt = std::make_unique<RenderTarget>(RenderTarget::Desc{
    .width = gWindow.GetCurrentClientAreaSize().width,
    .height = gWindow.GetCurrentClientAreaSize().height,
    .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    .depthBufferBitCount = 0,
    .stencilBufferBitCount = 0,
    .debugName = "Main RT"
  });

  // CREATE INPUT LAYOUTS

  D3D11_INPUT_ELEMENT_DESC constexpr inputDescs[]{
    {
      .SemanticName = "POSITION",
      .SemanticIndex = 0,
      .Format = DXGI_FORMAT_R32G32B32_FLOAT,
      .InputSlot = 0,
      .AlignedByteOffset = 0,
      .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
      .InstanceDataStepRate = 0
    },
    {
      .SemanticName = "NORMAL",
      .SemanticIndex = 0,
      .Format = DXGI_FORMAT_R32G32B32_FLOAT,
      .InputSlot = 1,
      .AlignedByteOffset = 0,
      .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
      .InstanceDataStepRate = 0
    },
    {
      .SemanticName = "TEXCOORD",
      .SemanticIndex = 0,
      .Format = DXGI_FORMAT_R32G32_FLOAT,
      .InputSlot = 2,
      .AlignedByteOffset = 0,
      .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
      .InstanceDataStepRate = 0
    },
    {
      .SemanticName = "TANGENT",
      .SemanticIndex = 0,
      .Format = DXGI_FORMAT_R32G32B32_FLOAT,
      .InputSlot = 3,
      .AlignedByteOffset = 0,
      .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
      .InstanceDataStepRate = 0
    }
  };

  if (FAILED(mDevice->CreateInputLayout(inputDescs, ARRAYSIZE(inputDescs), gMeshVSBin, ARRAYSIZE(gMeshVSBin), mAllAttribsIl.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create all-attributes input layout."};
  }

  // CREATE SHADERS

  if (FAILED(mDevice->CreateVertexShader(gMeshVSBin, ARRAYSIZE(gMeshVSBin), nullptr, mMeshVs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create mesh vertex shader."};
  }

  char constexpr meshVsName[]{"Mesh Vertex Shader"};
  mMeshVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(meshVsName), meshVsName);

  if (FAILED(mDevice->CreatePixelShader(gMeshPbrPSBin, ARRAYSIZE(gMeshPbrPSBin), nullptr, mMeshPbrPs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create mesh pbr pixel shader."};
  }

  char constexpr meshPbrPsName[]{"Mesh PBR Pixel Shader"};
  mMeshPbrPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(meshPbrPsName), meshPbrPsName);

  if (FAILED(mDevice->CreatePixelShader(gPostProcessPSBin, ARRAYSIZE(gPostProcessPSBin), nullptr, mPostProcessPs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create textured post process pixel shader."};
  }

  char constexpr postProcessPsName[]{"Postprocess Pixel Shader"};
  mPostProcessPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(postProcessPsName), postProcessPsName);

  if (FAILED(mDevice->CreateVertexShader(gSkyboxVSBin, ARRAYSIZE(gSkyboxVSBin), nullptr, mSkyboxVs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to create skybox vertex shader."};
  }

  char constexpr skyboxVsName[]{"Skybox Vertex Shader"};
  mSkyboxVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(skyboxVsName), skyboxVsName);

  if (FAILED(mDevice->CreatePixelShader(gSkyboxPSBin, ARRAYSIZE(gSkyboxPSBin), nullptr, mSkyboxPs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to create skybox pixel shader."};
  }

  char constexpr skyboxPsName[]{"Skybox Pixel Shader"};
  mSkyboxPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(skyboxPsName), skyboxPsName);

  if (FAILED(mDevice->CreateVertexShader(gDepthOnlyVSBin, ARRAYSIZE(gDepthOnlyVSBin), nullptr, mDepthOnlyVs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-only vertex shader."};
  }

  char constexpr depthOnlyVsName[]{"Depth-Only Vertex Shader"};
  mDepthOnlyVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(depthOnlyVsName), depthOnlyVsName);

  if (FAILED(mDevice->CreatePixelShader(gDepthOnlyPSBin, ARRAYSIZE(gDepthOnlyPSBin), nullptr, mDepthOnlyPs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-only pixel shader."};
  }

  char constexpr depthOnlyPsName[]{"Depth-Only Pixel Shader"};
  mDepthOnlyPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(depthOnlyPsName), depthOnlyPsName);

  if (FAILED(mDevice->CreateVertexShader(gScreenVSBin, ARRAYSIZE(gScreenVSBin), nullptr, mScreenVs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create screen vertex shader."};
  }

  char constexpr screenVsName[]{"Screen Vertex Shader"};
  mScreenVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(screenVsName), screenVsName);

  if (FAILED(mDevice->CreateVertexShader(gLineGizmoVSBin, ARRAYSIZE(gLineGizmoVSBin), nullptr, mLineGizmoVs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create line gizmo vertex shader."};
  }

  char constexpr lineGizmoVsName[]{"Line Gizmo Vertex Shader"};
  mLineGizmoVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(lineGizmoVsName), lineGizmoVsName);

  if (FAILED(mDevice->CreatePixelShader(gGizmoPSBin, ARRAYSIZE(gGizmoPSBin), nullptr, mGizmoPs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create gizmo pixel shader."};
  }

  char constexpr gizmoPsName[]{"Gizmo Pixel Shader"};
  mGizmoPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(gizmoPsName), gizmoPsName);

  // CREATE CONSTANT BUFFERS


  D3D11_BUFFER_DESC constexpr perFrameCbDesc{
    .ByteWidth = sizeof(PerFrameCB),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&perFrameCbDesc, nullptr, mPerFrameCb.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create per frame CB."};
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
    throw std::runtime_error{"Failed to create per camera CB."};
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
    throw std::runtime_error{"Failed to create per draw CB."};
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
    throw std::runtime_error{"Failed to create post-process CB."};
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
    throw std::runtime_error{"Failed to create skybox pass CB."};
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
    throw std::runtime_error{"Failed to create depth-only pass CB."};
  }

  // CREATE RASTERIZER STATES

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
    throw std::runtime_error{"Failed to create skybox pass rasterizer state."};
  }

  D3D11_RASTERIZER_DESC constexpr shadowPassRasterizerDesc{
    .FillMode = D3D11_FILL_SOLID,
    .CullMode = D3D11_CULL_BACK,
    .FrontCounterClockwise = FALSE,
    .DepthBias = -1,
    .DepthBiasClamp = 0,
    .SlopeScaledDepthBias = -2.5,
    .DepthClipEnable = TRUE,
    .ScissorEnable = FALSE,
    .MultisampleEnable = FALSE,
    .AntialiasedLineEnable = FALSE
  };

  if (FAILED(mDevice->CreateRasterizerState(&shadowPassRasterizerDesc, mShadowPassRs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create shadow pass rasterizer state."};
  }

  // CREATE DEPTH STENCIL STATES


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
    throw std::runtime_error{"Failed to create depth-test greater write depth-stencil state."};
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
    throw std::runtime_error{"Failed to create depth-test less write depth-stencil state."};
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
    throw std::runtime_error{"Failed to create depth-test less-or-equal no-write DSS."};
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
    throw std::runtime_error{"Failed to create depth-test greater-or-equal no-write DSS."};
  }

  // CREATE SHADOW ATLASES

  //mDirectionalShadowAtlas = std::make_unique<DirectionalShadowAtlas>(mDevice.Get(), 4096);
  mDirShadowMapArr = std::make_unique<DirectionalShadowMapArray>(mDevice.Get(), 4096);
  mPunctualShadowAtlas = std::make_unique<PunctualShadowAtlas>(mDevice.Get(), 4096);

  // CREATE SAMPLER STATES


  D3D11_SAMPLER_DESC constexpr cmpPcfGreaterEqual{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL,
    .BorderColor = {0, 0, 0, 0},
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPcfGreaterEqual, mCmpPcfGreaterEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create PCF greater-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr cmpPcfLessEqual{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,
    .BorderColor = {0, 0, 0, 0},
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPcfLessEqual, mCmpPcfLessEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create PCF less-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr cmpPointGreaterEqualDesc{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL,
    .BorderColor = {0, 0, 0, 0},
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPointGreaterEqualDesc, mCmpPointGreaterEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create point-filter greater-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr cmpPointLessEqualDesc{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressW = D3D11_TEXTURE_ADDRESS_BORDER,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL,
    .BorderColor = {0, 0, 0, 0},
    .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPointLessEqualDesc, mCmpPointLessEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create point-filter less-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af16Desc{
    .Filter = D3D11_FILTER_ANISOTROPIC,
    .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    .MipLODBias = 0,
    .MaxAnisotropy = 16,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af16Desc, mAf16Ss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create AF16 sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af8Desc{
    .Filter = D3D11_FILTER_ANISOTROPIC,
    .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    .MipLODBias = 0,
    .MaxAnisotropy = 8,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af8Desc, mAf8Ss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create AF8 sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af4Desc{
    .Filter = D3D11_FILTER_ANISOTROPIC,
    .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    .MipLODBias = 0,
    .MaxAnisotropy = 4,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af4Desc, mAf4Ss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create AF4 sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr trilinearDesc{
    .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
    .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&trilinearDesc, mTrilinearSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create trilinear sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr bilinearDesc{
    .Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&bilinearDesc, mBilinearSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create bilinear sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr pointDesc{
    .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT,
    .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
    .MipLODBias = 0,
    .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER,
    .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&pointDesc, mPointSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create point-filter sampler state."};
  }

  // CREATE DEFAULT ASSETS

  mDefaultMaterial = CreateAndInitialize<Material>();
  mDefaultMaterial->SetGuid(DEFAULT_MATERIAL_GUID);
  mDefaultMaterial->SetName("Default Material");
  gResourceManager.Add(mDefaultMaterial);

  std::vector<Vector3> cubeNormals;
  CalculateNormals(CUBE_POSITIONS, CUBE_INDICES, cubeNormals);

  std::vector<Vector3> cubeTangents;
  CalculateTangents(CUBE_POSITIONS, CUBE_UVS, CUBE_INDICES, cubeTangents);

  std::vector<Vector3> quadNormals;
  CalculateNormals(QUAD_POSITIONS, QUAD_INDICES, quadNormals);

  std::vector<Vector3> quadTangents;
  CalculateTangents(QUAD_POSITIONS, QUAD_UVS, QUAD_INDICES, quadTangents);

  mCubeMesh = CreateAndInitialize<Mesh>();
  mCubeMesh->SetGuid(CUBE_MESH_GUID);
  mCubeMesh->SetName("Cube");
  mCubeMesh->SetPositions(CUBE_POSITIONS);
  mCubeMesh->SetNormals(std::move(cubeNormals));
  mCubeMesh->SetUVs(CUBE_UVS);
  mCubeMesh->SetTangents(std::move(cubeTangents));
  mCubeMesh->SetIndices(CUBE_INDICES);
  mCubeMesh->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  mCubeMesh->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(CUBE_INDICES.size()), 0, AABB{}}});
  if (!mCubeMesh->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default cube mesh."};
  }
  gResourceManager.Add(mCubeMesh);

  mPlaneMesh = CreateAndInitialize<Mesh>();
  mPlaneMesh->SetGuid(PLANE_MESH_GUID);
  mPlaneMesh->SetName("Plane");
  mPlaneMesh->SetPositions(QUAD_POSITIONS);
  mPlaneMesh->SetNormals(std::move(quadNormals));
  mPlaneMesh->SetUVs(QUAD_UVS);
  mPlaneMesh->SetTangents(std::move(quadTangents));
  mPlaneMesh->SetIndices(QUAD_INDICES);
  mPlaneMesh->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  mPlaneMesh->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(QUAD_INDICES.size()), 0, AABB{}}});
  if (!mPlaneMesh->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default plane mesh."};
  }
  gResourceManager.Add(mPlaneMesh);

  gWindow.OnWindowSize.add_handler(this, &OnWindowSize);

  dxgiFactory2->MakeWindowAssociation(gWindow.GetHandle(), DXGI_MWA_NO_WINDOW_CHANGES);
}


auto Renderer::Impl::ShutDown() -> void {
  gWindow.OnWindowSize.remove_handler(this, &OnWindowSize);
}


auto Renderer::Impl::DrawCamera(Camera const& cam, RenderTarget const* const rt) -> void {
  auto const rtWidth{rt ? rt->GetDesc().width : gWindow.GetCurrentClientAreaSize().width};
  auto const rtHeight{rt ? rt->GetDesc().height : gWindow.GetCurrentClientAreaSize().height};
  auto const rtAspect{static_cast<float>(rtWidth) / static_cast<float>(rtHeight)};

  RenderTarget::Desc const hdrRtDesc{
    .width = rtWidth,
    .height = rtHeight,
    .colorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT,
    .depthBufferBitCount = 32,
    .stencilBufferBitCount = 0,
    .sampleCount = static_cast<int>(GetMultisamplingMode()),
    .debugName = "Camera HDR RenderTarget"
  };

  auto const& hdrRt{GetTemporaryRenderTarget(hdrRtDesc)};

  FLOAT constexpr clearColor[]{0, 0, 0, 1};
  mImmediateContext->ClearRenderTargetView(hdrRt.GetRtv(), clearColor);


  D3D11_VIEWPORT const viewport{
    .TopLeftX = 0,
    .TopLeftY = 0,
    .Width = static_cast<FLOAT>(rtWidth),
    .Height = static_cast<FLOAT>(rtHeight),
    .MinDepth = 0,
    .MaxDepth = 1
  };

  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_CMP_PCF, 1, GetShadowPcfSampler().GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_CMP_POINT, 1, GetShadowPointSampler().GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_AF16, 1, mAf16Ss.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_AF8, 1, mAf8Ss.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_AF4, 1, mAf4Ss.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_TRI, 1, mTrilinearSs.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_BI, 1, mBilinearSs.GetAddressOf());
  mImmediateContext->PSSetSamplers(SAMPLER_SLOT_POINT, 1, mPointSs.GetAddressOf());

  D3D11_MAPPED_SUBRESOURCE mappedPerFrameCB;
  if (FAILED(mImmediateContext->Map(mPerFrameCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerFrameCB))) {
    throw std::runtime_error{"Failed to map per frame CB."};
  }

  auto const perFrameCbData{static_cast<PerFrameCB*>(mappedPerFrameCB.pData)};
  perFrameCbData->gPerFrameConstants.shadowCascadeCount = mCascadeCount;
  perFrameCbData->gPerFrameConstants.visualizeShadowCascades = mVisualizeShadowCascades;
  perFrameCbData->gPerFrameConstants.shadowFilteringMode = static_cast<int>(mShadowFilteringMode);
  perFrameCbData->gPerFrameConstants.ambientLightColor = mAmbientLightColor;

  mImmediateContext->Unmap(mPerFrameCb.Get(), 0);

  mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, mPerFrameCb.GetAddressOf());
  mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, mPerFrameCb.GetAddressOf());

  auto const camPos{cam.GetPosition()};
  auto const camViewMtx{cam.CalculateViewMatrix()};
  auto const camProjMtx{cam.CalculateProjectionMatrix(rtAspect)};
  auto const camViewProjMtx{camViewMtx * camProjMtx};
  Frustum const camFrustWS{camViewProjMtx};

  Visibility visibility{
    .lightIndices = std::pmr::vector<int>{&GetTmpMemRes()},
    .staticMeshIndices = std::pmr::vector<StaticMeshSubmeshIndex>{&GetTmpMemRes()}
  };
  CullLights(camFrustWS, visibility);

  // Shadow pass

  auto const shadowCascadeBoundaries{CalculateCameraShadowCascadeBoundaries(cam)};

  ID3D11ShaderResourceView* const nullSrv{nullptr};
  mImmediateContext->PSSetShaderResources(RES_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, &nullSrv);
  mImmediateContext->PSSetShaderResources(RES_SLOT_DIR_SHADOW_ATLAS, 1, &nullSrv);
  mImmediateContext->PSSetShader(nullptr, nullptr, 0);
  mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  std::array<Matrix4, MAX_CASCADE_COUNT> shadowViewProjMatrices;

  for (auto const lightIdx : visibility.lightIndices) {
    if (auto const light{mLights[lightIdx]}; light->GetType() == LightComponent::Type::Directional && light->IsCastingShadow()) {
      float const camNear{cam.GetNearClipPlane()};
      float const camFar{cam.GetFarClipPlane()};

      enum FrustumVertex : int {
        FrustumVertex_NearTopRight    = 0,
        FrustumVertex_NearTopLeft     = 1,
        FrustumVertex_NearBottomLeft  = 2,
        FrustumVertex_NearBottomRight = 3,
        FrustumVertex_FarTopRight     = 4,
        FrustumVertex_FarTopLeft      = 5,
        FrustumVertex_FarBottomLeft   = 6,
        FrustumVertex_FarBottomRight  = 7,
      };

      // Order of vertices is CCW from top right, near first
      auto const frustumVertsWS{
        [&cam, rtAspect, camNear, camFar] {
          std::array<Vector3, 8> ret;

          Vector3 const nearWorldForward{cam.GetPosition() + cam.GetForwardAxis() * camNear};
          Vector3 const farWorldForward{cam.GetPosition() + cam.GetForwardAxis() * camFar};

          switch (cam.GetType()) {
            case Camera::Type::Perspective: {
              float const tanHalfFov{std::tan(ToRadians(cam.GetHorizontalPerspectiveFov() / 2.0f))};
              float const nearExtentX{camNear * tanHalfFov};
              float const nearExtentY{nearExtentX / rtAspect};
              float const farExtentX{camFar * tanHalfFov};
              float const farExtentY{farExtentX / rtAspect};

              ret[FrustumVertex_NearTopRight] = nearWorldForward + cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() * nearExtentY;
              ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() * nearExtentY;
              ret[FrustumVertex_NearBottomLeft] = nearWorldForward - cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
              ret[FrustumVertex_NearBottomRight] = nearWorldForward + cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
              ret[FrustumVertex_FarTopRight] = farWorldForward + cam.GetRightAxis() * farExtentX + cam.GetUpAxis() * farExtentY;
              ret[FrustumVertex_FarTopLeft] = farWorldForward - cam.GetRightAxis() * farExtentX + cam.GetUpAxis() * farExtentY;
              ret[FrustumVertex_FarBottomLeft] = farWorldForward - cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
              ret[FrustumVertex_FarBottomRight] = farWorldForward + cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
              break;
            }
            case Camera::Type::Orthographic: {
              float const extentX{cam.GetHorizontalOrthographicSize() / 2.0f};
              float const extentY{extentX / rtAspect};

              ret[FrustumVertex_NearTopRight] = nearWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
              ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
              ret[FrustumVertex_NearBottomLeft] = nearWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
              ret[FrustumVertex_NearBottomRight] = nearWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
              ret[FrustumVertex_FarTopRight] = farWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
              ret[FrustumVertex_FarTopLeft] = farWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
              ret[FrustumVertex_FarBottomLeft] = farWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
              ret[FrustumVertex_FarBottomRight] = farWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
              break;
            }
          }

          return ret;
        }()
      };

      auto const frustumDepth{camFar - camNear};

      for (auto cascadeIdx{0}; cascadeIdx < mCascadeCount; cascadeIdx++) {
        // cascade vertices in world space
        auto const cascadeVertsWS{
          [&frustumVertsWS, &shadowCascadeBoundaries, cascadeIdx, camNear, frustumDepth] {
            auto const [cascadeNear, cascadeFar]{shadowCascadeBoundaries[cascadeIdx]};

            float const cascadeNearNorm{(cascadeNear - camNear) / frustumDepth};
            float const cascadeFarNorm{(cascadeFar - camNear) / frustumDepth};

            std::array<Vector3, 8> ret;

            for (int j = 0; j < 4; j++) {
              Vector3 const& from{frustumVertsWS[j]};
              Vector3 const& to{frustumVertsWS[j + 4]};

              ret[j] = Lerp(from, to, cascadeNearNorm);
              ret[j + 4] = Lerp(from, to, cascadeFarNorm);
            }

            return ret;
          }()
        };

        Vector3 cascadeCenterWS{Vector3::Zero()};

        for (Vector3 const& cascadeVertWS : cascadeVertsWS) {
          cascadeCenterWS += cascadeVertWS;
        }

        cascadeCenterWS /= 8.0f;

        float sphereRadius{0.0f};

        for (Vector3 const& cascadeVertWS : cascadeVertsWS) {
          sphereRadius = std::max(sphereRadius, Distance(cascadeCenterWS, cascadeVertWS));
        }

        auto const shadowMapSize{mDirShadowMapArr->GetSize()};
        auto const worldUnitsPerTexel{sphereRadius * 2.0f / static_cast<float>(shadowMapSize)};

        Matrix4 const shadowViewMtx{Matrix4::LookToLH(Vector3::Zero(), light->GetDirection(), Vector3::Up())};
        cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx};
        cascadeCenterWS /= worldUnitsPerTexel;
        cascadeCenterWS[0] = std::floor(cascadeCenterWS[0]);
        cascadeCenterWS[1] = std::floor(cascadeCenterWS[1]);
        cascadeCenterWS *= worldUnitsPerTexel;
        cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx.Inverse()};

        auto shadowNearClipPlane{-sphereRadius - light->GetShadowExtension()};
        auto shadowFarClipPlane{sphereRadius};
        Graphics::AdjustClipPlanesForReversedDepth(shadowNearClipPlane, shadowFarClipPlane);
        Matrix4 const shadowProjMtx{Matrix4::OrthographicAsymZLH(-sphereRadius, sphereRadius, sphereRadius, -sphereRadius, shadowNearClipPlane, shadowFarClipPlane)};

        shadowViewProjMatrices[cascadeIdx] = Matrix4::LookToLH(cascadeCenterWS, light->GetDirection(), Vector3::Up()) * shadowProjMtx;

        mImmediateContext->OMSetRenderTargets(0, nullptr, mDirShadowMapArr->GetDsv(cascadeIdx));
        mImmediateContext->OMSetDepthStencilState(GetShadowDrawDss().Get(), 0);

        mImmediateContext->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);
        mImmediateContext->VSSetConstantBuffers(CB_SLOT_DEPTH_ONLY_PASS, 1, mDepthOnlyCb.GetAddressOf());

        mImmediateContext->PSSetShader(mDepthOnlyPs.Get(), nullptr, 0);

        mImmediateContext->ClearDepthStencilView(mDirShadowMapArr->GetDsv(cascadeIdx), D3D11_CLEAR_DEPTH, Graphics::GetDepthClearValueForReversedDepth(), 0);

        mImmediateContext->RSSetState(mShadowPassRs.Get());

        mImmediateContext->IASetInputLayout(mAllAttribsIl.Get());

        D3D11_VIEWPORT const shadowViewport{
          .TopLeftX = 0,
          .TopLeftY = 0,
          .Width = static_cast<float>(shadowMapSize),
          .Height = static_cast<float>(shadowMapSize),
          .MinDepth = 0,
          .MaxDepth = 1
        };

        mImmediateContext->RSSetViewports(1, &shadowViewport);

        D3D11_MAPPED_SUBRESOURCE mapped;
        mImmediateContext->Map(mDepthOnlyCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        auto const depthOnlyCbData{static_cast<DepthOnlyCB*>(mapped.pData)};
        depthOnlyCbData->gDepthOnlyViewProjMtx = shadowViewProjMatrices[cascadeIdx];
        mImmediateContext->Unmap(mDepthOnlyCb.Get(), 0);

        Frustum const shadowFrustumWS{shadowViewProjMatrices[cascadeIdx]};

        Visibility perLightVisibility{
          .lightIndices = std::pmr::vector<int>{&GetTmpMemRes()},
          .staticMeshIndices = std::pmr::vector<StaticMeshSubmeshIndex>{&GetTmpMemRes()}
        };

        CullStaticMeshComponents(shadowFrustumWS, perLightVisibility);
        DrawMeshes(perLightVisibility.staticMeshIndices);
      }

      break;
    }
  }

  //mDirectionalShadowAtlas->Update(mLights, visibility, cam, shadowCascadeBoundaries, rtAspect, mCascadeCount);
  //DrawShadowMaps(*mDirectionalShadowAtlas);

  mPunctualShadowAtlas->Update(mLights, visibility, cam, camViewProjMtx, mShadowDistance);
  DrawShadowMaps(*mPunctualShadowAtlas);

  CullStaticMeshComponents(camFrustWS, visibility);

  // Depth pre-pass

  mImmediateContext->OMSetRenderTargets(0, nullptr, hdrRt.GetDsv());
  mImmediateContext->OMSetDepthStencilState(mDepthTestLessWriteDss.Get(), 0);

  mImmediateContext->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_DEPTH_ONLY_PASS, 1, mDepthOnlyCb.GetAddressOf());

  mImmediateContext->PSSetShader(mDepthOnlyPs.Get(), nullptr, 0);

  mImmediateContext->RSSetViewports(1, &viewport);
  mImmediateContext->RSSetState(nullptr);

  mImmediateContext->IASetInputLayout(mAllAttribsIl.Get());

  mImmediateContext->ClearDepthStencilView(hdrRt.GetDsv(), D3D11_CLEAR_DEPTH, 1.0f, 0);

  D3D11_MAPPED_SUBRESOURCE mappedCb;
  if (FAILED(mImmediateContext->Map(mDepthOnlyCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCb))) {
    throw std::runtime_error{"Failed to map CB for depth pre-pass."};
  }
  static_cast<DepthOnlyCB*>(mappedCb.pData)->gDepthOnlyViewProjMtx = camViewProjMtx;
  mImmediateContext->Unmap(mDepthOnlyCb.Get(), 0);

  DrawMeshes(visibility.staticMeshIndices);

  // Full forward lighting pass

  D3D11_MAPPED_SUBRESOURCE mappedPerCamCB;
  if (FAILED(mImmediateContext->Map(mPerCamCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedPerCamCB))) {
    throw std::runtime_error{"Failed to map per camera CB."};
  }

  auto const perCamCbData{static_cast<PerCameraCB*>(mappedPerCamCB.pData)};
  perCamCbData->gPerCamConstants.viewProjMtx = camViewProjMtx;
  perCamCbData->gPerCamConstants.camPos = camPos;

  for (int i = 0; i < MAX_CASCADE_COUNT; i++) {
    perCamCbData->gPerCamConstants.shadowCascadeSplitDistances[i] = shadowCascadeBoundaries[i].farClip;
  }

  mImmediateContext->Unmap(mPerCamCb.Get(), 0);

  auto const lightCount{std::ssize(visibility.lightIndices)};
  mLightBuffer->Resize(static_cast<int>(lightCount));
  auto const lightBufferData{mLightBuffer->Map()};

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

  for (auto i{0}; i < lightCount; i++) {
    if (auto const light{mLights[visibility.lightIndices[i]]}; light->GetType() == LightComponent::Type::Directional && light->IsCastingShadow()) {
      lightBufferData[i].isCastingShadow = TRUE;

      for (auto cascadeIdx{0}; cascadeIdx < mCascadeCount; cascadeIdx++) {
        lightBufferData[i].sampleShadowMap[cascadeIdx] = TRUE;
        lightBufferData[i].shadowViewProjMatrices[cascadeIdx] = shadowViewProjMatrices[cascadeIdx];
      }

      break;
    }
  }

  //mDirectionalShadowAtlas->SetLookUpInfo(lightBufferData);
  mPunctualShadowAtlas->SetLookUpInfo(lightBufferData);

  mLightBuffer->Unmap();

  mImmediateContext->VSSetShader(mMeshVs.Get(), nullptr, 0);
  mImmediateContext->VSSetConstantBuffers(CB_SLOT_PER_CAM, 1, mPerCamCb.GetAddressOf());

  mImmediateContext->OMSetRenderTargets(1, std::array{hdrRt.GetRtv()}.data(), hdrRt.GetDsv());
  mImmediateContext->OMSetDepthStencilState(mDepthTestLessEqualNoWriteDss.Get(), 0);

  mImmediateContext->PSSetShader(mMeshPbrPs.Get(), nullptr, 0);
  mImmediateContext->PSSetConstantBuffers(CB_SLOT_PER_CAM, 1, mPerCamCb.GetAddressOf());
  mImmediateContext->PSSetShaderResources(RES_SLOT_LIGHTS, 1, std::array{mLightBuffer->GetSrv()}.data());
  mImmediateContext->PSSetShaderResources(RES_SLOT_DIR_SHADOW_MAP_ARRAY, 1, std::array{mDirShadowMapArr->GetSrv()}.data());
  mImmediateContext->PSSetShaderResources(RES_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, std::array{mPunctualShadowAtlas->GetSrv()}.data());

  mImmediateContext->RSSetViewports(1, &viewport);
  mImmediateContext->RSSetState(nullptr);

  mImmediateContext->IASetInputLayout(mAllAttribsIl.Get());

  DrawMeshes(visibility.staticMeshIndices);
  DrawSkybox(camViewMtx, camProjMtx);

  RenderTarget const* postProcessRt;

  if (GetMultisamplingMode() == MultisamplingMode::Off) {
    postProcessRt = std::addressof(hdrRt);
  } else {
    auto resolveHdrRtDesc{hdrRtDesc};
    resolveHdrRtDesc.sampleCount = 1;
    auto const& resolveHdrRt{GetTemporaryRenderTarget(resolveHdrRtDesc)};
    mImmediateContext->ResolveSubresource(resolveHdrRt.GetColorTexture(), 0, hdrRt.GetColorTexture(), 0, *hdrRtDesc.colorFormat);
    postProcessRt = std::addressof(resolveHdrRt);
  }

  mImmediateContext->RSSetViewports(1, &viewport);
  PostProcess(postProcessRt->GetColorSrv(), rt ? rt->GetRtv() : mSwapChain->GetRtv());
}


auto Renderer::Impl::DrawAllCameras(RenderTarget const* const rt) -> void {
  for (auto const& cam : mGameRenderCameras) {
    DrawCamera(*cam, rt);
  }
}


auto Renderer::Impl::DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void {
  mGizmoColors.emplace_back(color);
  mLineGizmoVertexData.emplace_back(from, static_cast<std::uint32_t>(mGizmoColors.size() - 1), to, 0.0f);
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
  mImmediateContext->PSSetShaderResources(RES_SLOT_GIZMO_COLOR, 1, std::array{mGizmoColorBuffer->GetSrv()}.data());

  if (!mLineGizmoVertexData.empty()) {
    mImmediateContext->VSSetShader(mLineGizmoVs.Get(), nullptr, 0);
    mImmediateContext->VSSetShaderResources(RES_SLOT_LINE_GIZMO_VERTEX, 1, std::array{mLineGizmoVertexDataBuffer->GetSrv()}.data());
    mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    mImmediateContext->DrawInstanced(2, static_cast<UINT>(mLineGizmoVertexData.size()), 0, 0);
  }
}


auto Renderer::Impl::ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
  auto const rtv{mMainRt->GetRtv()};
  FLOAT constexpr clearColor[]{0, 0, 0, 1};
  ctx->ClearRenderTargetView(rtv, clearColor);
  ctx->OMSetRenderTargets(1, &rtv, nullptr);
}


auto Renderer::Impl::BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
  ComPtr<ID3D11Resource> mainRtColorTex;
  mMainRt->GetRtv()->GetResource(mainRtColorTex.GetAddressOf());

  ComPtr<ID3D11Resource> backBuf;
  mSwapChain->GetRtv()->GetResource(backBuf.GetAddressOf());

  ctx->CopyResource(backBuf.Get(), mainRtColorTex.Get());
}


auto Renderer::Impl::Present() noexcept -> void {
  mSwapChain->Present(static_cast<UINT>(mSyncInterval));

  ClearGizmoDrawQueue();
  ReleaseTempRenderTargets();
}


auto Renderer::Impl::GetDevice() const noexcept -> ID3D11Device* {
  return mDevice.Get();
}


auto Renderer::Impl::GetThreadContext() noexcept -> ObserverPtr<ID3D11DeviceContext> {
  auto const thisThreadId{std::this_thread::get_id()};

  std::unique_lock const lock{mPerThreadCtxMutex};

  if (auto const it{mPerThreadCtx.find(thisThreadId)}; it != std::end(mPerThreadCtx)) {
    return it->second.Get();
  }

  ComPtr<ID3D11DeviceContext> thisThreadCtx;
  [[maybe_unused]] auto const hr{mDevice->CreateDeferredContext(0, thisThreadCtx.GetAddressOf())};
  assert(SUCCEEDED(hr));

  [[maybe_unused]] auto const& [it, inserted]{mPerThreadCtx.emplace(thisThreadId, std::move(thisThreadCtx))};
  assert(inserted);

  return it->second.Get();
}


auto Renderer::Impl::ExecuteCommandList(ObserverPtr<ID3D11CommandList> const cmdList) noexcept -> void {
  std::unique_lock const lock{mImmediateCtxMutex};
  mImmediateContext->ExecuteCommandList(cmdList, FALSE);
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


auto Renderer::Impl::GetDefaultMaterial() const noexcept -> ObserverPtr<Material> {
  return mDefaultMaterial;
}


auto Renderer::Impl::GetCubeMesh() const noexcept -> ObserverPtr<Mesh> {
  return mCubeMesh;
}


auto Renderer::Impl::GetPlaneMesh() const noexcept -> ObserverPtr<Mesh> {
  return mPlaneMesh;
}


auto Renderer::Impl::GetSyncInterval() const noexcept -> int {
  return mSyncInterval;
}


auto Renderer::Impl::SetSyncInterval(int const interval) noexcept -> void {
  mSyncInterval = interval;
}


auto Renderer::Impl::GetInFlightFrameCount() const noexcept -> int {
  return mInFlightFrameCount;
}


auto Renderer::Impl::SetInFlightFrameCount(int const count) -> void {
  mInFlightFrameCount = std::clamp(count, MIN_IN_FLIGHT_FRAME_COUNT, MAX_IN_FLIGHT_FRAME_COUNT);

  if (FAILED(mDxgiDevice->SetMaximumFrameLatency(mInFlightFrameCount))) {
    throw std::runtime_error{"Failed to set in-flight frame count."};
  }
}


auto Renderer::Impl::GetMultisamplingMode() const noexcept -> MultisamplingMode {
  return mMsaaMode;
}


auto Renderer::Impl::SetMultisamplingMode(MultisamplingMode const mode) noexcept -> void {
  mMsaaMode = mode;
}


auto Renderer::Impl::GetShadowDistance() const noexcept -> float {
  return mShadowDistance;
}


auto Renderer::Impl::SetShadowDistance(float const shadowDistance) noexcept -> void {
  mShadowDistance = std::max(shadowDistance, 0.0f);
}


auto Renderer::Impl::GetShadowCascadeCount() const noexcept -> int {
  return mCascadeCount;
}


auto Renderer::Impl::SetShadowCascadeCount(int const cascadeCount) noexcept -> void {
  mCascadeCount = std::clamp(cascadeCount, 1, MAX_CASCADE_COUNT);
  int const splitCount{mCascadeCount - 1};

  for (int i = 1; i < splitCount; i++) {
    mCascadeSplits[i] = std::max(mCascadeSplits[i - 1], mCascadeSplits[i]);
  }
}


auto Renderer::Impl::GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const> {
  return {std::begin(mCascadeSplits), static_cast<std::size_t>(mCascadeCount - 1)};
}


auto Renderer::Impl::SetNormalizedShadowCascadeSplit(int const idx, float const split) noexcept -> void {
  auto const splitCount{mCascadeCount - 1};

  if (idx < 0 || idx >= splitCount) {
    return;
  }

  float const clampMin{idx == 0 ? 0.0f : mCascadeSplits[idx - 1]};
  float const clampMax{idx == splitCount - 1 ? 1.0f : mCascadeSplits[idx + 1]};

  mCascadeSplits[idx] = std::clamp(split, clampMin, clampMax);
}


auto Renderer::Impl::IsVisualizingShadowCascades() const noexcept -> bool {
  return mVisualizeShadowCascades;
}


auto Renderer::Impl::VisualizeShadowCascades(bool const visualize) noexcept -> void {
  mVisualizeShadowCascades = visualize;
}


auto Renderer::Impl::GetShadowFilteringMode() const noexcept -> ShadowFilteringMode {
  return mShadowFilteringMode;
}


auto Renderer::Impl::SetShadowFilteringMode(ShadowFilteringMode const filteringMode) noexcept -> void {
  mShadowFilteringMode = filteringMode;
}


auto Renderer::Impl::GetAmbientLightColor() const noexcept -> Vector3 const& {
  return mAmbientLightColor;
}


auto Renderer::Impl::SetAmbientLightColor(Vector3 const& color) noexcept -> void {
  mAmbientLightColor = color;
}


auto Renderer::Impl::GetGamma() const noexcept -> f32 {
  return 1.f / mInvGamma;
}


auto Renderer::Impl::SetGamma(f32 const gamma) noexcept -> void {
  mInvGamma = 1.f / gamma;
}


auto Renderer::Impl::Register(StaticMeshComponent const& staticMeshComponent) noexcept -> void {
  std::unique_lock const lock{mStaticMeshMutex};
  mStaticMeshComponents.emplace_back(std::addressof(staticMeshComponent));
}


auto Renderer::Impl::Unregister(StaticMeshComponent const& staticMeshComponent) noexcept -> void {
  std::unique_lock const lock{mStaticMeshMutex};
  std::erase(mStaticMeshComponents, std::addressof(staticMeshComponent));
}


auto Renderer::Impl::Register(LightComponent const& lightComponent) noexcept -> void {
  std::unique_lock const lock{mLightMutex};
  mLights.emplace_back(std::addressof(lightComponent));
}


auto Renderer::Impl::Unregister(LightComponent const& lightComponent) noexcept -> void {
  std::unique_lock const lock{mLightMutex};
  std::erase(mLights, std::addressof(lightComponent));
}


auto Renderer::Impl::Register(SkyboxComponent const& skyboxComponent) noexcept -> void {
  std::unique_lock const lock{mSkyboxMutex};
  mSkyboxes.emplace_back(std::addressof(skyboxComponent));
}


auto Renderer::Impl::Unregister(SkyboxComponent const& skyboxComponent) noexcept -> void {
  std::unique_lock const lock{mSkyboxMutex};
  std::erase(mSkyboxes, std::addressof(skyboxComponent));
}


auto Renderer::Impl::Register(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{mGameCameraMutex};
  mGameRenderCameras.emplace_back(&cam);
}


auto Renderer::Impl::Unregister(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{mGameCameraMutex};
  std::erase(mGameRenderCameras, &cam);
}
}
