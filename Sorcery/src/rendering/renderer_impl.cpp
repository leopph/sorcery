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


auto Renderer::Impl::ExtractCurrentState(FramePacket& packet) const -> void {
  packet.buffers.clear();
  packet.textures.clear();
  packet.light_data.clear();
  packet.mesh_data.clear();
  packet.submesh_data.clear();
  packet.cam_data.clear();

  packet.light_data.reserve(lights_.size());

  for (auto const light : lights_) {
    packet.light_data.emplace_back(light->GetColor(), light->GetIntensity(), light->GetDirection(),
      light->GetEntity().GetTransform().GetWorldPosition(), light->GetType(), light->GetRange(), light->GetInnerAngle(),
      light->GetOuterAngle(), light->IsCastingShadow(), light->GetShadowNearPlane(), light->GetShadowNormalBias(),
      light->GetShadowDepthBias(), light->GetShadowExtension());
  }

  packet.mesh_data.reserve(static_mesh_components_.size());

  auto const find_or_emplace_back_buffer{
    [&packet](graphics::SharedDeviceChildHandle<graphics::Buffer> const& buf) -> unsigned {
      unsigned idx;

      if (auto const it{std::ranges::find(packet.buffers, buf)}; it != std::ranges::end(packet.buffers)) {
        idx = static_cast<unsigned>(it - packet.buffers.begin());
      } else {
        idx = static_cast<unsigned>(packet.buffers.size());
        packet.buffers.emplace_back(buf);
      }

      return idx;
    }
  };

  auto const find_or_emplace_back_texture{
    [&packet](graphics::SharedDeviceChildHandle<graphics::Texture> const& tex) -> unsigned {
      unsigned idx;

      if (auto const it{std::ranges::find(packet.textures, tex)}; it != std::ranges::end(packet.textures)) {
        idx = static_cast<unsigned>(it - packet.textures.begin());
      } else {
        idx = static_cast<unsigned>(packet.textures.size());
        packet.textures.emplace_back(tex);
      }

      return idx;
    }
  };

  for (auto const comp : static_mesh_components_) {
    auto const mesh{comp->GetMesh()};

    if (!mesh) {
      continue;
    }

    auto const pos_buf_local_idx{find_or_emplace_back_buffer(mesh->GetPositionBuffer())};
    auto const norm_buf_local_idx{find_or_emplace_back_buffer(mesh->GetNormalBuffer())};
    auto const tan_buf_local_idx{find_or_emplace_back_buffer(mesh->GetTangentBuffer())};
    auto const uv_buf_local_idx{find_or_emplace_back_buffer(mesh->GetUvBuffer())};
    auto const idx_buf_local_idx{find_or_emplace_back_buffer(mesh->GetIndexBuffer())};
    packet.mesh_data.emplace_back(pos_buf_local_idx, norm_buf_local_idx, tan_buf_local_idx, uv_buf_local_idx,
      idx_buf_local_idx);

    packet.submesh_data.reserve(packet.submesh_data.size() + mesh->GetSubmeshCount());

    for (auto const& submesh : mesh->GetSubMeshes()) {
      auto const mtl{comp->GetMaterials()[submesh.material_index]};

      if (!mtl) {
        continue;
      }

      auto const mtl_buf_local_idx{find_or_emplace_back_buffer(mtl->GetBuffer())};

      for (auto const tex : {
             mtl->GetAlbedoMap(), mtl->GetMetallicMap(), mtl->GetRoughnessMap(), mtl->GetAoMap(), mtl->GetNormalMap(),
             mtl->GetOpacityMask()
           }) {
        if (tex) {
          find_or_emplace_back_texture(tex->GetTex());
        }
      }

      packet.submesh_data.emplace_back(static_cast<unsigned>(packet.mesh_data.size() - 1), submesh.base_vertex,
        submesh.first_index, submesh.index_count, mtl_buf_local_idx, submesh.bounds);

      packet.instance_data.emplace_back(static_cast<unsigned>(packet.submesh_data.size() - 1),
        comp->GetEntity().GetTransform().GetLocalToWorldMatrix());
    }
  }

  packet.cam_data.reserve(game_render_cameras_.size());

  for (auto const cam : game_render_cameras_) {
    packet.cam_data.emplace_back(cam->GetPosition(), cam->GetRightAxis(), cam->GetUpAxis(), cam->GetForwardAxis(),
      cam->GetNearClipPlane(), cam->GetFarClipPlane(), cam->GetType(), cam->GetVerticalPerspectiveFov(),
      cam->GetVerticalOrthographicSize(), cam->GetRenderTarget());
  }
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


auto Renderer::Impl::CullStaticMeshComponents(Frustum const& frustum_ws,
                                              std::pmr::vector<StaticMeshSubmeshIndex>& visible_indices) const -> void {
  visible_indices.clear();

  for (int i = 0; i < static_cast<int>(static_mesh_components_.size()); i++) {
    if (auto const mesh{static_mesh_components_[i]->GetMesh()}) {
      if (auto const& modelMtx{static_mesh_components_[i]->GetEntity().GetTransform().GetLocalToWorldMatrix()};
        frustum_ws.Intersects(mesh->GetBounds().Transform(modelMtx))) {
        auto const submeshes{mesh->GetSubMeshes()};
        for (int j{0}; j < std::ssize(submeshes); j++) {
          if (frustum_ws.Intersects(submeshes[j].bounds.Transform(modelMtx))) {
            visible_indices.emplace_back(i, j);
          }
        }
      }
    }
  }
}


auto Renderer::Impl::CullLights(Frustum const& frustum_ws, std::pmr::vector<int> visible_indices) const -> void {
  visible_indices.clear();

  for (int light_idx = 0; light_idx < static_cast<int>(lights_.size()); light_idx++) {
    switch (auto const light{lights_[light_idx]}; light->GetType()) {
      case LightComponent::Type::Directional: {
        visible_indices.emplace_back(light_idx);
        break;
      }

      case LightComponent::Type::Spot: {
        auto const light_vertices_ws{
          [light] {
            auto vertices{CalculateSpotLightLocalVertices(*light)};

            for (auto const model_mtx_no_scale{
                   light->GetEntity().GetTransform().CalculateLocalToWorldMatrixWithoutScale()
                 }; auto& vertex : vertices) {
              vertex = Vector3{Vector4{vertex, 1} * model_mtx_no_scale};
            }

            return vertices;
          }()
        };

        if (frustum_ws.Intersects(AABB::FromVertices(light_vertices_ws))) {
          visible_indices.emplace_back(light_idx);
        }

        break;
      }

      case LightComponent::Type::Point: {
        if (BoundingSphere const bounds_ws{
          Vector3{light->GetEntity().GetTransform().GetWorldPosition()}, light->GetRange()
        }; frustum_ws.Intersects(bounds_ws)) {
          visible_indices.emplace_back(light_idx);
        }
        break;
      }
    }
  }
}


auto Renderer::Impl::SetPerFrameConstants(ConstantBuffer<ShaderPerFrameConstants>& cb, int const rt_width,
                                          int const rt_height) const noexcept -> void {
  cb.Update(ShaderPerFrameConstants{
    .ambientLightColor = Scene::GetActiveScene()->GetAmbientLightVector(), .shadowCascadeCount = cascade_count_,
    .screenSize = Vector2{rt_width, rt_height}, .visualizeShadowCascades = shadow_cascades_,
    .shadowFilteringMode = static_cast<int>(shadow_filtering_mode_)
  });
}


auto Renderer::Impl::SetPerViewConstants(ConstantBuffer<ShaderPerViewConstants>& cb, Matrix4 const& view_mtx,
                                         Matrix4 const& proj_mtx, ShadowCascadeBoundaries const& cascade_bounds,
                                         Vector3 const& view_pos) -> void {
  ShaderPerViewConstants data;
  data.viewMtx = view_mtx;
  data.projMtx = proj_mtx;
  data.invProjMtx = proj_mtx.Inverse();
  data.viewProjMtx = view_mtx * proj_mtx;
  data.viewPos = view_pos;

  for (int i = 0; i < MAX_CASCADE_COUNT; i++) {
    data.shadowCascadeSplitDistances[i] = cascade_bounds[i].farClip;
  }

  cb.Update(data);
}


auto Renderer::Impl::SetPerDrawConstants(ConstantBuffer<ShaderPerDrawConstants>& cb, Matrix4 const& model_mtx) -> void {
  cb.Update(ShaderPerDrawConstants{.modelMtx = model_mtx, .invTranspModelMtx = model_mtx.Inverse().Transpose()});
}


auto Renderer::Impl::DrawDirectionalShadowMaps(std::span<int const> light_indices, Camera const& cam, float rt_aspect,
                                               ShadowCascadeBoundaries const& shadow_cascade_boundaries,
                                               std::array<Matrix4, MAX_CASCADE_COUNT>& shadow_view_proj_matrices,
                                               graphics::CommandList& cmd) -> void {
  cmd.SetPipelineState(*depth_only_pso_);
  cmd.SetRenderTargets({}, dir_shadow_map_arr_->GetTex().get());
  cmd.ClearDepthStencil(*dir_shadow_map_arr_->GetTex(), D3D12_CLEAR_FLAG_DEPTH, 0.0f, 0, {});

  for (auto const lightIdx : light_indices) {
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
        [&cam, rt_aspect, camNear, camFar] {
          std::array<Vector3, 8> ret;

          Vector3 const nearWorldForward{cam.GetPosition() + cam.GetForwardAxis() * camNear};
          Vector3 const farWorldForward{cam.GetPosition() + cam.GetForwardAxis() * camFar};

          switch (cam.GetType()) {
            case Camera::Type::Perspective: {
              float const tanHalfFov{std::tan(ToRadians(cam.GetVerticalPerspectiveFov() / 2.0f))};
              float const nearExtentY{camNear * tanHalfFov};
              float const nearExtentX{nearExtentY * rt_aspect};
              float const farExtentY{camFar * tanHalfFov};
              float const farExtentX{farExtentY * rt_aspect};

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
              float const extentY{extentX / rt_aspect};

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
          [&frustumVertsWS, &shadow_cascade_boundaries, cascadeIdx, camNear, frustumDepth] {
            auto const [cascadeNear, cascadeFar]{shadow_cascade_boundaries[cascadeIdx]};

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

        shadow_view_proj_matrices[cascadeIdx] = shadowViewMtx * shadowProjMtx;

        cmd.SetRenderTargets({}, dir_shadow_map_arr_->GetTex().get());
        cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, rt_idx), cascadeIdx);

        D3D12_VIEWPORT const shadowViewport{
          0, 0, static_cast<float>(shadowMapSize), static_cast<float>(shadowMapSize), 0, 1
        };

        cmd.SetViewports(std::array{shadowViewport});

        if (next_per_view_cb_idx_ >= per_view_cbs_[frame_idx_].size()) {
          CreatePerViewConstantBuffers(1);
        }

        SetPerViewConstants(per_view_cbs_[frame_idx_][next_per_view_cb_idx_], shadowViewMtx, shadowProjMtx,
          ShadowCascadeBoundaries{}, Vector3{});
        cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, per_view_cb_idx),
          per_view_cbs_[frame_idx_][next_per_view_cb_idx_].GetBuffer()->GetConstantBuffer());

        Frustum const shadowFrustumWS{shadow_view_proj_matrices[cascadeIdx]};

        std::pmr::vector<StaticMeshSubmeshIndex> static_mesh_indices{&GetTmpMemRes()};
        CullStaticMeshComponents(shadowFrustumWS, static_mesh_indices);

        for (auto const& [mesh_idx, submesh_idx] : static_mesh_indices) {
          auto const component{static_mesh_components_[mesh_idx]};
          auto const mesh{component->GetMesh()};

          if (!mesh) {
            continue;
          }

          auto const submesh{mesh->GetSubMeshes()[submesh_idx]};
          auto const mtl{component->GetMaterials()[submesh.material_index]};

          if (!mtl) {
            continue;
          }

          auto const mtl_buf{mtl->GetBuffer()};

          if (next_per_draw_cb_idx_ >= per_draw_cbs_[frame_idx_].size()) {
            CreatePerDrawConstantBuffers(1);
          }

          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, pos_buf_idx),
            mesh->GetPositionBuffer()->GetShaderResource());
          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, uv_buf_idx), mesh->GetUvBuffer()->GetShaderResource());
          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, mtl_idx), mtl_buf->GetShaderResource());
          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, per_draw_cb_idx),
            per_draw_cbs_[frame_idx_][next_per_draw_cb_idx_].GetBuffer()->GetConstantBuffer());
          cmd.DrawIndexedInstanced(submesh.index_count, 1, submesh.first_index, submesh.base_vertex, 0);

          ++next_per_draw_cb_idx_;
        }

        ++next_per_view_cb_idx_;
      }

      break;
    }
  }
}


