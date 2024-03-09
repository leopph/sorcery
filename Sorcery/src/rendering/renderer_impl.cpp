#include "renderer_impl.hpp"

#include "ShadowCascadeBoundary.hpp"
#include "shaders/shader_interop.h"
#include "../MemoryAllocation.hpp"
#include "../Window.hpp"
#include "../scene_objects/Entity.hpp"
#include "../scene_objects/TransformComponent.hpp"
#include "../Resources/Scene.hpp"

#ifndef NDEBUG
#include "shaders/generated/Debug/depth_normal_ps.h"
#include "shaders/generated/Debug/depth_normal_vs.h"
#include "shaders/generated/Debug/depth_only_ps.h"
#include "shaders/generated/Debug/depth_only_vs.h"
#include "shaders/generated/Debug/depth_resolve_cs.h"
#include "shaders/generated/Debug/gizmos_line_vs.h"
#include "shaders/generated/Debug/gizmos_ps.h"
#include "shaders/generated/Debug/object_pbr_ps.h"
#include "shaders/generated/Debug/object_pbr_vs.h"
#include "shaders/generated/Debug/post_process_ps.h"
#include "shaders/generated/Debug/post_process_vs.h"
#include "shaders/generated/Debug/skybox_ps.h"
#include "shaders/generated/Debug/skybox_vs.h"
#include "shaders/generated/Debug/ssao_blur_ps.h"
#include "shaders/generated/Debug/ssao_main_ps.h"
#include "shaders/generated/Debug/ssao_vs.h"
#else
#include "shaders/generated/Release/depth_normal_ps.h"
#include "shaders/generated/Release/depth_normal_vs.h"
#include "shaders/generated/Release/depth_only_ps.h"
#include "shaders/generated/Release/depth_only_vs.h"
#include "shaders/generated/Release/depth_resolve_cs.h"
#include "shaders/generated/Release/gizmos_line_vs.h"
#include "shaders/generated/Release/gizmos_ps.h"
#include "shaders/generated/Release/object_pbr_ps.h"
#include "shaders/generated/Release/object_pbr_vs.h"
#include "shaders/generated/Release/post_process_ps.h"
#include "shaders/generated/Release/post_process_vs.h"
#include "shaders/generated/Release/skybox_ps.h"
#include "shaders/generated/Release/skybox_vs.h"
#include "shaders/generated/Release/ssao_blur_ps.h"
#include "shaders/generated/Release/ssao_main_ps.h"
#include "shaders/generated/Release/ssao_vs.h"
#endif

#include <random>


namespace sorcery {
namespace {
std::vector const kQuadPositions{Vector3{-1, 1, 0}, Vector3{-1, -1, 0}, Vector3{1, -1, 0}, Vector3{1, 1, 0}};


std::vector const kQuadUvs{Vector2{0, 0}, Vector2{0, 1}, Vector2{1, 1}, Vector2{1, 0}};


std::vector<std::uint32_t> const kQuadIndices{2, 1, 0, 0, 3, 2};


std::vector const kCubePositions{
  Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f}, Vector3{0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f},
  Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, 0.5f}, Vector3{-0.5f, 0.5f, -0.5f}, Vector3{-0.5f, 0.5f, -0.5f},
  Vector3{-0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f}, Vector3{0.5f, 0.5f, -0.5f},
  Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f}, Vector3{0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f},
  Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, 0.5f}, Vector3{-0.5f, -0.5f, -0.5f}, Vector3{-0.5f, -0.5f, -0.5f},
  Vector3{-0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f}, Vector3{0.5f, -0.5f, -0.5f},
};


std::vector const kCubeUvs{
  Vector2{1, 0}, Vector2{1, 0}, Vector2{0, 0}, Vector2{0, 0}, Vector2{0, 0}, Vector2{1, 0}, Vector2{1, 0},
  Vector2{0, 1}, Vector2{0, 0}, Vector2{0, 0}, Vector2{1, 1}, Vector2{1, 0}, Vector2{1, 1}, Vector2{1, 1},
  Vector2{0, 1}, Vector2{0, 1}, Vector2{0, 1}, Vector2{1, 1}, Vector2{1, 1}, Vector2{0, 0}, Vector2{0, 1},
  Vector2{0, 1}, Vector2{1, 0}, Vector2{1, 1}
};


std::vector<std::uint32_t> const kCubeIndices{
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
  auto const shadowDistance{std::min(cam.GetFarClipPlane(), shadow_distance_)};
  auto const shadowedFrustumDepth{shadowDistance - camNear};

  ShadowCascadeBoundaries boundaries;

  boundaries[0].nearClip = camNear;

  for (int i = 0; i < cascade_count_ - 1; i++) {
    boundaries[i + 1].nearClip = camNear + cascade_splits_[i] * shadowedFrustumDepth;
    boundaries[i].farClip = boundaries[i + 1].nearClip * 1.005f;
  }

  boundaries[cascade_count_ - 1].farClip = shadowDistance;

  for (int i = cascade_count_; i < MAX_CASCADE_COUNT; i++) {
    boundaries[i].nearClip = std::numeric_limits<float>::infinity();
    boundaries[i].farClip = std::numeric_limits<float>::infinity();
  }

  return boundaries;
}


