#include "RendererImpl.hpp"

#include "ShadowCascadeBoundary.hpp"
#include "../MemoryAllocation.hpp"
#include "../Window.hpp"
#include "../scene_objects/Entity.hpp"
#include "../scene_objects/TransformComponent.hpp"
#include "shaders\shader_interop.h"


#ifndef NDEBUG
#include "shaders/generated/Debug/DepthNormalPSBin.h"
#include "shaders/generated/Debug/DepthNormalVSBin.h"
#include "shaders/generated/Debug/DepthOnlyPSBin.h"
#include "shaders/generated/Debug/DepthOnlyVSBin.h"
#include "shaders/generated/Debug/DepthResolveCSBin.h"
#include "shaders/generated/Debug/GizmoPSBin.h"
#include "shaders/generated/Debug/LineGizmoVSBin.h"
#include "shaders/generated/Debug/MeshPbrPSBin.h"
#include "shaders/generated/Debug/MeshVSBin.h"
#include "shaders/generated/Debug/PostProcessPSBin.h"
#include "shaders/generated/Debug/ScreenVSBin.h"
#include "shaders/generated/Debug/SkyboxPSBin.h"
#include "shaders/generated/Debug/SkyboxVSBin.h"
#include "shaders/generated/Debug/SsaoBlurPSBin.h"
#include "shaders/generated/Debug/SsaoPSBin.h"
#else
#include "shaders/generated/Release/DepthNormalPSBin.h"
#include "shaders/generated/Release/DepthNormalVSBin.h"
#include "shaders/generated/Release/DepthOnlyPSBin.h"
#include "shaders/generated/Release/DepthOnlyVSBin.h"
#include "shaders/generated/Release/DepthResolveCSBin.h"
#include "shaders/generated/Release/GizmoPSBin.h"
#include "shaders/generated/Release/LineGizmoVSBin.h"
#include "shaders/generated/Release/MeshPbrPSBin.h"
#include "shaders/generated/Release/MeshVSBin.h"
#include "shaders/generated/Release/PostProcessPSBin.h"
#include "shaders/generated/Release/ScreenVSBin.h"
#include "shaders/generated/Release/SkyboxPSBin.h"
#include "shaders/generated/Release/SkyboxVSBin.h"
#include "shaders/generated/Release/SsaoBlurPSBin.h"
#include "shaders/generated/Release/SsaoPSBin.h"
#endif

#include "../Resources/Scene.hpp"

#include <d3d11_1.h>

#include <random>

using Microsoft::WRL::ComPtr;


namespace sorcery {
namespace {
std::vector const QUAD_POSITIONS{Vector3{-1, 1, 0}, Vector3{-1, -1, 0}, Vector3{1, -1, 0}, Vector3{1, 1, 0}};


std::vector const QUAD_UVS{Vector2{0, 0}, Vector2{0, 1}, Vector2{1, 1}, Vector2{1, 0}};


std::vector<std::uint32_t> const QUAD_INDICES{2, 1, 0, 0, 3, 2};


std::vector const CUBE_POSITIONS{
  Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f},
  Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, -0.5f}, Vector3{-0.5f, 0.5f, -0.5f},
  Vector3{-0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f},
  Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f},
  Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, -0.5f}, Vector3{-0.5f, -0.5f, -0.5f},
  Vector3{-0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f},
};


std::vector const CUBE_UVS{
  Vector2{1, 0}, Vector2{1, 0}, Vector2{0, 0}, Vector2{0, 0}, Vector2{0, 0}, Vector2{1, 0}, Vector2{1, 0},
  Vector2{0, 1}, Vector2{0, 0}, Vector2{0, 0}, Vector2{1, 1}, Vector2{1, 0}, Vector2{1, 1}, Vector2{1, 1},
  Vector2{0, 1}, Vector2{0, 1}, Vector2{0, 1}, Vector2{1, 1}, Vector2{1, 1}, Vector2{0, 0}, Vector2{0, 1},
  Vector2{0, 1}, Vector2{1, 0}, Vector2{1, 1}
};


std::vector<std::uint32_t> const CUBE_INDICES{
  // Top face
  7, 4, 1, 1, 10, 7,
  // Bottom face
  16, 19, 22, 22, 13, 16,
  // Front face
  23, 20, 8, 8, 11, 23,
  // Back face
  17, 14, 2, 2, 5, 17,
  // Right face
  21, 9, 0, 0, 12, 21,
  // Left face
  15, 3, 6, 6, 18, 15
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
    if (auto const mesh{mStaticMeshComponents[i]->GetMesh()}) {
      if (auto const& modelMtx{mStaticMeshComponents[i]->GetEntity().GetTransform().GetLocalToWorldMatrix()}; frustumWS.
        Intersects(mesh->GetBounds().Transform(modelMtx))) {
        auto const submeshes{mesh->GetSubMeshes()};
        for (int j{0}; j < std::ssize(submeshes); j++) {
          if (frustumWS.Intersects(submeshes[j].bounds.Transform(modelMtx))) {
            visibility.staticMeshIndices.emplace_back(i, j);
          }
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

            for (auto const modelMtxNoScale{
                   light->GetEntity().GetTransform().CalculateLocalToWorldMatrixWithoutScale()
                 }; auto& vertex : vertices) {
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
          .center = Vector3{light->GetEntity().GetTransform().GetWorldPosition()}, .radius = light->GetRange()
        };

        if (frustumWS.Intersects(boundsWS)) {
          visibility.lightIndices.emplace_back(lightIdx);
        }
        break;
      }
    }
  }
}


auto Renderer::Impl::SetPerFrameConstants(ObserverPtr<ID3D11DeviceContext> const ctx, int const rtWidth,
                                          int const rtHeight) const noexcept -> void {
  D3D11_MAPPED_SUBRESOURCE mapped;
  [[maybe_unused]] auto const hr{ctx->Map(mPerFrameCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)};
  assert(SUCCEEDED(hr));

  *static_cast<PerFrameCB*>(mapped.pData) = PerFrameCB{
    .gPerFrameConstants = ShaderPerFrameConstants{
      .ambientLightColor = Scene::GetActiveScene()->GetAmbientLightVector(), .shadowCascadeCount = mCascadeCount,
      .screenSize = Vector2{rtWidth, rtHeight}, .visualizeShadowCascades = mVisualizeShadowCascades,
      .shadowFilteringMode = static_cast<int>(mShadowFilteringMode)
    }
  };

  ctx->Unmap(mPerFrameCb.Get(), 0);
}


auto Renderer::Impl::SetPerViewConstants(ObserverPtr<ID3D11DeviceContext> const ctx, Matrix4 const& viewMtx,
                                         Matrix4 const& projMtx, ShadowCascadeBoundaries const& shadowCascadeBoundaries,
                                         Vector3 const& viewPos) const noexcept -> void {
  D3D11_MAPPED_SUBRESOURCE mapped;
  [[maybe_unused]] auto const hr{ctx->Map(mPerViewCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)};
  assert(SUCCEEDED(hr));

  auto const perViewCbData{static_cast<PerViewCB*>(mapped.pData)};

  perViewCbData->gPerViewConstants.viewMtx = viewMtx;
  perViewCbData->gPerViewConstants.projMtx = projMtx;
  perViewCbData->gPerViewConstants.invProjMtx = projMtx.Inverse();
  perViewCbData->gPerViewConstants.viewProjMtx = viewMtx * projMtx;
  perViewCbData->gPerViewConstants.viewPos = viewPos;

  for (int i = 0; i < MAX_CASCADE_COUNT; i++) {
    perViewCbData->gPerViewConstants.shadowCascadeSplitDistances[i] = shadowCascadeBoundaries[i].farClip;
  }

  ctx->Unmap(mPerViewCb.Get(), 0);
}


auto Renderer::Impl::SetPerDrawConstants(ObserverPtr<ID3D11DeviceContext> const ctx,
                                         Matrix4 const& modelMtx) const noexcept -> void {
  D3D11_MAPPED_SUBRESOURCE mapped;
  [[maybe_unused]] auto const hr{ctx->Map(mPerDrawCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)};
  assert(SUCCEEDED(hr));

  *static_cast<PerDrawCB*>(mapped.pData) = PerDrawCB{
    .gPerDrawConstants = ShaderPerDrawConstants{
      .modelMtx = modelMtx, .invTranspModelMtx = modelMtx.Inverse().Transpose()
    }
  };

  ctx->Unmap(mPerDrawCb.Get(), 0);
}