auto Renderer::Impl::DrawPunctualShadowMaps(PunctualShadowAtlas const& atlas, graphics::CommandList& cmd) -> void {
  cmd.SetPipelineState(*depth_only_pso_);
  cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, rt_idx), 0);
  cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, samp_idx), samp_af16_wrap_.Get());

  cmd.SetRenderTargets({}, atlas.GetTex().get());
  cmd.ClearDepthStencil(*atlas.GetTex(), D3D12_CLEAR_FLAG_DEPTH, 0, 0, {});

  auto const cell_size_norm{atlas.GetNormalizedElementSize()};

  for (auto i = 0; i < atlas.GetElementCount(); i++) {
    auto const& cell{atlas.GetCell(i)};
    auto const cell_offset_norm{atlas.GetNormalizedElementOffset(i)};
    auto const subcell_size{cell_size_norm * cell.GetNormalizedElementSize() * static_cast<float>(atlas.GetSize())};

    for (auto j = 0; j < cell.GetElementCount(); j++) {
      if (auto const& subcell{cell.GetSubcell(j)}) {
        auto const subcell_offset{
          (cell_offset_norm + cell.GetNormalizedElementOffset(j) * cell_size_norm) * static_cast<float>(atlas.GetSize())
        };

        D3D12_VIEWPORT const viewport{subcell_offset[0], subcell_offset[1], subcell_size, subcell_size, 0, 1};

        cmd.SetViewports(std::span{&viewport, 1});

        if (next_per_view_cb_idx_ >= per_view_cbs_[frame_idx_].size()) {
          CreatePerViewConstantBuffers(1);
        }

        SetPerViewConstants(per_view_cbs_[frame_idx_][next_per_view_cb_idx_], Matrix4::Identity(),
          subcell->shadowViewProjMtx, ShadowCascadeBoundaries{}, Vector3{});
        cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, per_view_cb_idx),
          per_view_cbs_[frame_idx_][next_per_view_cb_idx_].GetBuffer()->GetConstantBuffer());

        Frustum const shadow_frustum_ws{subcell->shadowViewProjMtx};

        std::pmr::vector<StaticMeshSubmeshIndex> static_mesh_indices{&GetTmpMemRes()};
        CullStaticMeshComponents(shadow_frustum_ws, static_mesh_indices);

        for (auto const& [mesh_idx, submesh_idx] : static_mesh_indices) {
          auto const component{static_mesh_components_[mesh_idx]};
          auto const mesh{component->GetMesh()};

          if (!mesh) {
            continue;
          }

          auto const submesh{mesh->GetSubMeshes()[submesh_idx]};
          auto const mtl{component->GetMaterials()[submesh.material_index]};

          if (!mtl) {
            continue;
          }

          auto const mtl_buf{mtl->GetBuffer()};

          if (next_per_draw_cb_idx_ >= per_draw_cbs_[frame_idx_].size()) {
            CreatePerDrawConstantBuffers(1);
          }

          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, pos_buf_idx),
            mesh->GetPositionBuffer()->GetShaderResource());
          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, uv_buf_idx), mesh->GetUvBuffer()->GetShaderResource());
          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, mtl_idx), mtl_buf->GetShaderResource());
          cmd.SetPipelineParameter(offsetof(DepthOnlyDrawParams, per_draw_cb_idx),
            per_draw_cbs_[frame_idx_][next_per_draw_cb_idx_].GetBuffer()->GetConstantBuffer());
          cmd.DrawIndexedInstanced(submesh.index_count, 1, submesh.first_index, submesh.base_vertex, 0);

          ++next_per_draw_cb_idx_;
        }

        ++next_per_view_cb_idx_;
      }
    }
  }
}