auto Renderer::Impl::CullStaticMeshComponents(Frustum const& frustumWS, Visibility& visibility) const -> void {
  visibility.staticMeshIndices.clear();

  for (int i = 0; i < static_cast<int>(static_mesh_components_.size()); i++) {
    if (auto const mesh{static_mesh_components_[i]->GetMesh()}) {
      if (auto const& modelMtx{static_mesh_components_[i]->GetEntity().GetTransform().GetLocalToWorldMatrix()};
        frustumWS.Intersects(mesh->GetBounds().Transform(modelMtx))) {
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

  for (int lightIdx = 0; lightIdx < static_cast<int>(lights_.size()); lightIdx++) {
    switch (auto const light{lights_[lightIdx]}; light->GetType()) {
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
      .ambientLightColor = Scene::GetActiveScene()->GetAmbientLightVector(), .shadowCascadeCount = cascade_count_,
      .screenSize = Vector2{rtWidth, rtHeight}, .visualizeShadowCascades = shadow_cascades_,
      .shadowFilteringMode = static_cast<int>(shadow_filtering_mode_)
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
    if (auto const light{lights_[lightIdx]}; light->GetType() == LightComponent::Type::Directional && light->
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

      for (auto cascadeIdx{0}; cascadeIdx < cascade_count_; cascadeIdx++) {
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

        auto const shadowMapSize{dir_shadow_map_arr_->GetSize()};
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
          GetProjectionMatrixForRendering(Matrix4::OrthographicOffCenter(-sphereRadius, sphereRadius, sphereRadius,
            -sphereRadius, -sphereRadius - light->GetShadowExtension(), sphereRadius))
        };

        shadowViewProjMatrices[cascadeIdx] = shadowViewMtx * shadowProjMtx;

        ctx->OMSetRenderTargets(0, nullptr, dir_shadow_map_arr_->GetDsv(cascadeIdx));
        ctx->OMSetDepthStencilState(mDepthTestGreaterWriteDss.Get(), 0);

        ctx->VSSetShader(mDepthOnlyVs.Get(), nullptr, 0);

        ctx->PSSetShader(mDepthOnlyPs.Get(), nullptr, 0);

        ctx->ClearDepthStencilView(dir_shadow_map_arr_->GetDsv(cascadeIdx), D3D11_CLEAR_DEPTH, 0, 0);

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
    auto const component{static_mesh_components_[meshIdx]};
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
  ID3D11Buffer* const vertexBuffer{cube_mesh_->GetPositionBuffer().Get()};
  UINT constexpr stride{sizeof(Vector3)};
  UINT constexpr offset{0};
  ctx->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
  ctx->IASetIndexBuffer(cube_mesh_->GetIndexBuffer().Get(), cube_mesh_->GetIndexFormat(), 0);
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

  ctx->DrawIndexed(clamp_cast<UINT>(kCubeIndices.size()), 0, 0);

  // Restore state
  ctx->OMSetDepthStencilState(nullptr, 0);
  ctx->RSSetState(nullptr);
}


auto Renderer::Impl::PostProcess(ID3D11ShaderResourceView * const src, ID3D11RenderTargetView * const dst,
 ObserverPtr<ID3D11DeviceContext > ctx)
noexcept
->
void {
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
  cbData->invGamma = inv_gamma_;
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
  gizmo_colors_.clear();
  line_gizmo_vertex_data_.clear();
}


auto Renderer::Impl::ReleaseTempRenderTargets() noexcept -> void {
  std::erase_if(mTmpRenderTargets, [](TempRenderTargetRecord& tmpRtRecord) {
    tmpRtRecord.age_in_frames += 1;
    return tmpRtRecord.age_in_frames >= max_tmp_rt_age_;
  });
}


auto Renderer::Impl::RecreateSsaoSamples(int const sampleCount) noexcept -> void {
  ssao_samples_buffer_->Resize(sampleCount);
  auto const ctx{GetThreadContext()};
  auto const ssaoSamples{ssao_samples_buffer_->Map(ctx)};

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

  ssao_samples_buffer_->Unmap(ctx);
}


auto Renderer::Impl::RecreatePipelines() -> bool {
  if (!device_->WaitIdle()) {
    return false;
  }

  CD3DX12_DEPTH_STENCIL_DESC const depth_stencil_desc{
    TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_GREATER, FALSE, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}
  };
  DXGI_SAMPLE_DESC const sample_desc{static_cast<UINT>(msaa_mode_), 0};
  CD3DX12_RT_FORMAT_ARRAY const render_target_format{&render_target_format_, 1};
  CD3DX12_RT_FORMAT_ARRAY const color_format{&color_buffer_format_, 1};
  CD3DX12_RT_FORMAT_ARRAY const ssao_format{&ssao_buffer_format_, 1};

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_VS vs;
    CD3DX12_PIPELINE_STATE_STREAM_VS ps;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth_stencil;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT depth_stencil_format;
    CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC sample_desc;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rt_formats;
  } depth_normal_pso_desc{
      CD3DX12_SHADER_BYTECODE{g_depth_normal_vs_bytes, ARRAYSIZE(g_depth_normal_vs_bytes)},
      CD3DX12_SHADER_BYTECODE{g_depth_normal_ps_bytes, ARRAYSIZE(g_depth_normal_ps_bytes)}, depth_stencil_desc,
      depth_format_, sample_desc, CD3DX12_RT_FORMAT_ARRAY{&normal_buffer_format_, 1}
    };

  depth_normal_pso_ = device_->CreatePipelineState({sizeof(depth_normal_pso_desc), &depth_normal_pso_desc},
    sizeof(DepthNormalDrawParams) / 4, false);

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_VS vs;
    CD3DX12_PIPELINE_STATE_STREAM_VS ps;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth_stencil;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT depth_stencil_format;
    CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC sample_desc;
  } depth_only_pso_desc{
      CD3DX12_SHADER_BYTECODE{g_depth_only_vs_bytes, ARRAYSIZE(g_depth_only_vs_bytes)},
      CD3DX12_SHADER_BYTECODE{g_depth_only_ps_bytes, ARRAYSIZE(g_depth_only_ps_bytes)}, depth_stencil_desc,
      depth_format_, sample_desc
    };

  depth_only_pso_ = device_->CreatePipelineState({sizeof(depth_only_pso_desc), &depth_only_pso_desc},
    sizeof(DepthOnlyDrawParams) / 4, false);

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_CS cs;
  } depth_resolve_pso_desc{CD3DX12_SHADER_BYTECODE{g_depth_resolve_cs_bytes, ARRAYSIZE(g_depth_resolve_cs_bytes)}};

  depth_resolve_pso_ = device_->CreatePipelineState({sizeof(depth_resolve_pso_desc), &depth_resolve_pso_desc},
    sizeof(DepthResolveDrawParams) / 4, true);

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_VS vs;
    CD3DX12_PIPELINE_STATE_STREAM_VS ps;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rt_formats;
  } line_gizmo_pso_desc{
      CD3DX12_SHADER_BYTECODE{g_gizmos_line_vs_bytes, ARRAYSIZE(g_gizmos_line_vs_bytes)},
      CD3DX12_SHADER_BYTECODE{g_gizmos_ps_bytes, ARRAYSIZE(g_gizmos_ps_bytes)}, render_target_format
    };

  line_gizmo_pso_ = device_->CreatePipelineState({sizeof(line_gizmo_pso_desc), &line_gizmo_pso_desc},
    sizeof(GizmoDrawParams) / 4, false);

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_VS vs;
    CD3DX12_PIPELINE_STATE_STREAM_VS ps;
    CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC sample_desc;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rt_formats;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT depth_stencil_format;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth_stencil;
  } object_pso_desc{
      CD3DX12_SHADER_BYTECODE{g_object_pbr_vs_bytes, ARRAYSIZE(g_object_pbr_vs_bytes)},
      CD3DX12_SHADER_BYTECODE{g_object_pbr_ps_bytes, ARRAYSIZE(g_object_pbr_ps_bytes)}, sample_desc, color_format,
      depth_format_, depth_stencil_desc
    };

  object_pso_ = device_->CreatePipelineState({sizeof(object_pso_desc), &object_pso_desc}, sizeof(ObjectDrawParams) / 4,
    false);

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_VS vs;
    CD3DX12_PIPELINE_STATE_STREAM_VS ps;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtv_formats;
  } post_process_pso_desc{
      CD3DX12_SHADER_BYTECODE{&g_post_process_vs_bytes, ARRAYSIZE(g_post_process_vs_bytes)},
      CD3DX12_SHADER_BYTECODE{&g_post_process_ps_bytes, ARRAYSIZE(g_post_process_ps_bytes)}, render_target_format
    };

  post_process_pso_ = device_->CreatePipelineState({sizeof(post_process_pso_desc), &post_process_pso_desc},
    sizeof(PostProcessDrawParams) / 4, false);

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_VS vs;
    CD3DX12_PIPELINE_STATE_STREAM_VS ps;
    CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC sample_desc;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rt_formats;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT depth_stencil_format;
    CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL depth_stencil;
    CD3DX12_RASTERIZER_DESC rasterizer;
  } skybox_pso_desc{
      CD3DX12_SHADER_BYTECODE{&g_skybox_vs_bytes, ARRAYSIZE(g_skybox_vs_bytes)},
      CD3DX12_SHADER_BYTECODE{&g_skybox_ps_bytes, ARRAYSIZE(g_skybox_ps_bytes)}, sample_desc, color_format,
      depth_format_,
      CD3DX12_DEPTH_STENCIL_DESC{
        TRUE, D3D12_DEPTH_WRITE_MASK_ZERO, D3D12_COMPARISON_FUNC_GREATER_EQUAL, FALSE, {}, {}, {}, {}, {}, {}, {}, {},
        {}, {}
      },
      CD3DX12_RASTERIZER_DESC{
        D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_FRONT, FALSE, D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
        D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, TRUE, FALSE, 0, {}
      }
    };

  skybox_pso_ = device_->CreatePipelineState({sizeof(skybox_pso_desc), &skybox_pso_desc}, sizeof(SkyboxDrawParams) / 4,
    false);

  struct {
    CD3DX12_PIPELINE_STATE_STREAM_VS vs;
    CD3DX12_PIPELINE_STATE_STREAM_VS ps;
    CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rt_formats;
  } ssao_pso_desc{
      CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
      CD3DX12_SHADER_BYTECODE{&g_ssao_main_ps_bytes, ARRAYSIZE(g_ssao_main_ps_bytes)}, ssao_format
    };

  ssao_pso_ = device_->CreatePipelineState({sizeof(ssao_pso_desc), &ssao_pso_desc}, sizeof(SsaoDrawParams) / 4, false);

  decltype(ssao_pso_desc) ssao_blur_pso_desc{
    CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
    CD3DX12_SHADER_BYTECODE{&g_ssao_blur_ps_bytes, ARRAYSIZE(g_ssao_blur_ps_bytes)}, ssao_format
  };

  ssao_blur_pso_ = device_->CreatePipelineState({sizeof(ssao_blur_pso_desc), &ssao_blur_pso_desc},
    sizeof(SsaoBlurDrawParams) / 4, false);

  return true;
}