auto Renderer::Impl::DrawDirectionalShadowMaps(Visibility const& visibility, Camera const& cam, float rtAspect,
                                               ShadowCascadeBoundaries const& shadowCascadeBoundaries,
                                               std::array<Matrix4, MAX_CASCADE_COUNT>& shadowViewProjMatrices,
                                               ObserverPtr<ID3D11DeviceContext> const ctx) -> void {
  for (auto const lightIdx : visibility.lightIndices) {
    if (auto const light{mLights[lightIdx]}; light->GetType() == LightComponent::Type::Directional && light->
                                             IsCastingShadow()) {
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
              float const tanHalfFov{std::tan(ToRadians(cam.GetVerticalPerspectiveFov() / 2.0f))};
              float const nearExtentY{camNear * tanHalfFov};
              float const nearExtentX{nearExtentY * rtAspect};
              float const farExtentY{camFar * tanHalfFov};
              float const farExtentX{farExtentY * rtAspect};

              ret[FrustumVertex_NearTopRight] = nearWorldForward + cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() *
                                                nearExtentY;
              ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() *
                                               nearExtentY;
              ret[FrustumVertex_NearBottomLeft] =
                nearWorldForward - cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
              ret[FrustumVertex_NearBottomRight] =
                nearWorldForward + cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
              ret[FrustumVertex_FarTopRight] = farWorldForward + cam.GetRightAxis() * farExtentX + cam.GetUpAxis() *
                                               farExtentY;
              ret[FrustumVertex_FarTopLeft] = farWorldForward - cam.GetRightAxis() * farExtentX + cam.GetUpAxis() *
                                              farExtentY;
              ret[FrustumVertex_FarBottomLeft] =
                farWorldForward - cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
              ret[FrustumVertex_FarBottomRight] =
                farWorldForward + cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
              break;
            }
            case Camera::Type::Orthographic: {
              float const extentX{cam.GetVerticalOrthographicSize() / 2.0f};
              float const extentY{extentX / rtAspect};

              ret[FrustumVertex_NearTopRight] = nearWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() *
                                                extentY;
              ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() *
                                               extentY;
              ret[FrustumVertex_NearBottomLeft] =
                nearWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
              ret[FrustumVertex_NearBottomRight] =
                nearWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
              ret[FrustumVertex_FarTopRight] = farWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() *
                                               extentY;
              ret[FrustumVertex_FarTopLeft] = farWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() *
                                              extentY;
              ret[FrustumVertex_FarBottomLeft] =
                farWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
              ret[FrustumVertex_FarBottomRight] =
                farWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
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

        Matrix4 shadowViewMtx{Matrix4::LookTo(Vector3::Zero(), light->GetDirection(), Vector3::Up())};
        cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx};
        cascadeCenterWS /= worldUnitsPerTexel;
        cascadeCenterWS[0] = std::floor(cascadeCenterWS[0]);
        cascadeCenterWS[1] = std::floor(cascadeCenterWS[1]);
        cascadeCenterWS *= worldUnitsPerTexel;
        cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx.Inverse()};

        shadowViewMtx = Matrix4::LookTo(cascadeCenterWS, light->GetDirection(), Vector3::Up());
        auto const shadowProjMtx{
          GetProjectionMatrixForRendering(Matrix4::OrthographicOffCenter(-sphereRadius, sphereRadius,
            sphereRadius, -sphereRadius, -sphereRadius - light->GetShadowExtension(), sphereRadius))
        };

        shadowViewProjMatrices[cascadeIdx] = shadowViewMtx * shadowProjMtx;

        ctx->OMSetRenderTargets(0, nullptr, mDirShadowMapArr->GetDsv(cascadeIdx));
        ctx->OMSetDepthStencilState(mDepthTestGreaterWriteDss.Get(), 0);

        ctx->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);

        ctx->PSSetShader(mDepthOnlyPs.Get(), nullptr, 0);

        ctx->ClearDepthStencilView(mDirShadowMapArr->GetDsv(cascadeIdx), D3D11_CLEAR_DEPTH, 0, 0);

        ctx->RSSetState(mShadowPassRs.Get());

        ctx->IASetInputLayout(mAllAttribsIl.Get());

        D3D11_VIEWPORT const shadowViewport{
          .TopLeftX = 0, .TopLeftY = 0, .Width = static_cast<float>(shadowMapSize),
          .Height = static_cast<float>(shadowMapSize), .MinDepth = 0, .MaxDepth = 1
        };

        ctx->RSSetViewports(1, &shadowViewport);

        SetPerViewConstants(ctx, shadowViewMtx, shadowProjMtx, ShadowCascadeBoundaries{}, Vector3{});
        ctx->VSSetConstantBuffers(CB_SLOT_PER_VIEW, 1, std::array{mPerViewCb.Get()}.data());

        Frustum const shadowFrustumWS{shadowViewProjMatrices[cascadeIdx]};

        Visibility perLightVisibility{
          .lightIndices = std::pmr::vector<int>{&GetTmpMemRes()},
          .staticMeshIndices = std::pmr::vector<StaticMeshSubmeshIndex>{&GetTmpMemRes()}
        };

        CullStaticMeshComponents(shadowFrustumWS, perLightVisibility);
        DrawMeshes(perLightVisibility.staticMeshIndices, ctx);
      }

      break;
    }
  }
}


auto Renderer::Impl::DrawShadowMaps(ShadowAtlas const& atlas, ObserverPtr<ID3D11DeviceContext> const ctx) -> void {
  ctx->OMSetRenderTargets(0, nullptr, atlas.GetDsv());
  ctx->OMSetDepthStencilState(mDepthTestGreaterWriteDss.Get(), 0);

  ctx->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);

  ctx->PSSetShader(mDepthOnlyPs.Get(), nullptr, 0);

  ctx->ClearDepthStencilView(atlas.GetDsv(), D3D11_CLEAR_DEPTH, 0, 0);

  ctx->RSSetState(mShadowPassRs.Get());

  ctx->IASetInputLayout(mAllAttribsIl.Get());

  auto const cellSizeNorm{atlas.GetNormalizedElementSize()};

  for (auto i = 0; i < atlas.GetElementCount(); i++) {
    auto const& cell{atlas.GetCell(i)};
    auto const cellOffsetNorm{atlas.GetNormalizedElementOffset(i)};
    auto const subcellSize{cellSizeNorm * cell.GetNormalizedElementSize() * static_cast<float>(atlas.GetSize())};

    for (auto j = 0; j < cell.GetElementCount(); j++) {
      if (auto const& subcell{cell.GetSubcell(j)}) {
        auto const subcellOffset{
          (cellOffsetNorm + cell.GetNormalizedElementOffset(j) * cellSizeNorm) * static_cast<float>(atlas.GetSize())
        };

        D3D11_VIEWPORT const viewport{
          .TopLeftX = subcellOffset[0], .TopLeftY = subcellOffset[1], .Width = subcellSize, .Height = subcellSize,
          .MinDepth = 0, .MaxDepth = 1
        };

        ctx->RSSetViewports(1, &viewport);

        SetPerViewConstants(ctx, Matrix4::Identity(), subcell->shadowViewProjMtx, ShadowCascadeBoundaries{}, Vector3{});
        ctx->VSSetConstantBuffers(CB_SLOT_PER_VIEW, 1, std::array{mPerViewCb.Get()}.data());

        Frustum const shadowFrustumWS{subcell->shadowViewProjMtx};

        Visibility perLightVisibility{
          .lightIndices = std::pmr::vector<int>{&GetTmpMemRes()},
          .staticMeshIndices = std::pmr::vector<StaticMeshSubmeshIndex>{&GetTmpMemRes()}
        };

        CullStaticMeshComponents(shadowFrustumWS, perLightVisibility);
        DrawMeshes(perLightVisibility.staticMeshIndices, ctx);
      }
    }
  }
}


auto Renderer::Impl::DrawMeshes(std::span<StaticMeshSubmeshIndex const> const culledIndices,
                                ObserverPtr<ID3D11DeviceContext> ctx) noexcept -> void {
  ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  ctx->VSSetConstantBuffers(CB_SLOT_PER_DRAW, 1, mPerDrawCb.GetAddressOf());
  ctx->PSSetConstantBuffers(CB_SLOT_PER_DRAW, 1, mPerDrawCb.GetAddressOf());

  for (auto const& [meshIdx, submeshIdx] : culledIndices) {
    auto const component{mStaticMeshComponents[meshIdx]};
    auto const mesh{component->GetMesh()};

    if (!mesh) {
      continue;
    }

    std::array const vertexBuffers{
      mesh->GetPositionBuffer().Get(), mesh->GetNormalBuffer().Get(), mesh->GetUVBuffer().Get(),
      mesh->GetTangentBuffer().Get()
    };
    UINT constexpr strides[]{sizeof(Vector3), sizeof(Vector3), sizeof(Vector2), sizeof(Vector3)};
    UINT constexpr offsets[]{0, 0, 0, 0};
    ctx->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffers.size()), vertexBuffers.data(), strides, offsets);
    ctx->IASetIndexBuffer(mesh->GetIndexBuffer().Get(), mesh->GetIndexFormat(), 0);

    SetPerDrawConstants(ctx, component->GetEntity().GetTransform().GetLocalToWorldMatrix());

    auto const submesh{mesh->GetSubMeshes()[submeshIdx]};
    auto const mtl{component->GetMaterials()[submesh.materialIndex]};

    if (!mtl) {
      continue;
    }

    auto const mtlBuffer{mtl->GetBuffer()};
    ctx->VSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);
    ctx->PSSetConstantBuffers(CB_SLOT_PER_MATERIAL, 1, &mtlBuffer);

    auto const albedoSrv{
      [mtl] {
        auto const albedoMap{mtl->GetAlbedoMap()};
        return albedoMap ? albedoMap->GetSrv() : nullptr;
      }()
    };
    ctx->PSSetShaderResources(RES_SLOT_ALBEDO_MAP, 1, &albedoSrv);

    auto const metallicSrv{
      [mtl] {
        auto const metallicMap{mtl->GetMetallicMap()};
        return metallicMap ? metallicMap->GetSrv() : nullptr;
      }()
    };
    ctx->PSSetShaderResources(RES_SLOT_METALLIC_MAP, 1, &metallicSrv);

    auto const roughnessSrv{
      [mtl] {
        auto const roughnessMap{mtl->GetRoughnessMap()};
        return roughnessMap ? roughnessMap->GetSrv() : nullptr;
      }()
    };
    ctx->PSSetShaderResources(RES_SLOT_ROUGHNESS_MAP, 1, &roughnessSrv);

    auto const aoSrv{
      [mtl] {
        auto const aoMap{mtl->GetAoMap()};
        return aoMap ? aoMap->GetSrv() : nullptr;
      }()
    };
    ctx->PSSetShaderResources(RES_SLOT_AO_MAP, 1, &aoSrv);

    auto const normalSrv{
      [&mtl] {
        auto const normalMap{mtl->GetNormalMap()};
        return normalMap ? normalMap->GetSrv() : nullptr;
      }()
    };
    ctx->PSSetShaderResources(RES_SLOT_NORMAL_MAP, 1, &normalSrv);

    auto const opacitySrv{
      [&mtl] {
        auto const opacityMask{mtl->GetOpacityMask()};
        return opacityMask ? opacityMask->GetSrv() : nullptr;
      }()
    };
    ctx->PSSetShaderResources(RES_SLOT_OPACITY_MASK, 1, &opacitySrv);


    ctx->DrawIndexed(submesh.indexCount, submesh.firstIndex, submesh.baseVertex);
  }
}