auto Renderer::Impl::DrawSkybox(graphics::CommandList& cmd) noexcept -> void {
  auto const cubemap{Scene::GetActiveScene()->GetSkybox()};

  if (!cubemap) {
    return;
  }

  cmd.SetPipelineState(*skybox_pso_);
  cmd.SetPipelineParameter(offsetof(SkyboxDrawParams, pos_buf_idx),
    cube_mesh_->GetPositionBuffer()->GetShaderResource());
  cmd.SetPipelineParameter(offsetof(SkyboxDrawParams, cubemap_idx), cubemap->GetTex()->GetShaderResource());
  cmd.SetPipelineParameter(offsetof(SkyboxDrawParams, samp_idx), samp_af16_clamp_.Get());
  // TODO set per view cb
  cmd.DrawIndexedInstanced(static_cast<UINT>(kCubeIndices.size()), 1, 0, 0, 0);
}


auto Renderer::Impl::PostProcess(graphics::Texture const& src, graphics::Texture const& dst,
                                 graphics::CommandList& cmd) const noexcept -> void {
  cmd.SetPipelineState(*post_process_pso_);
  cmd.SetPipelineParameter(offsetof(PostProcessDrawParams, in_tex_idx), src.GetShaderResource());
  cmd.SetPipelineParameter(offsetof(PostProcessDrawParams, inv_gamma), *std::bit_cast<UINT*>(&inv_gamma_));
  cmd.SetRenderTargets(std::array{dst}, nullptr);
  cmd.DrawInstanced(3, 1, 0, 0);
}