auto Renderer::Impl::CreatePerViewConstantBuffers(UINT const count) -> void {
  for (auto& cbs : per_view_cbs_) {
    cbs.reserve(cbs.size() + count);

    for (std::size_t i{0}; i < count; i++) {
      if (auto opt{ConstantBuffer<ShaderPerViewConstants>::New(*device_)}) {
        cbs.emplace_back(std::move(*opt));
      }
    }
  }
}


auto Renderer::Impl::CreatePerDrawConstantBuffers(UINT const count) -> void {
  for (auto& cbs : per_draw_cbs_) {
    cbs.reserve(cbs.size() + count);

    for (std::size_t i{0}; i < count; i++) {
      if (auto opt{ConstantBuffer<ShaderPerDrawConstants>::New(*device_)}) {
        cbs.emplace_back(std::move(*opt));
      }
    }
  }
}


auto Renderer::Impl::OnWindowSize(Impl* const self, Extent2D<std::uint32_t> const size) -> void {
  std::ignore = self->device_->SwapChainResize(*self->swap_chain_, size.width, size.height);

  if (size.width != 0 && size.height != 0) {
    RenderTarget::Desc desc{self->main_rt_->GetDesc()};
    desc.width = size.width;
    desc.height = size.height;
    self->main_rt_ = RenderTarget::New(*self->device_, desc);
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
  bool device_debug{false};

#ifndef NDEBUG
  device_debug = true;
#endif

  device_ = graphics::GraphicsDevice::New(device_debug);
  swap_chain_ = device_->CreateSwapChain(graphics::SwapChainDesc{0, 0, 2, render_target_format_, 0, DXGI_SCALING_NONE},
    static_cast<HWND>(gWindow.GetNativeHandle()));

  light_buffer_ = std::make_unique<StructuredBuffer<ShaderLight>>(device_.get());

  gizmo_color_buffer_ = std::make_unique<StructuredBuffer<Vector4>>(device_.get());

  line_gizmo_vertex_data_buffer_ = std::make_unique<StructuredBuffer<ShaderLineGizmoVertexData>>(device_.get());

  main_rt_ = RenderTarget::New(*device_, RenderTarget::Desc{
    static_cast<UINT>(gWindow.GetClientAreaSize().width), static_cast<UINT>(gWindow.GetClientAreaSize().height),
    DXGI_FORMAT_R8G8B8A8_UNORM, std::nullopt, 1, L"Main RT", false
  });

  dir_shadow_map_arr_ = std::make_unique<DirectionalShadowMapArray>(device_.get(), depth_format_, 4096);
  punctual_shadow_atlas_ = std::make_unique<PunctualShadowAtlas>(device_.get(), depth_format_, 4096);

  std::ignore = RecreatePipelines();

  for (auto& cb : per_frame_cbs_) {
    if (auto opt{ConstantBuffer<ShaderPerFrameConstants>::New(*device_)}) {
      cb = std::move(*opt);
    }
  }

  CreatePerViewConstantBuffers(1);
  CreatePerDrawConstantBuffers(100);

  samp_cmp_pcf_ge_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_GREATER_EQUAL, {},
    0, 0
  });

  samp_cmp_pcf_le_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_LESS_EQUAL, {}, 0, 0
  });

  samp_cmp_point_ge_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_GREATER_EQUAL, {}, 0, 0
  });

  samp_cmp_point_le_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_LESS_EQUAL, {}, 0, 0
  });

  samp_af16_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 16, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af8_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 8, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af4_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 4, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af2_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 2, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_tri_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_bi_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0,
    std::numeric_limits<float>::max()
  });

  samp_af16_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 16, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af8_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 8, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af4_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 4, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af2_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 2, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_tri_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 1, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  samp_bi_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 1, D3D12_COMPARISON_FUNC_ALWAYS, {}, 0, std::numeric_limits<float>::max()
  });

  // CREATE DEFAULT ASSETS

  default_material_ = CreateAndInitialize<Material>();
  default_material_->SetGuid(default_material_guid_);
  default_material_->SetName("Default Material");
  gResourceManager.Add(default_material_);

  std::vector<Vector3> cubeNormals;
  CalculateNormals(kCubePositions, kCubeIndices, cubeNormals);

  std::vector<Vector3> cubeTangents;
  CalculateTangents(kCubePositions, kCubeUvs, kCubeIndices, cubeTangents);

  std::vector<Vector3> quadNormals;
  CalculateNormals(kQuadPositions, kQuadIndices, quadNormals);

  std::vector<Vector3> quadTangents;
  CalculateTangents(kQuadPositions, kQuadUvs, kQuadIndices, quadTangents);

  cube_mesh_ = CreateAndInitialize<Mesh>();
  cube_mesh_->SetGuid(cube_mesh_guid_);
  cube_mesh_->SetName("Cube");
  cube_mesh_->SetPositions(kCubePositions);
  cube_mesh_->SetNormals(std::move(cubeNormals));
  cube_mesh_->SetUVs(kCubeUvs);
  cube_mesh_->SetTangents(std::move(cubeTangents));
  cube_mesh_->SetIndices(kCubeIndices);
  cube_mesh_->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  cube_mesh_->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(kCubeIndices.size()), 0, AABB{}}});
  if (!cube_mesh_->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default cube mesh."};
  }
  gResourceManager.Add(cube_mesh_);

  plane_mesh_ = CreateAndInitialize<Mesh>();
  plane_mesh_->SetGuid(plane_mesh_guid_);
  plane_mesh_->SetName("Plane");
  plane_mesh_->SetPositions(kQuadPositions);
  plane_mesh_->SetNormals(std::move(quadNormals));
  plane_mesh_->SetUVs(kQuadUvs);
  plane_mesh_->SetTangents(std::move(quadTangents));
  plane_mesh_->SetIndices(kQuadIndices);
  plane_mesh_->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  plane_mesh_->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(kQuadIndices.size()), 0, AABB{}}});
  if (!plane_mesh_->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default plane mesh."};
  }
  gResourceManager.Add(plane_mesh_);

  sphere_mesh_ = CreateAndInitialize<Mesh>();
  sphere_mesh_->SetGuid(sphere_mesh_guid_);
  sphere_mesh_->SetName("Sphere");
  std::vector<Vector3> spherePositions;
  std::vector<Vector3> sphereNormals;
  std::vector<Vector3> sphereTangents;
  std::vector<Vector2> sphereUvs;
  std::vector<std::uint32_t> sphereIndices;
  GenerateSphere(1, 50, 50, spherePositions, sphereNormals, sphereUvs, sphereIndices);
  auto const sphereIdxCount{std::size(sphereIndices)};
  CalculateTangents(spherePositions, sphereUvs, sphereIndices, sphereTangents);
  sphere_mesh_->SetPositions(std::move(spherePositions));
  sphere_mesh_->SetNormals(std::move(sphereNormals));
  sphere_mesh_->SetUVs(std::move(sphereUvs));
  sphere_mesh_->SetTangents(std::move(sphereTangents));
  sphere_mesh_->SetIndices(std::move(sphereIndices));
  sphere_mesh_->SetMaterialSlots(std::array{Mesh::MaterialSlotInfo{"Material"}});
  sphere_mesh_->SetSubMeshes(std::array{Mesh::SubMeshInfo{0, 0, static_cast<int>(sphereIdxCount), 0, AABB{}}});
  if (!sphere_mesh_->ValidateAndUpdate(false)) {
    throw std::runtime_error{"Failed to validate and update default sphere mesh."};
  }
  gResourceManager.Add(sphere_mesh_);

  gWindow.OnWindowSize.add_handler(this, &OnWindowSize);

  ssao_samples_buffer_ = std::make_unique<StructuredBuffer<Vector4>>(device_.get());
  RecreateSsaoSamples(ssao_params_.sampleCount);

  std::vector<Vector4> ssao_noise;
  std::uniform_real_distribution dist{0.0f, 1.0f};
  std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

  for (auto i{0}; i < SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM; i++) {
    ssao_noise.emplace_back(dist(gen) * 2 - 1, dist(gen) * 2 - 1, 0, 0);
  }

  auto const ssao_noise_upload_buf{
    device_->CreateBuffer(graphics::BufferDesc{
      static_cast<UINT>(ssao_noise.size() * sizeof(Vector4)), 0, false, false, false
    }, D3D12_HEAP_TYPE_UPLOAD)
  };
  std::memcpy(ssao_noise_upload_buf->Map(), ssao_noise.data(), ssao_noise.size() * sizeof(Vector4));

  auto constexpr ssao_noise_tex_format{DXGI_FORMAT_R32G32B32A32_FLOAT};

  ssao_noise_tex_ = device_->CreateTexture(graphics::TextureDesc{
    graphics::TextureDimension::k2D, SSAO_NOISE_TEX_DIM, SSAO_NOISE_TEX_DIM, 1, 1, ssao_noise_tex_format, {1, 0},
    D3D12_RESOURCE_FLAG_NONE, false, false, true, false
  }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr);
  std::ignore = ssao_noise_tex_->SetDebugName(L"SSAO Noise");

  std::array<std::uint8_t, 4> constexpr static white_color_data{255, 255, 255, 255};

  auto const white_tex_upload_buf{
    device_->CreateBuffer(graphics::BufferDesc{white_color_data.size()}, D3D12_HEAP_TYPE_UPLOAD)
  };
  std::memcpy(white_tex_upload_buf->Map(), white_color_data.data(), white_color_data.size());

  auto constexpr white_tex_format{DXGI_FORMAT_R8G8B8A8_UNORM};

  white_tex_ = device_->CreateTexture(graphics::TextureDesc{
    graphics::TextureDimension::k2D, 1, 1, 1, 1, white_tex_format, {1, 0}, D3D12_RESOURCE_FLAG_NONE, false, false, true,
    false
  }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_COPY_DEST, nullptr);

  std::ignore = command_lists_[0]->Begin(nullptr);
  command_lists_[0]->CopyTextureRegion(*ssao_noise_tex_, 0, 0, 0, 0, *ssao_noise_upload_buf,
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT{
      0,
      D3D12_SUBRESOURCE_FOOTPRINT{
        ssao_noise_tex_format, SSAO_NOISE_TEX_DIM, SSAO_NOISE_TEX_DIM, 1,
        RoundToNextMultiple(SSAO_NOISE_TEX_DIM * 16, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT)
      }
    });
  command_lists_[0]->CopyTextureRegion(*white_tex_, 0, 0, 0, 0, *white_tex_upload_buf.Get(),
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT{
      0, D3D12_SUBRESOURCE_FOOTPRINT{white_tex_format, 1, 1, 1, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT}
    });
  command_lists_[0]->Barrier({}, {}, std::array{
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_NO_ACCESS,
      D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE, ssao_noise_tex_.Get(),
      D3D12_BARRIER_SUBRESOURCE_RANGE{0, 1, 0, 1, 0, 1}, D3D12_TEXTURE_BARRIER_FLAG_NONE
    },
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_NO_ACCESS,
      D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE, white_tex_.Get(),
      D3D12_BARRIER_SUBRESOURCE_RANGE{0, 1, 0, 1, 0, 1}, D3D12_TEXTURE_BARRIER_FLAG_NONE
    }
  });

  std::ignore = command_lists_[0]->End();
  device_->ExecuteCommandLists(std::span{command_lists_[0].Get(), 1});
  std::ignore = device_->WaitIdle();
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
    .width = rtWidth, .height = rtHeight, .color_format = color_buffer_format_, .depth_buffer_bit_count = 32,
    .stencil_buffer_bit_count = 0, .sample_count = static_cast<int>(GetMultisamplingMode()),
    .debug_name = "Camera HDR RenderTarget"
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

  ctx->PSSetSamplers(SAMPLER_SLOT_CMP_PCF, 1, samp_cmp_pcf_ge_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_CMP_POINT, 1, samp_cmp_point_ge_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF16_CLAMP, 1, samp_af16_clamp_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF8_CLAMP, 1, samp_af8_clamp_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF4_CLAMP, 1, samp_af4_clamp_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF2_CLAMP, 1, samp_af2_clamp_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_TRI_CLAMP, 1, samp_tri_clamp_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_BI_CLAMP, 1, samp_bi_clamp_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_POINT_CLAMP, 1, samp_point_clamp_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF16_WRAP, 1, samp_af16_wrap_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF8_WRAP, 1, samp_af8_wrap_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF4_WRAP, 1, samp_af4_wrap_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_AF2_WRAP, 1, samp_af2_wrap_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_TRI_WRAP, 1, samp_tri_wrap_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_BI_WRAP, 1, samp_bi_wrap_.GetAddressOf());
  ctx->PSSetSamplers(SAMPLER_SLOT_POINT_WRAP, 1, samp_point_wrap_.GetAddressOf());

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
  punctual_shadow_atlas_->Update(lights_, visibility, cam, camViewProjMtx, shadow_distance_);
  DrawShadowMaps(*punctual_shadow_atlas_, ctx);
  annot->EndEvent();

  CullStaticMeshComponents(camFrustWS, visibility);

  SetPerViewConstants(ctx, camViewMtx, camProjMtx, shadowCascadeBoundaries, camPos);
  ctx->VSSetConstantBuffers(CB_SLOT_PER_VIEW, 1, std::array{mPerViewCb.Get()}.data());
  ctx->PSSetConstantBuffers(CB_SLOT_PER_VIEW, 1, std::array{mPerViewCb.Get()}.data());

  ctx->ClearDepthStencilView(hdrRt.GetDsv(), D3D11_CLEAR_DEPTH, 0, 0);

  auto const& normalRt{
    GetTemporaryRenderTarget(RenderTarget::Desc{
      .width = rtWidth, .height = rtHeight, .color_format = normal_buffer_format_, .depth_buffer_bit_count = 0,
      .stencil_buffer_bit_count = 0, .sample_count = 1, .debug_name = "Camera Normal RT"
    })
  };

  // Depth-Normal pre-pass
  if (depth_normal_pre_pass_enabled_) {
    annot->BeginEvent(L"Depth-Normal Pre-Pass");

    // If MSAA is enabled we render into an MSAA RT and then resolve into normalRt
    auto const& actualNormalRt{
      [this, &normalRt]() -> auto const& {
        if (GetMultisamplingMode() == MultisamplingMode::Off) {
          return normalRt;
        }

        auto actualNormalRtDesc{normalRt.GetDesc()};
        actualNormalRtDesc.sample_count = static_cast<int>(GetMultisamplingMode());
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
        *normalRt.GetDesc().color_format);
    }

    annot->EndEvent();
  }

  ObserverPtr<ID3D11ShaderResourceView> ssaoTexSrv{mWhiteTexSrv.Get()};

  // SSAO pass
  if (ssao_enabled_) {
    annot->BeginEvent(L"SSAO");

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = ctx->Map(mSsaoCb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    assert(SUCCEEDED(hr));

    *static_cast<SsaoCB*>(mapped.pData) = SsaoCB{
      .gSsaoConstants = ShaderSsaoConstants{
        .radius = ssao_params_.radius, .bias = ssao_params_.bias, .power = ssao_params_.power,
        .sampleCount = ssao_params_.sampleCount
      }
    };

    ctx->Unmap(mSsaoCb.Get(), 0);

    auto const& ssaoRt{
      GetTemporaryRenderTarget(RenderTarget::Desc{
        .width = rtWidth, .height = rtHeight, .color_format = ssao_buffer_format_, .depth_buffer_bit_count = 0,
        .stencil_buffer_bit_count = 0, .sample_count = 1, .debug_name = "SSAO RT"
      })
    };

    // If MSAA is enabled we have to resolve the depth texture before running SSAO
    auto const ssaoDepthSrv{
      [this, &hdrRt, ctx] {
        if (GetMultisamplingMode() == MultisamplingMode::Off) {
          return hdrRt.GetDepthSrv();
        }

        auto ssaoDepthRtDesc{hdrRt.GetDesc()};
        ssaoDepthRtDesc.color_format = [&ssaoDepthRtDesc] {
          // NOTE: 24 bit formats are not supported for UAVs!
          assert(ssaoDepthRtDesc.depth_buffer_bit_count == 32 || ssaoDepthRtDesc.depth_buffer_bit_count == 16);

          if (ssaoDepthRtDesc.depth_buffer_bit_count == 32) {
            return DXGI_FORMAT_R32_FLOAT;
          }

          if (ssaoDepthRtDesc.depth_buffer_bit_count == 16) {
            return DXGI_FORMAT_R16_FLOAT;
          }

          return DXGI_FORMAT_UNKNOWN; // This should never be reached.
        }();
        ssaoDepthRtDesc.sample_count = 1;
        ssaoDepthRtDesc.depth_buffer_bit_count = 0;
        ssaoDepthRtDesc.stencil_buffer_bit_count = 0;
        ssaoDepthRtDesc.enable_unordered_access = true;
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
    ctx->PSSetShaderResources(RES_SLOT_SSAO_SAMPLES, 1, std::array{ssao_samples_buffer_->GetBuffer()}.data());
    ctx->PSSetConstantBuffers(CB_SLOT_SSAO, 1, mSsaoCb.GetAddressOf());

    ctx->VSSetShader(mScreenVs.Get(), nullptr, 0);

    ctx->IASetInputLayout(nullptr);

    ctx->ClearRenderTargetView(ssaoRt.GetRtv(), std::array{0.0f, 0.0f, 0.0f, 0.0f}.data());
    ctx->Draw(3, 0);

    auto const& ssaoBlurRt{
      GetTemporaryRenderTarget([&ssaoRt] {
        auto ret{ssaoRt.GetDesc()};
        ret.debug_name = "SSAO Blur RT";
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
  light_buffer_->Resize(static_cast<int>(lightCount));
  auto const lightBufferData{light_buffer_->Map(ctx)};

  for (int i = 0; i < lightCount; i++) {
    lightBufferData[i].color = lights_[visibility.lightIndices[i]]->GetColor();
    lightBufferData[i].intensity = lights_[visibility.lightIndices[i]]->GetIntensity();
    lightBufferData[i].type = static_cast<int>(lights_[visibility.lightIndices[i]]->GetType());
    lightBufferData[i].direction = lights_[visibility.lightIndices[i]]->GetDirection();
    lightBufferData[i].isCastingShadow = FALSE;
    lightBufferData[i].range = lights_[visibility.lightIndices[i]]->GetRange();
    lightBufferData[i].halfInnerAngleCos = std::cos(
      ToRadians(lights_[visibility.lightIndices[i]]->GetInnerAngle() / 2.0f));
    lightBufferData[i].halfOuterAngleCos = std::cos(
      ToRadians(lights_[visibility.lightIndices[i]]->GetOuterAngle() / 2.0f));
    lightBufferData[i].position = lights_[visibility.lightIndices[i]]->GetEntity().GetTransform().GetWorldPosition();
    lightBufferData[i].depthBias = lights_[visibility.lightIndices[i]]->GetShadowDepthBias();
    lightBufferData[i].normalBias = lights_[visibility.lightIndices[i]]->GetShadowNormalBias();

    for (auto& sample : lightBufferData[i].sampleShadowMap) {
      sample = FALSE;
    }
  }

  // Set directional light shadow constant data

  for (auto i{0}; i < lightCount; i++) {
    if (auto const light{lights_[visibility.lightIndices[i]]};
      light->GetType() == LightComponent::Type::Directional && light->IsCastingShadow()) {
      lightBufferData[i].isCastingShadow = TRUE;

      for (auto cascadeIdx{0}; cascadeIdx < cascade_count_; cascadeIdx++) {
        lightBufferData[i].sampleShadowMap[cascadeIdx] = TRUE;
        lightBufferData[i].shadowViewProjMatrices[cascadeIdx] = shadowViewProjMatrices[cascadeIdx];
      }

      break;
    }
  }

  //mDirectionalShadowAtlas->SetLookUpInfo(lightBufferData);
  punctual_shadow_atlas_->SetLookUpInfo(lightBufferData);

  light_buffer_->Unmap(ctx);

  ctx->OMSetRenderTargets(1, std::array{hdrRt.GetRtv()}.data(), hdrRt.GetDsv());
  ctx->OMSetDepthStencilState(depth_normal_pre_pass_enabled_
                                ? mDepthTestGreaterEqualNoWriteDss.Get()
                                : mDepthTestGreaterEqualWriteDss.Get(), 0);

  ctx->PSSetShader(mMeshPbrPs.Get(), nullptr, 0);
  ctx->PSSetShaderResources(RES_SLOT_LIGHTS, 1, std::array{light_buffer_->GetBuffer()}.data());
  ctx->PSSetShaderResources(RES_SLOT_DIR_SHADOW_MAP_ARRAY, 1, std::array{dir_shadow_map_arr_->GetSrv()}.data());
  ctx->PSSetShaderResources(RES_SLOT_PUNCTUAL_SHADOW_ATLAS, 1, std::array{punctual_shadow_atlas_->GetSrv()}.data());
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
    resolveHdrRtDesc.sample_count = 1;
    auto const& resolveHdrRt{GetTemporaryRenderTarget(resolveHdrRtDesc)};
    ctx->ResolveSubresource(resolveHdrRt.GetColorTexture(), 0, hdrRt.GetColorTexture(), 0, *hdrRtDesc.color_format);
    postProcessRt = std::addressof(resolveHdrRt);
  }

  ctx->RSSetViewports(1, &viewport);
  PostProcess(postProcessRt->GetColorSrv(), rt ? rt->GetRtv() : mSwapChain->GetRtv(), ctx);

  annot->EndEvent();

  ExecuteCommandList(ctx);
}


auto Renderer::Impl::DrawAllCameras(RenderTarget const* const rt) -> void {
  for (auto const& cam : game_render_cameras_) {
    DrawCamera(*cam, rt);
  }
}


auto Renderer::Impl::DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void {
  gizmo_colors_.emplace_back(color);
  line_gizmo_vertex_data_.emplace_back(from, static_cast<std::uint32_t>(gizmo_colors_.size() - 1), to, 0.0f);
}


auto Renderer::Impl::DrawGizmos(RenderTarget const* const rt) -> void {
  auto const ctx{GetThreadContext()};

  ctx->OMSetRenderTargets(1, std::array{rt ? rt->GetRtv() : mSwapChain->GetRtv()}.data(), nullptr);

  gizmo_color_buffer_->Resize(static_cast<int>(std::ssize(gizmo_colors_)));
  std::ranges::copy(gizmo_colors_, std::begin(gizmo_color_buffer_->Map(ctx)));
  gizmo_color_buffer_->Unmap(ctx);

  line_gizmo_vertex_data_buffer_->Resize(static_cast<int>(std::ssize(line_gizmo_vertex_data_)));
  std::ranges::copy(line_gizmo_vertex_data_, std::begin(line_gizmo_vertex_data_buffer_->Map(ctx)));
  line_gizmo_vertex_data_buffer_->Unmap(ctx);

  ctx->PSSetShader(mGizmoPs.Get(), nullptr, 0);
  ctx->PSSetShaderResources(RES_SLOT_GIZMO_COLOR, 1, std::array{gizmo_color_buffer_->GetBuffer()}.data());

  if (!line_gizmo_vertex_data_.empty()) {
    ctx->VSSetShader(mLineGizmoVs.Get(), nullptr, 0);
    ctx->VSSetShaderResources(RES_SLOT_LINE_GIZMO_VERTEX, 1,
      std::array{line_gizmo_vertex_data_buffer_->GetBuffer()}.data());
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    ctx->DrawInstanced(2, static_cast<UINT>(line_gizmo_vertex_data_.size()), 0, 0);
  }

  ComPtr<ID3D11CommandList> cmdList;
  [[maybe_unused]] auto const hr{ctx->FinishCommandList(FALSE, cmdList.GetAddressOf())};
  assert(SUCCEEDED(hr));

  ExecuteCommandList(cmdList.Get());
}


auto Renderer::Impl::ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
  auto const rtv{main_rt_->GetRtv()};
  FLOAT constexpr clearColor[]{0, 0, 0, 1};
  ctx->ClearRenderTargetView(rtv, clearColor);
  ctx->OMSetRenderTargets(1, &rtv, nullptr);
}


auto Renderer::Impl::BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
  ComPtr<ID3D11Resource> mainRtColorTex;
  main_rt_->GetRtv()->GetResource(mainRtColorTex.GetAddressOf());

  ComPtr<ID3D11Resource> backBuf;
  mSwapChain->GetRtv()->GetResource(backBuf.GetAddressOf());

  ctx->CopyResource(backBuf.Get(), mainRtColorTex.Get());
}


auto Renderer::Impl::Present() noexcept -> void {
  mSwapChain->Present(static_cast<UINT>(sync_interval_));

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
  return default_material_;
}


auto Renderer::Impl::GetCubeMesh() const noexcept -> ObserverPtr<Mesh> {
  return cube_mesh_;
}


auto Renderer::Impl::GetPlaneMesh() const noexcept -> ObserverPtr<Mesh> {
  return plane_mesh_;
}


auto Renderer::Impl::GetSphereMesh() const noexcept -> ObserverPtr<Mesh> {
  return sphere_mesh_;
}


auto Renderer::Impl::GetSyncInterval() const noexcept -> int {
  return sync_interval_;
}


auto Renderer::Impl::SetSyncInterval(int const interval) noexcept -> void {
  sync_interval_ = interval;
}


auto Renderer::Impl::GetMultisamplingMode() const noexcept -> MultisamplingMode {
  return msaa_mode_;
}


auto Renderer::Impl::SetMultisamplingMode(MultisamplingMode const mode) noexcept -> void {
  msaa_mode_ = mode;
}


auto Renderer::Impl::IsDepthNormalPrePassEnabled() const noexcept -> bool {
  return depth_normal_pre_pass_enabled_;
}


auto Renderer::Impl::SetDepthNormalPrePassEnabled(bool const enabled) noexcept -> void {
  depth_normal_pre_pass_enabled_ = enabled;

  if (!enabled && IsSsaoEnabled()) {
    SetSsaoEnabled(false);
  }
}


auto Renderer::Impl::IsUsingPreciseColorFormat() const noexcept -> bool {
  return color_buffer_format_ == precise_color_buffer_format_;
}


auto Renderer::Impl::SetUsePreciseColorFormat(bool const value) noexcept -> void {
  color_buffer_format_ = value ? precise_color_buffer_format_ : imprecise_color_buffer_format_;
}


auto Renderer::Impl::GetShadowDistance() const noexcept -> float {
  return shadow_distance_;
}


auto Renderer::Impl::SetShadowDistance(float const shadowDistance) noexcept -> void {
  shadow_distance_ = std::max(shadowDistance, 0.0f);
}


auto Renderer::Impl::GetShadowCascadeCount() const noexcept -> int {
  return cascade_count_;
}


auto Renderer::Impl::SetShadowCascadeCount(int const cascadeCount) noexcept -> void {
  cascade_count_ = std::clamp(cascadeCount, 1, MAX_CASCADE_COUNT);
  int const splitCount{cascade_count_ - 1};

  for (int i = 1; i < splitCount; i++) {
    cascade_splits_[i] = std::max(cascade_splits_[i - 1], cascade_splits_[i]);
  }
}


auto Renderer::Impl::GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const> {
  return {std::begin(cascade_splits_), static_cast<std::size_t>(cascade_count_ - 1)};
}


auto Renderer::Impl::SetNormalizedShadowCascadeSplit(int const idx, float const split) noexcept -> void {
  auto const splitCount{cascade_count_ - 1};

  if (idx < 0 || idx >= splitCount) {
    return;
  }

  float const clampMin{idx == 0 ? 0.0f : cascade_splits_[idx - 1]};
  float const clampMax{idx == splitCount - 1 ? 1.0f : cascade_splits_[idx + 1]};

  cascade_splits_[idx] = std::clamp(split, clampMin, clampMax);
}


auto Renderer::Impl::IsVisualizingShadowCascades() const noexcept -> bool {
  return shadow_cascades_;
}


auto Renderer::Impl::VisualizeShadowCascades(bool const visualize) noexcept -> void {
  shadow_cascades_ = visualize;
}


auto Renderer::Impl::GetShadowFilteringMode() const noexcept -> ShadowFilteringMode {
  return shadow_filtering_mode_;
}


auto Renderer::Impl::SetShadowFilteringMode(ShadowFilteringMode const filteringMode) noexcept -> void {
  shadow_filtering_mode_ = filteringMode;
}


auto Renderer::Impl::IsSsaoEnabled() const noexcept -> bool {
  return ssao_enabled_;
}


auto Renderer::Impl::SetSsaoEnabled(bool const enabled) noexcept -> void {
  ssao_enabled_ = enabled;

  if (enabled && !IsDepthNormalPrePassEnabled()) {
    SetDepthNormalPrePassEnabled(true);
  }
}


auto Renderer::Impl::GetSsaoParams() const noexcept -> SsaoParams const& {
  return ssao_params_;
}


auto Renderer::Impl::SetSsaoParams(SsaoParams const& ssaoParams) noexcept -> void {
  if (ssao_params_.sampleCount != ssaoParams.sampleCount) {
    RecreateSsaoSamples(ssaoParams.sampleCount);
  }

  ssao_params_ = ssaoParams;
}


auto Renderer::Impl::GetGamma() const noexcept -> f32 {
  return 1.f / inv_gamma_;
}


auto Renderer::Impl::SetGamma(f32 const gamma) noexcept -> void {
  inv_gamma_ = 1.f / gamma;
}


auto Renderer::Impl::Register(StaticMeshComponent const& staticMeshComponent) noexcept -> void {
  std::unique_lock const lock{mStaticMeshMutex};
  static_mesh_components_.emplace_back(std::addressof(staticMeshComponent));
}


auto Renderer::Impl::Unregister(StaticMeshComponent const& staticMeshComponent) noexcept -> void {
  std::unique_lock const lock{mStaticMeshMutex};
  std::erase(static_mesh_components_, std::addressof(staticMeshComponent));
}


auto Renderer::Impl::Register(LightComponent const& lightComponent) noexcept -> void {
  std::unique_lock const lock{mLightMutex};
  lights_.emplace_back(std::addressof(lightComponent));
}


auto Renderer::Impl::Unregister(LightComponent const& lightComponent) noexcept -> void {
  std::unique_lock const lock{mLightMutex};
  std::erase(lights_, std::addressof(lightComponent));
}


auto Renderer::Impl::Register(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{mGameCameraMutex};
  game_render_cameras_.emplace_back(&cam);
}


auto Renderer::Impl::Unregister(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{mGameCameraMutex};
  std::erase(game_render_cameras_, &cam);
}
}