auto Renderer::Impl::DrawSkybox(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
  ID3D11Buffer* const vertexBuffer{mCubeMesh->GetPositionBuffer().Get()};
  UINT constexpr stride{sizeof(Vector3)};
  UINT constexpr offset{0};
  ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
  ctx->IASetIndexBuffer(mCubeMesh->GetIndexBuffer().Get(), mCubeMesh->GetIndexFormat(), 0);
  ctx->IASetInputLayout(mAllAttribsIl.Get());
  ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  ctx->VSSetShader(mSkyboxVs.Get(), nullptr, 0);
  ctx->PSSetShader(mSkyboxPs.Get(), nullptr, 0);

  auto const cubemap{Scene::GetActiveScene()->GetSkybox()};
  assert(cubemap);
  auto const cubemapSrv{cubemap->GetSrv()};
  ctx->PSSetShaderResources(RES_SLOT_SKYBOX_CUBEMAP, 1, &cubemapSrv);

  ctx->OMSetDepthStencilState(mDepthTestGreaterEqualNoWriteDss.Get(), 0);
  ctx->RSSetState(mSkyboxPassRs.Get());

  ctx->DrawIndexed(clamp_cast<UINT>(CUBE_INDICES.size()), 0, 0);

  // Restore state
  ctx->OMSetDepthStencilState(nullptr, 0);
  ctx->RSSetState(nullptr);
}


auto Renderer::Impl::PostProcess(ID3D11ShaderResourceView* const src, ID3D11RenderTargetView* const dst,
                                 ObserverPtr<ID3D11DeviceContext> ctx) noexcept -> void {
  // Back up old views to restore later.

  ComPtr<ID3D11RenderTargetView> rtvBackup;
  ComPtr<ID3D11DepthStencilView> dsvBackup;
  ComPtr<ID3D11ShaderResourceView> srvBackup;
  ctx->OMGetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.GetAddressOf());
  ctx->PSGetShaderResources(0, 1, srvBackup.GetAddressOf());

  // Do the step

  D3D11_MAPPED_SUBRESOURCE mappedCb;
  ctx->Map(mPostProcessCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCb);
  auto const cbData{static_cast<PostProcessCB*>(mappedCb.pData)};
  cbData->invGamma = mInvGamma;
  ctx->Unmap(mPostProcessCb.Get(), 0);

  ctx->VSSetShader(mScreenVs.Get(), nullptr, 0);
  ctx->PSSetShader(mPostProcessPs.Get(), nullptr, 0);

  ctx->OMSetRenderTargets(1, &dst, nullptr);

  ctx->PSSetConstantBuffers(CB_SLOT_POST_PROCESS, 1, mPostProcessCb.GetAddressOf());
  ctx->PSSetShaderResources(RES_SLOT_POST_PROCESS_SRC, 1, &src);

  ctx->IASetInputLayout(nullptr);
  ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  ctx->Draw(3, 0);

  // Restore old view bindings to that we don't leave any input/output conflicts behind.

  ctx->PSSetShaderResources(RES_SLOT_POST_PROCESS_SRC, 1, srvBackup.GetAddressOf());
  ctx->OMSetRenderTargets(1, rtvBackup.GetAddressOf(), dsvBackup.Get());
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


auto Renderer::Impl::RecreateSsaoSamples(int const sampleCount) noexcept -> void {
  mSsaoSamplesBuffer->Resize(sampleCount);
  auto const ctx{GetThreadContext()};
  auto const ssaoSamples{mSsaoSamplesBuffer->Map(ctx)};

  std::uniform_real_distribution dist{0.0f, 1.0f};
  std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

  for (auto i{0}; i < sampleCount; i++) {
    Vector3 sample{dist(gen) * 2 - 1, dist(gen) * 2 - 1, dist(gen)};
    Normalize(sample);
    sample *= dist(gen);

    auto scale{static_cast<float>(i) / static_cast<float>(sampleCount)};
    scale = std::lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;

    ssaoSamples[i] = Vector4{sample, 0};
  }

  mSsaoSamplesBuffer->Unmap(ctx);
}


auto Renderer::Impl::SetDebugName(ObserverPtr<ID3D11DeviceChild> const deviceChild,
                                  std::string_view const name) noexcept -> void {
  [[maybe_unused]] auto const hr{
    deviceChild->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(name)), name.data())
  };
  assert(SUCCEEDED(hr));
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


auto Renderer::Impl::GenerateSphere(float const radius, int const latitudes, int const longitudes,
                                    std::vector<Vector3>& vertices, std::vector<Vector3>& normals,
                                    std::vector<Vector2>& uvs, std::vector<std::uint32_t>& indices) -> void {
  // Based on: https://gist.github.com/Pikachuxxxx/5c4c490a7d7679824e0e18af42918efc

  auto const deltaLatitude{PI / static_cast<float>(latitudes)};
  auto const deltaLongitude{2 * PI / static_cast<float>(longitudes)};

  for (int i = 0; i <= latitudes; i++) {
    auto const latitudeAngle{PI / 2 - static_cast<float>(i) * deltaLatitude};

    float const xz{radius * std::cos(latitudeAngle)};
    float const y{radius * std::sin(latitudeAngle)};

    /* We add (latitudes + 1) vertices per longitude because of equator,
     * the North pole and South pole are not counted here, as they overlap.
     * The first and last vertices have same position and normal, but
     * different tex coords. */
    for (int j = 0; j <= longitudes; j++) {
      auto const longitudeAngle{static_cast<float>(j) * deltaLongitude};

      auto const x{xz * std::cos(longitudeAngle)};
      auto const z{xz * std::sin(longitudeAngle)};
      auto const u{static_cast<float>(j) / static_cast<float>(longitudes)};
      auto const v{static_cast<float>(i) / static_cast<float>(latitudes)};
      vertices.emplace_back(x, y, z);
      uvs.emplace_back(u, v);
      normals.emplace_back(x / radius, y / radius, z / radius);
    }
  }


  /*  Indices
   *  k1--k1+1
   *  |  / |
   *  | /  |
   *  k2--k2+1 */
  for (int i = 0; i < latitudes; ++i) {
    unsigned int v1 = i * (longitudes + 1);
    unsigned int v2 = v1 + longitudes + 1;

    // 2 Triangles per latitude block excluding the first and last longitudes blocks
    for (int j = 0; j < longitudes; j++, v1++, v2++) {
      if (i != 0) {
        indices.push_back(v1);
        indices.push_back(v1 + 1);
        indices.push_back(v2);
      }

      if (i != latitudes - 1) {
        indices.push_back(v1 + 1);
        indices.push_back(v2 + 1);
        indices.push_back(v2);
      }
    }
  }
}


auto Renderer::Impl::GetProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4 {
  return proj_mtx * Matrix4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 1, 1};
}