auto Renderer::Impl::ClearGizmoDrawQueue() noexcept -> void {
  gizmo_colors_.clear();
  line_gizmo_vertex_data_.clear();
}


auto Renderer::Impl::ReleaseTempRenderTargets() noexcept -> void {
  std::erase_if(tmp_render_targets_, [](TempRenderTargetRecord& tmpRtRecord) {
    tmpRtRecord.age_in_frames += 1;
    return tmpRtRecord.age_in_frames >= max_tmp_rt_age_;
  });
}


auto Renderer::Impl::RecreateSsaoSamples(int const sample_count) const noexcept -> void {
  ssao_samples_buffer_->Resize(sample_count);
  auto const ssao_samples{ssao_samples_buffer_->GetData()};

  std::uniform_real_distribution dist{0.0f, 1.0f};
  std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

  for (auto i{0}; i < sample_count; i++) {
    Vector3 sample{dist(gen) * 2 - 1, dist(gen) * 2 - 1, dist(gen)};
    Normalize(sample);
    sample *= dist(gen);

    auto scale{static_cast<float>(i) / static_cast<float>(sample_count)};
    scale = std::lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;

    ssao_samples[i] = Vector4{sample, 0};
  }
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
    device_->CreateBuffer(graphics::BufferDesc{white_color_data.size(), 0, false, false, false}, D3D12_HEAP_TYPE_UPLOAD)
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
  command_lists_[0]->CopyTextureRegion(*white_tex_, 0, 0, 0, 0, *white_tex_upload_buf.get(),
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT{
      0, D3D12_SUBRESOURCE_FOOTPRINT{white_tex_format, 1, 1, 1, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT}
    });
  command_lists_[0]->Barrier({}, {}, std::array{
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_NO_ACCESS,
      D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE, ssao_noise_tex_.get(),
      D3D12_BARRIER_SUBRESOURCE_RANGE{0, 1, 0, 1, 0, 1}, D3D12_TEXTURE_BARRIER_FLAG_NONE
    },
    graphics::TextureBarrier{
      D3D12_BARRIER_SYNC_COPY, D3D12_BARRIER_SYNC_NONE, D3D12_BARRIER_ACCESS_COPY_DEST, D3D12_BARRIER_ACCESS_NO_ACCESS,
      D3D12_BARRIER_LAYOUT_COPY_DEST, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE, white_tex_.get(),
      D3D12_BARRIER_SUBRESOURCE_RANGE{0, 1, 0, 1, 0, 1}, D3D12_TEXTURE_BARRIER_FLAG_NONE
    }
  });

  std::ignore = command_lists_[0]->End();
  device_->ExecuteCommandLists(std::span{command_lists_[0].get(), 1});
  std::ignore = device_->WaitIdle();
}


auto Renderer::Impl::ShutDown() -> void {
  gWindow.OnWindowSize.remove_handler(this, &OnWindowSize);
}


auto Renderer::Impl::Render() -> void {
  auto& frame_packet{frame_packets_[frame_idx_]};
  ExtractCurrentState(frame_packet);

  frame_idx_ = (frame_idx_ + 1) % max_frames_in_flight_;
}


auto Renderer::Impl::DrawCamera(Camera const& cam, RenderTarget const* const rt) -> void {
  auto& cmd{*command_lists_[frame_idx_]};
  if (!cmd.Begin(nullptr)) {
    return;
  }

  auto const rtWidth{rt ? rt->GetDesc().width : gWindow.GetClientAreaSize().width};
  auto const rtHeight{rt ? rt->GetDesc().height : gWindow.GetClientAreaSize().height};
  auto const rtAspect{static_cast<float>(rtWidth) / static_cast<float>(rtHeight)};

  RenderTarget::Desc const hdrRtDesc{
    rtWidth, rtHeight, color_buffer_format_, depth_format_, static_cast<UINT>(GetMultisamplingMode()),
    L"Camera HDR RenderTarget", false
  };

  auto const& hdrRt{GetTemporaryRenderTarget(hdrRtDesc)};


  auto const clearColor{
    Scene::GetActiveScene()->GetSkyMode() == SkyMode::Color
      ? Vector4{Scene::GetActiveScene()->GetSkyColor(), 1}
      : Vector4{0, 0, 0, 1}
  };

  cmd.ClearRenderTarget(*hdrRt.GetColorTex(), std::span<FLOAT const, 4>{clearColor.GetData(), 4}, {});


  D3D12_VIEWPORT const viewport{0, 0, static_cast<FLOAT>(rtWidth), static_cast<FLOAT>(rtHeight), 0, 1};

  SetPerFrameConstants(per_frame_cbs_[frame_idx_], static_cast<int>(rtWidth), static_cast<int>(rtHeight));

  auto const camPos{cam.GetPosition()};
  auto const camViewMtx{cam.CalculateViewMatrix()};
  auto const camProjMtx{GetProjectionMatrixForRendering(cam.CalculateProjectionMatrix(rtAspect))};
  auto const camViewProjMtx{camViewMtx * camProjMtx};
  Frustum const camFrustWS{camViewProjMtx};

  std::pmr::vector<int> light_indices{&GetTmpMemRes()};
  CullLights(camFrustWS, light_indices);

  // Shadow pass
  std::array<Matrix4, MAX_CASCADE_COUNT> shadowViewProjMatrices;
  auto const shadowCascadeBoundaries{CalculateCameraShadowCascadeBoundaries(cam)};
  DrawDirectionalShadowMaps(light_indices, cam, rtAspect, shadowCascadeBoundaries, shadowViewProjMatrices, cmd);

  punctual_shadow_atlas_->Update(lights_, light_indices, cam, camViewProjMtx, shadow_distance_);
  DrawPunctualShadowMaps(*punctual_shadow_atlas_, cmd);

  std::pmr::vector<StaticMeshSubmeshIndex> static_mesh_indices{&GetTmpMemRes()};
  CullStaticMeshComponents(camFrustWS, static_mesh_indices);

  if (next_per_view_cb_idx_ >= per_view_cbs_[frame_idx_].size()) {
    CreatePerViewConstantBuffers(1);
  }

  auto const cam_per_view_cb_idx{next_per_view_cb_idx_};
  ++next_per_view_cb_idx_;

  SetPerViewConstants(per_view_cbs_[frame_idx_][cam_per_view_cb_idx], camViewMtx, camProjMtx, shadowCascadeBoundaries,
    camPos);

  cmd.ClearDepthStencil(*hdrRt.GetDepthStencilTex(), D3D12_CLEAR_FLAG_DEPTH, 0, 0, {});

  auto const& normalRt{
    GetTemporaryRenderTarget(RenderTarget::Desc{
      rtWidth, rtHeight, normal_buffer_format_, std::nullopt, 1, L"Camera Normal RT"
    })
  };

  // Depth-Normal pre-pass
  if (depth_normal_pre_pass_enabled_) {
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

    cmd.SetPipelineState(*depth_normal_pso_);
    cmd.SetRenderTargets(std::span{actualNormalRt.GetColorTex().get(), 1}, hdrRt.GetDepthStencilTex().get());
    cmd.ClearRenderTarget(*actualNormalRt.GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});

    for (auto const [staticMeshIdx, submeshIdx] : static_mesh_indices) {
      auto const& component{*static_mesh_components_[staticMeshIdx]};
      auto const mesh{component.GetMesh()};

      if (!mesh) {
        continue;
      }

      auto const submesh{mesh->GetSubMeshes()[submeshIdx]};
      auto const mtl{component.GetMaterials()[submesh.material_index]};

      if (!mtl) {
        continue;
      }

      if (next_per_draw_cb_idx_ >= per_draw_cbs_[frame_idx_].size()) {
        CreatePerDrawConstantBuffers(1);
      }

      SetPerDrawConstants(per_draw_cbs_[frame_idx_][next_per_draw_cb_idx_],
        component.GetEntity().GetTransform().GetLocalToWorldMatrix());

      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, pos_buf_idx),
        mesh->GetPositionBuffer()->GetShaderResource());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, norm_buf_idx),
        mesh->GetNormalBuffer()->GetShaderResource());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, tan_buf_idx),
        mesh->GetTangentBuffer()->GetShaderResource());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, uv_buf_idx), mesh->GetUvBuffer()->GetShaderResource());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, mtl_idx), mtl->GetBuffer()->GetConstantBuffer());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, samp_idx), samp_af16_wrap_.Get());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, rt_idx), 0);
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, per_draw_cb_idx),
        per_draw_cbs_[frame_idx_][next_per_draw_cb_idx_].GetBuffer()->GetConstantBuffer());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, per_view_cb_idx),
        per_view_cbs_[frame_idx_][cam_per_view_cb_idx].GetBuffer()->GetConstantBuffer());
      cmd.SetPipelineParameter(offsetof(DepthNormalDrawParams, per_frame_cb_idx),
        per_frame_cbs_[frame_idx_].GetBuffer()->GetConstantBuffer());

      ++next_per_draw_cb_idx_;
    }

    // If we have MSAA enabled, actualNormalRt is an MSAA texture that we have to resolve into normalRt
    if (GetMultisamplingMode() != MultisamplingMode::Off) {
      cmd.Resolve(*normalRt.GetColorTex(), *actualNormalRt.GetColorTex(), *normalRt.GetDesc().color_format);
    }
  }

  auto ssaoTexSrv{white_tex_.get()};

  // SSAO pass
  if (ssao_enabled_) {
    auto const& ssaoRt{
      GetTemporaryRenderTarget(RenderTarget::Desc{rtWidth, rtHeight, ssao_buffer_format_, std::nullopt, 1, L"SSAO RT"})
    };

    // If MSAA is enabled we have to resolve the depth texture before running SSAO
    auto const ssao_depth_tex{
      [this, &hdrRt, &cmd] {
        if (GetMultisamplingMode() == MultisamplingMode::Off) {
          return hdrRt.GetDepthStencilTex();
        }

        auto ssaoDepthRtDesc{hdrRt.GetDesc()};
        ssaoDepthRtDesc.color_format = DXGI_FORMAT_R32_FLOAT;
        ssaoDepthRtDesc.sample_count = 1;
        ssaoDepthRtDesc.depth_stencil_format = std::nullopt;
        ssaoDepthRtDesc.enable_unordered_access = true;
        auto const& ssaoDepthRt{GetTemporaryRenderTarget(ssaoDepthRtDesc)};

        cmd.SetPipelineState(*depth_resolve_pso_);
        cmd.SetPipelineParameter(offsetof(DepthResolveDrawParams, in_tex_idx),
          ssaoDepthRt.GetColorTex()->GetUnorderedAccess());
        cmd.SetPipelineParameter(offsetof(DepthResolveDrawParams, out_tex_idx),
          hdrRt.GetColorTex()->GetUnorderedAccess());
        cmd.Dispatch(
          static_cast<UINT>(std::ceil(static_cast<float>(ssaoDepthRtDesc.width) / DEPTH_RESOLVE_CS_THREADS_X)),
          static_cast<UINT>(std::ceil(static_cast<float>(ssaoDepthRtDesc.height) / DEPTH_RESOLVE_CS_THREADS_Y)),
          static_cast<UINT>(std::ceil(1.0f / DEPTH_RESOLVE_CS_THREADS_Z)));

        return ssaoDepthRt.GetColorTex();
      }()
    };

    cmd.SetPipelineState(*ssao_pso_);
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, noise_tex_idx), ssao_noise_tex_->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, depth_tex_idx), ssao_depth_tex->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, normal_tex_idx), normalRt.GetColorTex()->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, samp_buf_idx),
      ssao_samples_buffer_->GetBuffer()->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, point_clamp_samp_idx), samp_point_clamp_.Get());
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, point_wrap_samp_idx), samp_point_wrap_.Get());
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, radius), *std::bit_cast<UINT*>(&ssao_params_.radius));
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, bias), *std::bit_cast<UINT*>(&ssao_params_.bias));
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, power), *std::bit_cast<UINT*>(&ssao_params_.power));
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, sample_count), ssao_params_.sampleCount);
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, per_view_cb_idx),
      per_view_cbs_[frame_idx_][cam_per_view_cb_idx].GetBuffer()->GetConstantBuffer());
    cmd.SetPipelineParameter(offsetof(SsaoDrawParams, per_frame_cb_idx),
      per_frame_cbs_[frame_idx_].GetBuffer()->GetConstantBuffer());
    cmd.SetRenderTargets(std::span{ssaoRt.GetColorTex().get(), 1}, nullptr);
    cmd.ClearRenderTarget(*ssaoRt.GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cmd.DrawInstanced(3, 1, 0, 0);

    auto const& ssaoBlurRt{
      GetTemporaryRenderTarget([&ssaoRt] {
        auto ret{ssaoRt.GetDesc()};
        ret.debug_name = L"SSAO Blur RT";
        return ret;
      }())
    };

    cmd.SetPipelineState(*ssao_blur_pso_);
    cmd.SetPipelineParameter(offsetof(SsaoBlurDrawParams, in_tex_idx), ssaoBlurRt.GetColorTex()->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(SsaoBlurDrawParams, point_clamp_samp_idx), samp_point_clamp_.Get());
    cmd.SetRenderTargets(std::span{ssaoBlurRt.GetColorTex().get(), 1}, nullptr);
    cmd.ClearRenderTarget(*ssaoBlurRt.GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cmd.DrawInstanced(3, 1, 0, 0);

    ssaoTexSrv = ssaoBlurRt.GetColorTex().get();
  }

  // Full forward lighting pass

  auto const lightCount{std::ssize(light_indices)};
  light_buffer_->Resize(static_cast<int>(lightCount));
  auto const lightBufferData{light_buffer_->GetData()};

  for (int i = 0; i < lightCount; i++) {
    lightBufferData[i].color = lights_[light_indices[i]]->GetColor();
    lightBufferData[i].intensity = lights_[light_indices[i]]->GetIntensity();
    lightBufferData[i].type = static_cast<int>(lights_[light_indices[i]]->GetType());
    lightBufferData[i].direction = lights_[light_indices[i]]->GetDirection();
    lightBufferData[i].isCastingShadow = FALSE;
    lightBufferData[i].range = lights_[light_indices[i]]->GetRange();
    lightBufferData[i].halfInnerAngleCos = std::cos(ToRadians(lights_[light_indices[i]]->GetInnerAngle() / 2.0f));
    lightBufferData[i].halfOuterAngleCos = std::cos(ToRadians(lights_[light_indices[i]]->GetOuterAngle() / 2.0f));
    lightBufferData[i].position = lights_[light_indices[i]]->GetEntity().GetTransform().GetWorldPosition();
    lightBufferData[i].depthBias = lights_[light_indices[i]]->GetShadowDepthBias();
    lightBufferData[i].normalBias = lights_[light_indices[i]]->GetShadowNormalBias();

    for (auto& sample : lightBufferData[i].sampleShadowMap) {
      sample = FALSE;
    }
  }

  // Set directional light shadow constant data

  for (auto i{0}; i < lightCount; i++) {
    if (auto const light{lights_[light_indices[i]]};
      light->GetType() == LightComponent::Type::Directional && light->IsCastingShadow()) {
      lightBufferData[i].isCastingShadow = TRUE;

      for (auto cascadeIdx{0}; cascadeIdx < cascade_count_; cascadeIdx++) {
        lightBufferData[i].sampleShadowMap[cascadeIdx] = TRUE;
        lightBufferData[i].shadowViewProjMatrices[cascadeIdx] = shadowViewProjMatrices[cascadeIdx];
      }

      break;
    }
  }

  punctual_shadow_atlas_->SetLookUpInfo(lightBufferData);

  cmd.SetPipelineState(*object_pso_);
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, mtl_samp_idx), samp_af16_wrap_.Get());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, point_clamp_samp_idx), samp_point_clamp_.Get());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, shadow_samp_idx), samp_cmp_pcf_ge_.Get());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, ssao_tex_idx), ssaoTexSrv->GetShaderResource());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, light_buf_idx), light_buffer_->GetBuffer()->GetShaderResource());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, dir_shadow_arr_idx),
    dir_shadow_map_arr_->GetTex()->GetShaderResource());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, punc_shadow_atlas_idx),
    punctual_shadow_atlas_->GetTex()->GetShaderResource());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, per_view_cb_idx),
    per_view_cbs_[frame_idx_][cam_per_view_cb_idx].GetBuffer()->GetConstantBuffer());
  cmd.SetPipelineParameter(offsetof(ObjectDrawParams, per_frame_cb_idx),
    per_frame_cbs_[frame_idx_].GetBuffer()->GetConstantBuffer());
  cmd.SetRenderTargets(std::span{hdrRt.GetColorTex().get(), 1}, hdrRt.GetDepthStencilTex().get());

  for (auto const [staticMeshIdx, submeshIdx] : static_mesh_indices) {
    auto const& component{static_mesh_components_[staticMeshIdx]};
    auto const mesh{component->GetMesh()};

    if (!mesh) {
      continue;
    }

    auto const& submesh{mesh->GetSubMeshes()[submeshIdx]};
    auto const mtl{component->GetMaterials()[submesh.material_index]};

    if (!mtl) {
      continue;
    }

    if (next_per_draw_cb_idx_ >= per_draw_cbs_[frame_idx_].size()) {
      CreatePerDrawConstantBuffers(1);
    }

    SetPerDrawConstants(per_draw_cbs_[frame_idx_][next_per_draw_cb_idx_],
      component->GetEntity().GetTransform().GetLocalToWorldMatrix());

    cmd.SetPipelineParameter(offsetof(ObjectDrawParams, pos_buf_idx), mesh->GetPositionBuffer()->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(ObjectDrawParams, norm_buf_idx), mesh->GetNormalBuffer()->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(ObjectDrawParams, tan_buf_idx), mesh->GetTangentBuffer()->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(ObjectDrawParams, uv_buf_idx), mesh->GetUvBuffer()->GetShaderResource());
    cmd.SetPipelineParameter(offsetof(ObjectDrawParams, mtl_idx), mtl->GetBuffer()->GetConstantBuffer());
    cmd.SetPipelineParameter(offsetof(ObjectDrawParams, per_draw_cb_idx),
      per_draw_cbs_[frame_idx_][next_per_draw_cb_idx_].GetBuffer()->GetConstantBuffer());

    ++next_per_draw_cb_idx_;
  }


  if (auto const& activeScene{Scene::GetActiveScene()};
    activeScene->GetSkyMode() == SkyMode::Skybox && activeScene->GetSkybox()) {
    DrawSkybox(cmd);
  }

  RenderTarget const* postProcessRt;

  if (GetMultisamplingMode() == MultisamplingMode::Off) {
    postProcessRt = std::addressof(hdrRt);
  } else {
    auto resolveHdrRtDesc{hdrRtDesc};
    resolveHdrRtDesc.sample_count = 1;
    auto const& resolveHdrRt{GetTemporaryRenderTarget(resolveHdrRtDesc)};
    cmd.Resolve(*resolveHdrRt.GetColorTex(), *hdrRt.GetColorTex(), *hdrRtDesc.color_format);
    postProcessRt = std::addressof(resolveHdrRt);
  }

  cmd.SetViewports(std::span{&viewport, 1});
  PostProcess(*postProcessRt->GetColorTex(),
    *(rt
        ? rt->GetColorTex()
        : device_->SwapChainGetBuffers(*swap_chain_)[device_->SwapChainGetCurrentBufferIndex(*swap_chain_)]), cmd);
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
  gizmo_color_buffer_->Resize(static_cast<int>(std::ssize(gizmo_colors_)));
  std::ranges::copy(gizmo_colors_, std::begin(gizmo_color_buffer_->GetData()));

  line_gizmo_vertex_data_buffer_->Resize(static_cast<int>(std::ssize(line_gizmo_vertex_data_)));
  std::ranges::copy(line_gizmo_vertex_data_, std::begin(line_gizmo_vertex_data_buffer_->GetData()));

  auto& cmd{*command_lists_[frame_idx_]};
  cmd.SetPipelineState(*line_gizmo_pso_);
  cmd.SetPipelineParameter(offsetof(GizmoDrawParams, vertex_buf_idx),
    line_gizmo_vertex_data_buffer_->GetBuffer()->GetShaderResource());
  cmd.SetPipelineParameter(offsetof(GizmoDrawParams, color_buf_idx),
    gizmo_color_buffer_->GetBuffer()->GetShaderResource());
  // TODO set per view cb

  auto const actual_rt{
    rt
      ? rt->GetColorTex().get()
      : device_->SwapChainGetBuffers(*swap_chain_)[device_->SwapChainGetCurrentBufferIndex(*swap_chain_)].get()
  };
  cmd.SetRenderTargets(std::span{actual_rt, 1}, nullptr);

  if (!line_gizmo_vertex_data_.empty()) {
    cmd.DrawInstanced(2, static_cast<UINT>(line_gizmo_vertex_data_.size()), 0, 0);
  }
}