auto Renderer::Impl::StartUp() -> void {
  // CREATE DEVICE AND IMMEDIATE CONTEXT

  UINT creationFlags{0};
  D3D_FEATURE_LEVEL constexpr requestedFeatureLevels[]{D3D_FEATURE_LEVEL_11_0};

#ifndef NDEBUG
  creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  if (FAILED(
    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, requestedFeatureLevels, 1,
      D3D11_SDK_VERSION, mDevice.GetAddressOf(), nullptr, mImmediateContext.GetAddressOf()))) {
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

  mLightBuffer = std::make_unique<StructuredBuffer<ShaderLight>>(mDevice);
  mGizmoColorBuffer = std::make_unique<StructuredBuffer<Vector4>>(mDevice);
  mLineGizmoVertexDataBuffer = std::make_unique<StructuredBuffer<ShaderLineGizmoVertexData>>(mDevice);
  mMainRt = std::make_unique<RenderTarget>(RenderTarget::Desc{
    .width = static_cast<UINT>(gWindow.GetClientAreaSize().width),
    .height = static_cast<UINT>(gWindow.GetClientAreaSize().height), .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
    .depthBufferBitCount = 0, .stencilBufferBitCount = 0, .debugName = "Main RT"
  });

  // CREATE INPUT LAYOUTS

  D3D11_INPUT_ELEMENT_DESC constexpr inputDescs[]{
    {
      .SemanticName = "POSITION", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 0,
      .AlignedByteOffset = 0, .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA, .InstanceDataStepRate = 0
    },
    {
      .SemanticName = "NORMAL", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 1,
      .AlignedByteOffset = 0, .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA, .InstanceDataStepRate = 0
    },
    {
      .SemanticName = "TEXCOORD", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32_FLOAT, .InputSlot = 2,
      .AlignedByteOffset = 0, .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA, .InstanceDataStepRate = 0
    },
    {
      .SemanticName = "TANGENT", .SemanticIndex = 0, .Format = DXGI_FORMAT_R32G32B32_FLOAT, .InputSlot = 3,
      .AlignedByteOffset = 0, .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA, .InstanceDataStepRate = 0
    }
  };

  if (FAILED(
    mDevice->CreateInputLayout(inputDescs, ARRAYSIZE(inputDescs), gMeshVSBin, ARRAYSIZE(gMeshVSBin), mAllAttribsIl.
      GetAddressOf()))) {
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

  if (FAILED(
    mDevice->CreatePixelShader(gPostProcessPSBin, ARRAYSIZE(gPostProcessPSBin), nullptr, mPostProcessPs.GetAddressOf()
    ))) {
    throw std::runtime_error{"Failed to create textured post process pixel shader."};
  }

  char constexpr postProcessPsName[]{"Postprocess Pixel Shader"};
  mPostProcessPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(postProcessPsName), postProcessPsName);

  if (FAILED(
    mDevice->CreateVertexShader(gSkyboxVSBin, ARRAYSIZE(gSkyboxVSBin), nullptr, mSkyboxVs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to create skybox vertex shader."};
  }

  char constexpr skyboxVsName[]{"Skybox Vertex Shader"};
  mSkyboxVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(skyboxVsName), skyboxVsName);

  if (FAILED(
    mDevice->CreatePixelShader(gSkyboxPSBin, ARRAYSIZE(gSkyboxPSBin), nullptr, mSkyboxPs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to create skybox pixel shader."};
  }

  char constexpr skyboxPsName[]{"Skybox Pixel Shader"};
  mSkyboxPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(skyboxPsName), skyboxPsName);

  if (FAILED(
    mDevice->CreateVertexShader(gDepthOnlyVSBin, ARRAYSIZE(gDepthOnlyVSBin), nullptr, mDepthOnlyVs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-only vertex shader."};
  }

  char constexpr depthOnlyVsName[]{"Depth-Only Vertex Shader"};
  mDepthOnlyVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(depthOnlyVsName), depthOnlyVsName);

  if (FAILED(
    mDevice->CreatePixelShader(gDepthOnlyPSBin, ARRAYSIZE(gDepthOnlyPSBin), nullptr, mDepthOnlyPs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-only pixel shader."};
  }

  char constexpr depthOnlyPsName[]{"Depth-Only Pixel Shader"};
  mDepthOnlyPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(depthOnlyPsName), depthOnlyPsName);

  if (FAILED(mDevice->CreateVertexShader(gScreenVSBin, ARRAYSIZE(gScreenVSBin), nullptr, mScreenVs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create screen vertex shader."};
  }

  char constexpr screenVsName[]{"Screen Vertex Shader"};
  mScreenVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(screenVsName), screenVsName);

  if (FAILED(
    mDevice->CreateVertexShader(gLineGizmoVSBin, ARRAYSIZE(gLineGizmoVSBin), nullptr, mLineGizmoVs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create line gizmo vertex shader."};
  }

  char constexpr lineGizmoVsName[]{"Line Gizmo Vertex Shader"};
  mLineGizmoVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(lineGizmoVsName), lineGizmoVsName);

  if (FAILED(mDevice->CreatePixelShader(gGizmoPSBin, ARRAYSIZE(gGizmoPSBin), nullptr, mGizmoPs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create gizmo pixel shader."};
  }

  char constexpr gizmoPsName[]{"Gizmo Pixel Shader"};
  mGizmoPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(gizmoPsName), gizmoPsName);

  [[maybe_unused]] auto hr{
    mDevice->CreatePixelShader(gDepthNormalPSBin, ARRAYSIZE(gDepthNormalPSBin), nullptr, mDepthNormalPs.GetAddressOf())
  };
  assert(SUCCEEDED(hr));
  char constexpr depthNormalPsName[]{"Depth-Normal Pixel Shader"};
  hr = mDepthNormalPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(depthNormalPsName), depthNormalPsName);
  assert(SUCCEEDED(hr));

  hr = mDevice->CreateVertexShader(gDepthNormalVSBin, ARRAYSIZE(gDepthNormalVSBin), nullptr,
    mDepthNormalVs.GetAddressOf());
  assert(SUCCEEDED(hr));
  char constexpr depthNormalVsName[]{"Depth-Normal Vertex Shader"};
  hr = mDepthNormalVs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(depthNormalVsName), depthNormalVsName);
  assert(SUCCEEDED(hr));

  hr = mDevice->CreatePixelShader(gSsaoPSBin, ARRAYSIZE(gSsaoPSBin), nullptr, mSsaoPs.GetAddressOf());
  assert(SUCCEEDED(hr));
  char constexpr ssaoPsName[]{"SSAO Pixel Shader"};
  hr = mSsaoPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(ssaoPsName), ssaoPsName);
  assert(SUCCEEDED(hr));

  hr = mDevice->CreatePixelShader(gSsaoBlurPSBin, ARRAYSIZE(gSsaoBlurPSBin), nullptr, mSsaoBlurPs.GetAddressOf());
  assert(SUCCEEDED(hr));
  char constexpr ssaoBlurPsName[]{"SSAO Blur Pixel Shader"};
  hr = mSsaoBlurPs->SetPrivateData(WKPDID_D3DDebugObjectName, ARRAYSIZE(ssaoBlurPsName), ssaoBlurPsName);
  assert(SUCCEEDED(hr));

  hr = mDevice->CreateComputeShader(gDepthResolveCSBin, ARRAYSIZE(gDepthResolveCSBin), nullptr,
    mDepthResolveCs.GetAddressOf());
  assert(SUCCEEDED(hr));
  SetDebugName(mDepthResolveCs.Get(), "Depth Resolve Compute Shader");

  // CREATE CONSTANT BUFFERS


  D3D11_BUFFER_DESC constexpr perFrameCbDesc{
    .ByteWidth = sizeof(PerFrameCB), .Usage = D3D11_USAGE_DYNAMIC, .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, .MiscFlags = 0, .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&perFrameCbDesc, nullptr, mPerFrameCb.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create per frame CB."};
  }

  D3D11_BUFFER_DESC constexpr perViewCbDesc{
    .ByteWidth = sizeof(PerViewCB), .Usage = D3D11_USAGE_DYNAMIC, .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, .MiscFlags = 0, .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&perViewCbDesc, nullptr, mPerViewCb.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create per view CB."};
  }

  D3D11_BUFFER_DESC constexpr perDrawCbDesc{
    .ByteWidth = sizeof(PerDrawCB), .Usage = D3D11_USAGE_DYNAMIC, .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, .MiscFlags = 0, .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&perDrawCbDesc, nullptr, mPerDrawCb.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create per draw CB."};
  }

  D3D11_BUFFER_DESC constexpr postProcessCbDesc{
    .ByteWidth = sizeof(PostProcessCB), .Usage = D3D11_USAGE_DYNAMIC, .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, .MiscFlags = 0, .StructureByteStride = 0
  };

  if (FAILED(mDevice->CreateBuffer(&postProcessCbDesc, nullptr, mPostProcessCb.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create post-process CB."};
  }

  D3D11_BUFFER_DESC constexpr ssaoCbDesc{
    .ByteWidth = sizeof(SsaoCB), .Usage = D3D11_USAGE_DYNAMIC, .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE, .MiscFlags = 0, .StructureByteStride = 0
  };

  hr = mDevice->CreateBuffer(&ssaoCbDesc, nullptr, mSsaoCb.GetAddressOf());
  assert(SUCCEEDED(hr));

  // CREATE RASTERIZER STATES

  D3D11_RASTERIZER_DESC constexpr skyboxPassRasterizerDesc{
    .FillMode = D3D11_FILL_SOLID, .CullMode = D3D11_CULL_NONE, .FrontCounterClockwise = FALSE, .DepthBias = 0,
    .DepthBiasClamp = 0.0f, .SlopeScaledDepthBias = 0.0f, .DepthClipEnable = TRUE, .ScissorEnable = FALSE,
    .MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE
  };

  if (FAILED(mDevice->CreateRasterizerState(&skyboxPassRasterizerDesc, mSkyboxPassRs.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to create skybox pass rasterizer state."};
  }

  D3D11_RASTERIZER_DESC constexpr shadowPassRasterizerDesc{
    .FillMode = D3D11_FILL_SOLID, .CullMode = D3D11_CULL_BACK, .FrontCounterClockwise = FALSE, .DepthBias = -1,
    .DepthBiasClamp = 0, .SlopeScaledDepthBias = -2.5, .DepthClipEnable = TRUE, .ScissorEnable = FALSE,
    .MultisampleEnable = FALSE, .AntialiasedLineEnable = FALSE
  };

  if (FAILED(mDevice->CreateRasterizerState(&shadowPassRasterizerDesc, mShadowPassRs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create shadow pass rasterizer state."};
  }

  // CREATE DEPTH STENCIL STATES


  D3D11_DEPTH_STENCIL_DESC constexpr depthTestGreaterWrite{
    .DepthEnable = TRUE, .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D11_COMPARISON_GREATER,
    .StencilEnable = FALSE, .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
    .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    .FrontFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    },
    .BackFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    }
  };

  if (FAILED(mDevice->CreateDepthStencilState(&depthTestGreaterWrite, mDepthTestGreaterWriteDss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-test greater write depth-stencil state."};
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestLessrWrite{
    .DepthEnable = TRUE, .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D11_COMPARISON_LESS,
    .StencilEnable = FALSE, .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
    .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    .FrontFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    },
    .BackFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    }
  };

  if (FAILED(mDevice->CreateDepthStencilState(&depthTestLessrWrite, mDepthTestLessWriteDss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-test less write depth-stencil state."};
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestGreaterEqualNoWriteDesc{
    .DepthEnable = TRUE, .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO, .DepthFunc = D3D11_COMPARISON_GREATER_EQUAL,
    .StencilEnable = FALSE, .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
    .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    .FrontFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    },
    .BackFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    }
  };

  if (FAILED(
    mDevice->CreateDepthStencilState(&depthTestGreaterEqualNoWriteDesc, mDepthTestGreaterEqualNoWriteDss.GetAddressOf()
    ))) {
    throw std::runtime_error{"Failed to create depth-test greater-or-equal no-write DSS."};
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestGreaterEqualWriteDesc{
    .DepthEnable = TRUE, .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D11_COMPARISON_GREATER_EQUAL,
    .StencilEnable = FALSE, .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
    .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    .FrontFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    },
    .BackFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    }
  };

  if (FAILED(
    mDevice->CreateDepthStencilState(&depthTestGreaterEqualWriteDesc, mDepthTestGreaterEqualWriteDss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-test greater-or-equal write DSS."};
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestLessEqualNoWriteDesc{
    .DepthEnable = TRUE, .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO, .DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
    .StencilEnable = FALSE, .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
    .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    .FrontFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    },
    .BackFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    }
  };

  if (FAILED(
    mDevice->CreateDepthStencilState(&depthTestLessEqualNoWriteDesc, mDepthTestLessEqualNoWriteDss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-test less-or-equal no-write DSS."};
  }

  D3D11_DEPTH_STENCIL_DESC constexpr depthTestLessEqualWriteDesc{
    .DepthEnable = TRUE, .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL, .DepthFunc = D3D11_COMPARISON_LESS_EQUAL,
    .StencilEnable = FALSE, .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
    .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
    .FrontFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    },
    .BackFace = {
      .StencilFailOp = D3D11_STENCIL_OP_KEEP, .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
      .StencilPassOp = D3D11_STENCIL_OP_KEEP, .StencilFunc = D3D11_COMPARISON_ALWAYS
    }
  };

  if (FAILED(
    mDevice->CreateDepthStencilState(&depthTestLessEqualWriteDesc, mDepthTestLessEqualWriteDss.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create depth-test less-or-equal write DSS."};
  }

  // CREATE SHADOW ATLASES

  //mDirectionalShadowAtlas = std::make_unique<DirectionalShadowAtlas>(mDevice.Get(), 4096);
  mDirShadowMapArr = std::make_unique<DirectionalShadowMapArray>(mDevice.Get(), 4096);
  mPunctualShadowAtlas = std::make_unique<PunctualShadowAtlas>(mDevice.Get(), 4096);

  // CREATE SAMPLER STATES


  D3D11_SAMPLER_DESC constexpr cmpPcfGreaterEqual{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER, .AddressW = D3D11_TEXTURE_ADDRESS_BORDER, .MipLODBias = 0,
    .MaxAnisotropy = 1, .ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL, .BorderColor = {0, 0, 0, 0}, .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPcfGreaterEqual, mCmpPcfGreaterEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create PCF greater-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr cmpPcfLessEqual{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER, .AddressW = D3D11_TEXTURE_ADDRESS_BORDER, .MipLODBias = 0,
    .MaxAnisotropy = 1, .ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL, .BorderColor = {0, 0, 0, 0}, .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPcfLessEqual, mCmpPcfLessEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create PCF less-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr cmpPointGreaterEqualDesc{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT, .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER, .AddressW = D3D11_TEXTURE_ADDRESS_BORDER, .MipLODBias = 0,
    .MaxAnisotropy = 1, .ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL, .BorderColor = {0, 0, 0, 0}, .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPointGreaterEqualDesc, mCmpPointGreaterEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create point-filter greater-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr cmpPointLessEqualDesc{
    .Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT, .AddressU = D3D11_TEXTURE_ADDRESS_BORDER,
    .AddressV = D3D11_TEXTURE_ADDRESS_BORDER, .AddressW = D3D11_TEXTURE_ADDRESS_BORDER, .MipLODBias = 0,
    .MaxAnisotropy = 1, .ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL, .BorderColor = {0, 0, 0, 0}, .MinLOD = 0,
    .MaxLOD = 0
  };

  if (FAILED(mDevice->CreateSamplerState(&cmpPointLessEqualDesc, mCmpPointLessEqualSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create point-filter less-equal comparison sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af16ClampDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, .MipLODBias = 0,
    .MaxAnisotropy = 16, .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX, .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af16ClampDesc, mAf16ClampSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create AF16 clamp sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af8ClampDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, .MipLODBias = 0,
    .MaxAnisotropy = 8, .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX, .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af8ClampDesc, mAf8ClampSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create AF8 clamp sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af4ClampDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, .MipLODBias = 0,
    .MaxAnisotropy = 4, .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX, .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&af4ClampDesc, mAf4ClampSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create AF4 clamp sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af2ClampDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, .MipLODBias = 0,
    .MaxAnisotropy = 2, .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX, .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&af2ClampDesc, mAf2ClampSs.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_SAMPLER_DESC constexpr trilinearClampDesc{
    .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, .MipLODBias = 0,
    .MaxAnisotropy = 1, .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX, .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&trilinearClampDesc, mTrilinearClampSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create trilinear clamp sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr bilinearClampDesc{
    .Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, .MipLODBias = 0,
    .MaxAnisotropy = 1, .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX, .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&bilinearClampDesc, mBilinearClampSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create bilinear clamp sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr pointClampDesc{
    .Filter = D3D11_FILTER_MIN_MAG_MIP_POINT, .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
    .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP, .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP, .MipLODBias = 0,
    .MaxAnisotropy = 1, .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f},
    .MinLOD = -FLT_MAX, .MaxLOD = FLT_MAX
  };

  if (FAILED(mDevice->CreateSamplerState(&pointClampDesc, mPointClampSs.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create point-filter clamp sampler state."};
  }

  D3D11_SAMPLER_DESC constexpr af16WrapDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_WRAP, .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP, .MipLODBias = 0, .MaxAnisotropy = 16,
    .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f}, .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&af16WrapDesc, mAf16WrapSs.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_SAMPLER_DESC constexpr af8WrapDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_WRAP, .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP, .MipLODBias = 0, .MaxAnisotropy = 8,
    .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f}, .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&af8WrapDesc, mAf8WrapSs.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_SAMPLER_DESC constexpr af4WrapDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_WRAP, .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP, .MipLODBias = 0, .MaxAnisotropy = 4,
    .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f}, .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&af4WrapDesc, mAf4WrapSs.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_SAMPLER_DESC constexpr af2WrapDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_WRAP, .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP, .MipLODBias = 0, .MaxAnisotropy = 2,
    .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f}, .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&af2WrapDesc, mAf2WrapSs.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_SAMPLER_DESC constexpr trilinearWrapDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_WRAP, .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP, .MipLODBias = 0, .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f}, .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&trilinearWrapDesc, mTrilinearWrapSs.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_SAMPLER_DESC constexpr bilinearWrapDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_WRAP, .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP, .MipLODBias = 0, .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f}, .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&bilinearWrapDesc, mBilinearWrapSs.GetAddressOf());
  assert(SUCCEEDED(hr));

  D3D11_SAMPLER_DESC constexpr pointWrapDesc{
    .Filter = D3D11_FILTER_ANISOTROPIC, .AddressU = D3D11_TEXTURE_ADDRESS_WRAP, .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
    .AddressW = D3D11_TEXTURE_ADDRESS_WRAP, .MipLODBias = 0, .MaxAnisotropy = 1,
    .ComparisonFunc = D3D11_COMPARISON_NEVER, .BorderColor = {1.0f, 1.0f, 1.0f, 1.0f}, .MinLOD = -FLT_MAX,
    .MaxLOD = FLT_MAX
  };

  hr = mDevice->CreateSamplerState(&pointWrapDesc, mPointWrapSs.GetAddressOf());
  assert(SUCCEEDED(hr));

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

  mSphereMesh = CreateAndInitialize<Mesh>();
  mSphereMesh->SetGuid(SPHERE_MESH_GUID);
  mSphereMesh->SetName("Sphere");
  std::vector<Vector3> spherePositions;
  std::vector<Vector3> sphereNormals;
  std::vector<Vector3> sphereTangents;
  std::vector<Vector2> sphereUvs;
  std::vector<std::uint32_t> sphereIndices;
  GenerateSphere(1, 50, 50, spherePositions, sphereNormals, sphereUvs, sphereIndices);
  auto const sphereIdxCount{std::size(sphereIndices)};
  CalculateTangents(spherePositions, sphereUvs, sphereIndices, sphereTangents);
  mSphereMesh->SetPositions(std::move(spherePositions));
  mSphereMesh->SetNormals(std::move(sphereNormals));
  mSphereMesh->SetUVs(std::move(sphereUvs));
  mSphereMesh->SetTangents(std::move(sphereTangents));
  mSphereMesh->SetIndices(std::move(sphereIndices));
  mSphereMesh->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  mSphereMesh->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(sphereIdxCount), 0, AABB{}}});
  if (!mSphereMesh->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default sphere mesh."};
  }
  gResourceManager.Add(mSphereMesh);

  gWindow.OnWindowSize.add_handler(this, &OnWindowSize);

  dxgiFactory2->MakeWindowAssociation(static_cast<HWND>(gWindow.GetNativeHandle()), DXGI_MWA_NO_WINDOW_CHANGES);

  // CREATE SSAO SAMPLES

  mSsaoSamplesBuffer = std::make_unique<StructuredBuffer<Vector4>>(mDevice);
  RecreateSsaoSamples(mSsaoParams.sampleCount);

  // CREATE SSAO NOISE TEXTURE

  std::vector<Vector4> ssaoNoise;
  std::uniform_real_distribution dist{0.0f, 1.0f};
  std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

  for (auto i{0}; i < SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM; i++) {
    ssaoNoise.emplace_back(dist(gen) * 2 - 1, dist(gen) * 2 - 1, 0, 0);
  }

  D3D11_TEXTURE2D_DESC constexpr ssaoNoiseTexDesc{
    .Width = SSAO_NOISE_TEX_DIM, .Height = SSAO_NOISE_TEX_DIM, .MipLevels = 1, .ArraySize = 1,
    .Format = DXGI_FORMAT_R32G32B32A32_FLOAT, .SampleDesc = {.Count = 1, .Quality = 0}, .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_SHADER_RESOURCE, .CPUAccessFlags = 0, .MiscFlags = 0
  };

  D3D11_SUBRESOURCE_DATA const ssaoNoiseTexData{
    .pSysMem = ssaoNoise.data(), .SysMemPitch = SSAO_NOISE_TEX_DIM * sizeof(decltype(ssaoNoise)::value_type)
  };

  hr = mDevice->CreateTexture2D(&ssaoNoiseTexDesc, &ssaoNoiseTexData, mSsaoNoiseTex.GetAddressOf());
  assert(SUCCEEDED(hr));
  SetDebugName(mSsaoNoiseTex.Get(), "SSAO Noise");

  D3D11_SHADER_RESOURCE_VIEW_DESC constexpr ssaoNoiseSrvDesc{
    .Format = ssaoNoiseTexDesc.Format, .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
    .Texture2D = {.MostDetailedMip = 0, .MipLevels = 1}
  };

  hr = mDevice->CreateShaderResourceView(mSsaoNoiseTex.Get(), &ssaoNoiseSrvDesc, mSsaoNoiseSrv.GetAddressOf());
  assert(SUCCEEDED(hr));
  SetDebugName(mSsaoNoiseSrv.Get(), "SSAO Noise");

  // CREATE WHITE TEXTURE

  D3D11_TEXTURE2D_DESC constexpr whiteTexDesc{
    .Width = 1, .Height = 1, .MipLevels = 1, .ArraySize = 1, .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = {.Count = 1, .Quality = 0}, .Usage = D3D11_USAGE_IMMUTABLE, .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0, .MiscFlags = 0
  };

  std::array<std::uint8_t, 4> constexpr static whiteColorData{255, 255, 255, 255};

  D3D11_SUBRESOURCE_DATA constexpr whiteTexData{
    .pSysMem = whiteColorData.data(), .SysMemPitch = sizeof(whiteColorData), .SysMemSlicePitch = 0
  };

  hr = mDevice->CreateTexture2D(&whiteTexDesc, &whiteTexData, mWhiteTex.GetAddressOf());
  assert(SUCCEEDED(hr));
  SetDebugName(mWhiteTex.Get(), "White");

  D3D11_SHADER_RESOURCE_VIEW_DESC constexpr whiteTexSrvDesc{
    .Format = whiteTexDesc.Format, .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
    .Texture2D = {.MostDetailedMip = 0, .MipLevels = 1}
  };

  hr = mDevice->CreateShaderResourceView(mWhiteTex.Get(), &whiteTexSrvDesc, mWhiteTexSrv.GetAddressOf());
  assert(SUCCEEDED(hr));
  SetDebugName(mWhiteTexSrv.Get(), "White");
}


auto Renderer::Impl::ShutDown() -> void {
  gWindow.OnWindowSize.remove_handler(this, &OnWindowSize);
}


auto Renderer::Impl::DrawCamera(Camera const& cam, RenderTarget const* const rt) -> void {
  auto const ctx{GetThreadContext()};
  ComPtr<ID3DUserDefinedAnnotation> annot;
  [[maybe_unused]] auto hr{ctx->QueryInterface(IID_PPV_ARGS(annot.GetAddressOf()))};
  assert(SUCCEEDED(hr));

  auto const rtWidth{rt ? rt->GetDesc().width : gWindow.GetClientAreaSize().width};
  auto const rtHeight{rt ? rt->GetDesc().height : gWindow.GetClientAreaSize().height};
  auto const rtAspect{static_cast<float>(rtWidth) / static_cast<float>(rtHeight)};

  RenderTarget::Desc const hdrRtDesc{
    .width = rtWidth, .height = rtHeight,
    .colorFormat = mUsePreciseColorBuffer ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R11G11B10_FLOAT,
    .depthBufferBitCount = 32, .stencilBufferBitCount = 0, .sampleCount = static_cast<int>(GetMultisamplingMode()),
    .debugName = "Camera HDR RenderTarget"
  };

  auto const& hdrRt{GetTemporaryRenderTarget(hdrRtDesc)};


  auto const clearColor{
    Scene::GetActiveScene()->GetSkyMode() == SkyMode::Color
      ? Vector4{Scene::GetActiveScene()->GetSkyColor(), 1}
      : Vector4{0, 0, 0, 1}
  };
  ctx->ClearRenderTargetView(hdrRt.GetRtv(), clearColor.GetData());


  D3D11_VIEWPORT const viewport{
    .TopLeftX = 0, .TopLeftY = 0, .Width = static_cast<FLOAT>(rtWidth), .Height = static_cast<FLOAT>(rtHeight),
    .MinDepth = 0, .MaxDepth = 1
  };

  ctx->PSSetSamplers(SAMPLER_SLOT_CMP_PCF, 1, mCmpPcfGreaterEqualSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_CMP_POINT, 1, mCmpPointGreaterEqualSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF16_CLAMP, 1, mAf16ClampSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF8_CLAMP, 1, mAf8ClampSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF4_CLAMP, 1, mAf4ClampSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF2_CLAMP, 1, mAf2ClampSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_TRI_CLAMP, 1, mTrilinearClampSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_BI_CLAMP, 1, mBilinearClampSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_POINT_CLAMP, 1, mPointClampSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF16_WRAP, 1, mAf16WrapSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF8_WRAP, 1, mAf8WrapSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF4_WRAP, 1, mAf4WrapSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF2_WRAP, 1, mAf2WrapSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_TRI_WRAP, 1, mTrilinearWrapSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_BI_WRAP, 1, mBilinearWrapSs.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_POINT_WRAP, 1, mPointWrapSs.GetAddressOf());

  SetPerFrameConstants(ctx, static_cast<int>(rtWidth), static_cast<int>(rtHeight));
  ctx->VSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, mPerFrameCb.GetAddressOf());
  ctx->PSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, mPerFrameCb.GetAddressOf());

  auto const camPos{cam.GetPosition()};
  auto const camViewMtx{cam.CalculateViewMatrix()};
  auto const camProjMtx{GetProjectionMatrixForRendering(cam.CalculateProjectionMatrix(rtAspect))};
  auto const camViewProjMtx{camViewMtx * camProjMtx};

  Frustum const camFrustWS{camViewProjMtx};
  Visibility visibility{
    .lightIndices = std::pmr::vector<int>{&GetTmpMemRes()},
    .staticMeshIndices = std::pmr::vector<StaticMeshSubmeshIndex>{&GetTmpMemRes()}
  };
  CullLights(camFrustWS, visibility);

  // Shadow pass

  annot->BeginEvent(L"Directional Light Shadows");
  ID3D11ShaderResourceView* const nullSrv{nullptr};
  ctx->PSSetShaderResources(RES_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, &nullSrv);
  ctx->PSSetShaderResources(RES_SLOT_DIR_SHADOW_ATLAS, 1, &nullSrv);
  ctx->PSSetShader(nullptr, nullptr, 0);
  ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  std::array<Matrix4, MAX_CASCADE_COUNT> shadowViewProjMatrices;
  auto const shadowCascadeBoundaries{CalculateCameraShadowCascadeBoundaries(cam)};
  DrawDirectionalShadowMaps(visibility, cam, rtAspect, shadowCascadeBoundaries, shadowViewProjMatrices, ctx);

  //mDirectionalShadowAtlas->Update(mLights, visibility, cam, shadowCascadeBoundaries, rtAspect, mCascadeCount);
  //DrawShadowMaps(*mDirectionalShadowAtlas);
  annot->EndEvent();

  annot->BeginEvent(L"Punctual Light Shadows");
  mPunctualShadowAtlas->Update(mLights, visibility, cam, camViewProjMtx, mShadowDistance);
  DrawShadowMaps(*mPunctualShadowAtlas, ctx);
  annot->EndEvent();

  CullStaticMeshComponents(camFrustWS, visibility);

  SetPerViewConstants(ctx, camViewMtx, camProjMtx, shadowCascadeBoundaries, camPos);
  ctx->VSSetConstantBuffers(CB_SLOT_PER_VIEW, 1, std::array{mPerViewCb.Get()}.data());
  ctx->PSSetConstantBuffers(CB_SLOT_PER_VIEW, 1, std::array{mPerViewCb.Get()}.data());

  ctx->ClearDepthStencilView(hdrRt.GetDsv(), D3D11_CLEAR_DEPTH, 0, 0);

  auto const& normalRt{
    GetTemporaryRenderTarget(RenderTarget::Desc{
      .width = rtWidth, .height = rtHeight, .colorFormat = DXGI_FORMAT_R8G8B8A8_SNORM, .depthBufferBitCount = 0,
      .stencilBufferBitCount = 0, .sampleCount = 1, .debugName = "Camera Normal RT"
    })
  };

  // Depth-Normal pre-pass
  if (mDepthNormalPrePassEnabled) {
    annot->BeginEvent(L"Depth-Normal Pre-Pass");

    // If MSAA is enabled we render into an MSAA RT and then resolve into normalRt
    auto const& actualNormalRt{
      [this, &normalRt]() -> auto const& {
        if (GetMultisamplingMode() == MultisamplingMode::Off) {
          return normalRt;
        }

        auto actualNormalRtDesc{normalRt.GetDesc()};
        actualNormalRtDesc.sampleCount = static_cast<int>(GetMultisamplingMode());
        return GetTemporaryRenderTarget(actualNormalRtDesc);
      }()
    };

    ctx->OMSetRenderTargets(1, std::array{actualNormalRt.GetRtv()}.data(), hdrRt.GetDsv());
    ctx->OMSetDepthStencilState(mDepthTestGreaterWriteDss.Get(), 0);

    ctx->PSSetShader(mDepthNormalPs.Get(), nullptr, 0);

    ctx->RSSetViewports(1, &viewport);
    ctx->RSSetState(nullptr);

    ctx->VSSetShader(mDepthNormalVs.Get(), nullptr, 0);

    ctx->IASetInputLayout(mAllAttribsIl.Get());

    ctx->ClearRenderTargetView(actualNormalRt.GetRtv(), std::array{0.0f, 0.0f, 0.0f, 0.0f}.data());
    DrawMeshes(visibility.staticMeshIndices, ctx);

    // If we have MSAA enabled, actualNormalRt is an MSAA texture that we have to resolve into normalRt
    if (GetMultisamplingMode() != MultisamplingMode::Off) {
      ctx->ResolveSubresource(normalRt.GetColorTexture(), 0, actualNormalRt.GetColorTexture(), 0,
        *normalRt.GetDesc().colorFormat);
    }

    annot->EndEvent();
  }

  ObserverPtr<ID3D11ShaderResourceView> ssaoTexSrv{mWhiteTexSrv.Get()};

  // SSAO pass
  if (mSsaoEnabled) {
    annot->BeginEvent(L"SSAO");

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = ctx->Map(mSsaoCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    assert(SUCCEEDED(hr));

    *static_cast<SsaoCB*>(mapped.pData) = SsaoCB{
      .gSsaoConstants = ShaderSsaoConstants{
        .radius = mSsaoParams.radius, .bias = mSsaoParams.bias, .power = mSsaoParams.power,
        .sampleCount = mSsaoParams.sampleCount
      }
    };

    ctx->Unmap(mSsaoCb.Get(), 0);

    auto const& ssaoRt{
      GetTemporaryRenderTarget(RenderTarget::Desc{
        .width = rtWidth, .height = rtHeight, .colorFormat = DXGI_FORMAT_R8_UNORM, .depthBufferBitCount = 0,
        .stencilBufferBitCount = 0, .sampleCount = 1, .debugName = "SSAO RT"
      })
    };

    // If MSAA is enabled we have to resolve the depth texture before running SSAO
    auto const ssaoDepthSrv{
      [this, &hdrRt, ctx] {
        if (GetMultisamplingMode() == MultisamplingMode::Off) {
          return hdrRt.GetDepthSrv();
        }

        auto ssaoDepthRtDesc{hdrRt.GetDesc()};
        ssaoDepthRtDesc.colorFormat = [&ssaoDepthRtDesc] {
          // NOTE: 24 bit formats are not supported for UAVs!
          assert(ssaoDepthRtDesc.depthBufferBitCount == 32 || ssaoDepthRtDesc.depthBufferBitCount == 16);

          if (ssaoDepthRtDesc.depthBufferBitCount == 32) {
            return DXGI_FORMAT_R32_FLOAT;
          }

          if (ssaoDepthRtDesc.depthBufferBitCount == 16) {
            return DXGI_FORMAT_R16_FLOAT;
          }

          return DXGI_FORMAT_UNKNOWN; // This should never be reached.
        }();
        ssaoDepthRtDesc.sampleCount = 1;
        ssaoDepthRtDesc.depthBufferBitCount = 0;
        ssaoDepthRtDesc.stencilBufferBitCount = 0;
        ssaoDepthRtDesc.enableUnorderedAccess = true;
        auto const& ssaoDepthRt{GetTemporaryRenderTarget(ssaoDepthRtDesc)};

        ctx->OMSetRenderTargets(1, std::array{static_cast<ID3D11RenderTargetView*>(nullptr)}.data(), nullptr);

        ctx->CSSetUnorderedAccessViews(UAV_SLOT_DEPTH_RESOLVE_OUTPUT, 1, std::array{ssaoDepthRt.GetColorUav()}.data(),
          nullptr);
        ctx->CSSetShaderResources(RES_SLOT_DEPTH_RESOLVE_INPUT, 1, std::array{hdrRt.GetDepthSrv()}.data());
        ctx->CSSetConstantBuffers(CB_SLOT_PER_FRAME, 1, mPerFrameCb.GetAddressOf());
        ctx->CSSetShader(mDepthResolveCs.Get(), nullptr, 0);

        ctx->Dispatch(
          static_cast<UINT>(std::ceil(static_cast<float>(ssaoDepthRtDesc.width) / DEPTH_RESOLVE_CS_THREADS_X)),
          static_cast<UINT>(std::ceil(static_cast<float>(ssaoDepthRtDesc.height) / DEPTH_RESOLVE_CS_THREADS_Y)),
          static_cast<UINT>(std::ceil(1.0f / DEPTH_RESOLVE_CS_THREADS_Z)));

        ctx->CSSetUnorderedAccessViews(UAV_SLOT_DEPTH_RESOLVE_OUTPUT, 1,
          std::array{static_cast<ID3D11UnorderedAccessView*>(nullptr)}.data(), nullptr);
        ctx->CSSetShaderResources(RES_SLOT_DEPTH_RESOLVE_INPUT, 1,
          std::array{static_cast<ID3D11ShaderResourceView*>(nullptr)}.data());

        return ssaoDepthRt.GetColorSrv();
      }()
    };

    ctx->OMSetRenderTargets(1, std::array{ssaoRt.GetRtv()}.data(), nullptr);

    ctx->PSSetShader(mSsaoPs.Get(), nullptr, 0);
    ctx->PSSetShaderResources(RES_SLOT_SSAO_DEPTH, 1, std::addressof(ssaoDepthSrv));
    ctx->PSSetShaderResources(RES_SLOT_SSAO_NORMAL, 1, std::array{normalRt.GetColorSrv()}.data());
    ctx->PSSetShaderResources(RES_SLOT_SSAO_NOISE, 1, mSsaoNoiseSrv.GetAddressOf());
    ctx->PSSetShaderResources(RES_SLOT_SSAO_SAMPLES, 1, std::array{mSsaoSamplesBuffer->GetSrv()}.data());
    ctx->PSSetConstantBuffers(CB_SLOT_SSAO, 1, mSsaoCb.GetAddressOf());

    ctx->VSSetShader(mScreenVs.Get(), nullptr, 0);

    ctx->IASetInputLayout(nullptr);

    ctx->ClearRenderTargetView(ssaoRt.GetRtv(), std::array{0.0f, 0.0f, 0.0f, 0.0f}.data());
    ctx->Draw(3, 0);

    auto const& ssaoBlurRt{
      GetTemporaryRenderTarget([&ssaoRt] {
        auto ret{ssaoRt.GetDesc()};
        ret.debugName = "SSAO Blur RT";
        return ret;
      }())
    };

    ctx->OMSetRenderTargets(1, std::array{ssaoBlurRt.GetRtv()}.data(), nullptr);

    ctx->PSSetShader(mSsaoBlurPs.Get(), nullptr, 0);
    ctx->PSSetShaderResources(RES_SLOT_SSAO_BLUR_INPUT, 1, std::array{ssaoRt.GetColorSrv()}.data());

    ctx->ClearRenderTargetView(ssaoBlurRt.GetRtv(), std::array{0.0f, 0.0f, 0.0f, 0.0f}.data());
    ctx->Draw(3, 0);

    annot->EndEvent();

    ssaoTexSrv = ssaoBlurRt.GetColorSrv();
  }

  // Full forward lighting pass

  annot->BeginEvent(L"Forward Lit Pass");

  auto const lightCount{std::ssize(visibility.lightIndices)};
  mLightBuffer->Resize(static_cast<int>(lightCount));
  auto const lightBufferData{mLightBuffer->Map(ctx)};

  for (int i = 0; i < lightCount; i++) {
    lightBufferData[i].color = mLights[visibility.lightIndices[i]]->GetColor();
    lightBufferData[i].intensity = mLights[visibility.lightIndices[i]]->GetIntensity();
    lightBufferData[i].type = static_cast<int>(mLights[visibility.lightIndices[i]]->GetType());
    lightBufferData[i].direction = mLights[visibility.lightIndices[i]]->GetDirection();
    lightBufferData[i].isCastingShadow = FALSE;
    lightBufferData[i].range = mLights[visibility.lightIndices[i]]->GetRange();
    lightBufferData[i].halfInnerAngleCos = std::cos(
      ToRadians(mLights[visibility.lightIndices[i]]->GetInnerAngle() / 2.0f));
    lightBufferData[i].halfOuterAngleCos = std::cos(
      ToRadians(mLights[visibility.lightIndices[i]]->GetOuterAngle() / 2.0f));
    lightBufferData[i].position = mLights[visibility.lightIndices[i]]->GetEntity().GetTransform().GetWorldPosition();
    lightBufferData[i].depthBias = mLights[visibility.lightIndices[i]]->GetShadowDepthBias();
    lightBufferData[i].normalBias = mLights[visibility.lightIndices[i]]->GetShadowNormalBias();

    for (auto& sample : lightBufferData[i].sampleShadowMap) {
      sample = FALSE;
    }
  }

  // Set directional light shadow constant data

  for (auto i{0}; i < lightCount; i++) {
    if (auto const light{mLights[visibility.lightIndices[i]]};
      light->GetType() == LightComponent::Type::Directional && light->IsCastingShadow()) {
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

  mLightBuffer->Unmap(ctx);

  ctx->OMSetRenderTargets(1, std::array{hdrRt.GetRtv()}.data(), hdrRt.GetDsv());
  ctx->OMSetDepthStencilState(mDepthNormalPrePassEnabled
                                ? mDepthTestGreaterEqualNoWriteDss.Get()
                                : mDepthTestGreaterEqualWriteDss.Get(), 0);

  ctx->PSSetShader(mMeshPbrPs.Get(), nullptr, 0);
  ctx->PSSetShaderResources(RES_SLOT_LIGHTS, 1, std::array{mLightBuffer->GetSrv()}.data());
  ctx->PSSetShaderResources(RES_SLOT_DIR_SHADOW_MAP_ARRAY, 1, std::array{mDirShadowMapArr->GetSrv()}.data());
  ctx->PSSetShaderResources(RES_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, std::array{mPunctualShadowAtlas->GetSrv()}.data());
  ctx->PSSetShaderResources(RES_SLOT_SSAO_TEX, 1, std::array{ssaoTexSrv}.data());

  ctx->RSSetViewports(1, &viewport);
  ctx->RSSetState(nullptr);

  ctx->VSSetShader(mMeshVs.Get(), nullptr, 0);

  ctx->IASetInputLayout(mAllAttribsIl.Get());

  DrawMeshes(visibility.staticMeshIndices, ctx);

  annot->EndEvent();

  if (auto const& activeScene{Scene::GetActiveScene()};
    activeScene->GetSkyMode() == SkyMode::Skybox && activeScene->GetSkybox()) {
    annot->BeginEvent(L"Skybox Pass");
    DrawSkybox(ctx);
    annot->EndEvent();
  }

  annot->BeginEvent(L"Post-Process Pass");
  RenderTarget const* postProcessRt;

  if (GetMultisamplingMode() == MultisamplingMode::Off) {
    postProcessRt = std::addressof(hdrRt);
  } else {
    auto resolveHdrRtDesc{hdrRtDesc};
    resolveHdrRtDesc.sampleCount = 1;
    auto const& resolveHdrRt{GetTemporaryRenderTarget(resolveHdrRtDesc)};
    ctx->ResolveSubresource(resolveHdrRt.GetColorTexture(), 0, hdrRt.GetColorTexture(), 0, *hdrRtDesc.colorFormat);
    postProcessRt = std::addressof(resolveHdrRt);
  }

  ctx->RSSetViewports(1, &viewport);
  PostProcess(postProcessRt->GetColorSrv(), rt ? rt->GetRtv() : mSwapChain->GetRtv(), ctx);

  annot->EndEvent();

  ExecuteCommandList(ctx);
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
  auto const ctx{GetThreadContext()};

  ctx->OMSetRenderTargets(1, std::array{rt ? rt->GetRtv() : mSwapChain->GetRtv()}.data(), nullptr);

  mGizmoColorBuffer->Resize(static_cast<int>(std::ssize(mGizmoColors)));
  std::ranges::copy(mGizmoColors, std::begin(mGizmoColorBuffer->Map(ctx)));
  mGizmoColorBuffer->Unmap(ctx);

  mLineGizmoVertexDataBuffer->Resize(static_cast<int>(std::ssize(mLineGizmoVertexData)));
  std::ranges::copy(mLineGizmoVertexData, std::begin(mLineGizmoVertexDataBuffer->Map(ctx)));
  mLineGizmoVertexDataBuffer->Unmap(ctx);

  ctx->PSSetShader(mGizmoPs.Get(), nullptr, 0);
  ctx->PSSetShaderResources(RES_SLOT_GIZMO_COLOR, 1, std::array{mGizmoColorBuffer->GetSrv()}.data());

  if (!mLineGizmoVertexData.empty()) {
    ctx->VSSetShader(mLineGizmoVs.Get(), nullptr, 0);
    ctx->VSSetShaderResources(RES_SLOT_LINE_GIZMO_VERTEX, 1, std::array{mLineGizmoVertexDataBuffer->GetSrv()}.data());
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    ctx->DrawInstanced(2, static_cast<UINT>(mLineGizmoVertexData.size()), 0, 0);
  }

  ComPtr<ID3D11CommandList> cmdList;
  [[maybe_unused]] auto const hr{ctx->FinishCommandList(FALSE, cmdList.GetAddressOf())};
  assert(SUCCEEDED(hr));

  ExecuteCommandList(cmdList.Get());
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


auto Renderer::Impl::ExecuteCommandList(ObserverPtr<ID3D11DeviceContext> const ctx) noexcept -> void {
  ComPtr<ID3D11CommandList> cmdList;
  [[maybe_unused]] auto const hr{ctx->FinishCommandList(FALSE, cmdList.GetAddressOf())};
  assert(SUCCEEDED(hr));
  ExecuteCommandList(cmdList.Get());
}


auto Renderer::Impl::GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget& {
  std::unique_lock const lock{mTmpRenderTargetsMutex};

  for (auto& [rt, lastUseInFrames] : mTmpRenderTargets) {
    if (rt->GetDesc() == desc && lastUseInFrames != 0 /*The RT wasn't already handed out this frame*/) {
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


auto Renderer::Impl::GetSphereMesh() const noexcept -> ObserverPtr<Mesh> {
  return mSphereMesh;
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


auto Renderer::Impl::IsDepthNormalPrePassEnabled() const noexcept -> bool {
  return mDepthNormalPrePassEnabled;
}


auto Renderer::Impl::SetDepthNormalPrePassEnabled(bool const enabled) noexcept -> void {
  mDepthNormalPrePassEnabled = enabled;

  if (!enabled && IsSsaoEnabled()) {
    SetSsaoEnabled(false);
  }
}


auto Renderer::Impl::IsUsingPreciseColorFormat() const noexcept -> bool {
  return mUsePreciseColorBuffer;
}


auto Renderer::Impl::SetUsePreciseColorFormat(bool const value) noexcept -> void {
  mUsePreciseColorBuffer = value;
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


auto Renderer::Impl::IsSsaoEnabled() const noexcept -> bool {
  return mSsaoEnabled;
}


auto Renderer::Impl::SetSsaoEnabled(bool const enabled) noexcept -> void {
  mSsaoEnabled = enabled;

  if (enabled && !IsDepthNormalPrePassEnabled()) {
    SetDepthNormalPrePassEnabled(true);
  }
}


auto Renderer::Impl::GetSsaoParams() const noexcept -> SsaoParams const& {
  return mSsaoParams;
}


auto Renderer::Impl::SetSsaoParams(SsaoParams const& ssaoParams) noexcept -> void {
  if (mSsaoParams.sampleCount != ssaoParams.sampleCount) {
    RecreateSsaoSamples(ssaoParams.sampleCount);
  }

  mSsaoParams = ssaoParams;
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


auto Renderer::Impl::Register(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{mGameCameraMutex};
  mGameRenderCameras.emplace_back(&cam);
}


auto Renderer::Impl::Unregister(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{mGameCameraMutex};
  std::erase(mGameRenderCameras, &cam);
}
}