/*auto Renderer::Impl::ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> const ctx) const noexcept -> void {
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
} TODO */


auto Renderer::Impl::Present() noexcept -> void {
  std::ignore = device_->SwapChainPresent(*swap_chain_, static_cast<UINT>(sync_interval_));
  ClearGizmoDrawQueue();
  ReleaseTempRenderTargets();
}


auto Renderer::Impl::GetDevice() const noexcept -> ObserverPtr<graphics::GraphicsDevice> {
  return device_.get();
}


auto Renderer::Impl::GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget& {
  std::unique_lock const lock{tmp_render_targets_mutex_};

  for (auto& [rt, lastUseInFrames] : tmp_render_targets_) {
    if (rt->GetDesc() == desc && lastUseInFrames != 0 /*The RT wasn't already handed out this frame*/) {
      lastUseInFrames = 0;
      return *rt;
    }
  }

  return *tmp_render_targets_.emplace_back(RenderTarget::New(*device_, desc), 0).rt;
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
  std::unique_lock const lock{static_mesh_mutex_};
  static_mesh_components_.emplace_back(std::addressof(staticMeshComponent));
}


auto Renderer::Impl::Unregister(StaticMeshComponent const& staticMeshComponent) noexcept -> void {
  std::unique_lock const lock{static_mesh_mutex_};
  std::erase(static_mesh_components_, std::addressof(staticMeshComponent));
}


auto Renderer::Impl::Register(LightComponent const& lightComponent) noexcept -> void {
  std::unique_lock const lock{light_mutex_};
  lights_.emplace_back(std::addressof(lightComponent));
}


auto Renderer::Impl::Unregister(LightComponent const& lightComponent) noexcept -> void {
  std::unique_lock const lock{light_mutex_};
  std::erase(lights_, std::addressof(lightComponent));
}


auto Renderer::Impl::Register(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{game_camera_mutex_};
  game_render_cameras_.emplace_back(&cam);
}


auto Renderer::Impl::Unregister(Camera const& cam) noexcept -> void {
  std::unique_lock const lock{game_camera_mutex_};
  std::erase(game_render_cameras_, &cam);
}
}
