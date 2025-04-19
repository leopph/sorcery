#include "scene_renderer.hpp"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <random>

#include "ShadowCascadeBoundary.hpp"
#include "../app.hpp"
#include "../MemoryAllocation.hpp"
#include "../ResourceManager.hpp"
#include "../Window.hpp"
#include "../Resources/Scene.hpp"
#include "../scene_objects/Entity.hpp"
#include "../scene_objects/TransformComponent.hpp"
#include "shaders/shader_interop.h"

#ifndef NDEBUG
#include "shaders/generated/Debug/deferred_lighting_ps.h"
#include "shaders/generated/Debug/deferred_lighting_vs.h"
#include "shaders/generated/Debug/depth_only_ms.h"
#include "shaders/generated/Debug/depth_only_ps.h"
#include "shaders/generated/Debug/depth_resolve_cs.h"
#include "shaders/generated/Debug/gbuffer_ms.h"
#include "shaders/generated/Debug/gbuffer_ps.h"
#include "shaders/generated/Debug/gizmos_line_vs.h"
#include "shaders/generated/Debug/gizmos_ps.h"
#include "shaders/generated/Debug/post_process_ps.h"
#include "shaders/generated/Debug/post_process_vs.h"
#include "shaders/generated/Debug/skybox_ms.h"
#include "shaders/generated/Debug/skybox_ps.h"
#include "shaders/generated/Debug/ssao_blur_ps.h"
#include "shaders/generated/Debug/ssao_main_ps.h"
#include "shaders/generated/Debug/ssao_vs.h"
#include "shaders/generated/Debug/ssr_compose_ps.h"
#include "shaders/generated/Debug/ssr_ps.h"
#include "shaders/generated/Debug/ssr_vs.h"
#include "shaders/generated/Debug/vtx_skinning_cs.h"
#else
#include "shaders/generated/Release/deferred_lighting_ps.h"
#include "shaders/generated/Release/deferred_lighting_vs.h"
#include "shaders/generated/Release/depth_only_ms.h"
#include "shaders/generated/Release/depth_only_ps.h"
#include "shaders/generated/Release/depth_resolve_cs.h"
#include "shaders/generated/Release/gbuffer_ms.h"
#include "shaders/generated/Release/gbuffer_ps.h"
#include "shaders/generated/Release/gizmos_line_vs.h"
#include "shaders/generated/Release/gizmos_ps.h"
#include "shaders/generated/Release/post_process_ps.h"
#include "shaders/generated/Release/post_process_vs.h"
#include "shaders/generated/Release/skybox_ms.h"
#include "shaders/generated/Release/skybox_ps.h"
#include "shaders/generated/Release/ssao_blur_ps.h"
#include "shaders/generated/Release/ssao_main_ps.h"
#include "shaders/generated/Release/ssao_vs.h"
#include "shaders/generated/Release/ssr_compose_ps.h"
#include "shaders/generated/Release/ssr_ps.h"
#include "shaders/generated/Release/ssr_vs.h"
#include "shaders/generated/Release/vtx_skinning_cs.h"
#endif


namespace sorcery::rendering {
namespace {
// Returns view matrices looking at each face of a cube from the specified origin.
// The order of faces is +X, -X, +Y, -Y, +Z, -Z.
[[nodiscard]] auto MakeCubeFaceViewMatrices(Vector3 const& origin) noexcept {
  return std::array{
    Matrix4::LookTo(origin, Vector3::Right(), Vector3::Up()), // +X
    Matrix4::LookTo(origin, Vector3::Left(), Vector3::Up()), // -X
    Matrix4::LookTo(origin, Vector3::Up(), Vector3::Backward()), // +Y
    Matrix4::LookTo(origin, Vector3::Down(), Vector3::Forward()), // -Y
    Matrix4::LookTo(origin, Vector3::Forward(), Vector3::Up()), // +Z
    Matrix4::LookTo(origin, Vector3::Backward(), Vector3::Up()), // -Z
  };
}
}


auto SceneRenderer::CalculateCameraShadowCascadeBoundaries(CameraData const& cam_data,
                                                           ShadowParams const& shadow_params) ->
  ShadowCascadeBoundaries {
  auto const cam_near{cam_data.near_plane};
  auto const shadow_distance{std::min(cam_data.far_plane, shadow_params.distance)};
  auto const shadowed_frustum_depth{shadow_distance - cam_near};

  ShadowCascadeBoundaries boundaries;

  boundaries[0].nearClip = cam_near;

  for (auto i = 0u; i < shadow_params.cascade_count - 1; i++) {
    boundaries[i + 1].nearClip = cam_near + shadow_params.normalized_cascade_splits[i] * shadowed_frustum_depth;
    boundaries[i].farClip = boundaries[i + 1].nearClip * 1.005f;
  }

  boundaries[shadow_params.cascade_count - 1].farClip = shadow_distance;

  for (int i = shadow_params.cascade_count; i < MAX_CASCADE_COUNT; i++) {
    boundaries[i].nearClip = std::numeric_limits<float>::infinity();
    boundaries[i].farClip = std::numeric_limits<float>::infinity();
  }

  return boundaries;
}


auto SceneRenderer::CullLights(Frustum const& frustum_ws, std::span<LightData const> const lights,
                               std::pmr::vector<unsigned>& visible_light_indices) -> void {
  visible_light_indices.clear();

  for (unsigned light_idx = 0; light_idx < static_cast<unsigned>(lights.size()); light_idx++) {
    switch (auto const light{lights[light_idx]}; light.type) {
      case LightComponent::Type::Directional: {
        visible_light_indices.emplace_back(light_idx);
        break;
      }

      case LightComponent::Type::Spot: {
        auto const light_vertices_ws{
          [light] {
            auto vertices{CalculateSpotLightLocalVertices(light.range, light.outer_angle)};

            for (auto const model_mtx_no_scale{light.local_to_world_mtx_no_scale}; auto& vertex : vertices) {
              vertex = Vector3{Vector4{vertex, 1} * model_mtx_no_scale};
            }

            return vertices;
          }()
        };

        if (frustum_ws.Intersects(AABB::FromVertices(light_vertices_ws))) {
          visible_light_indices.emplace_back(light_idx);
        }

        break;
      }

      case LightComponent::Type::Point: {
        if (BoundingSphere const bounds_ws{Vector3{light.position}, light.range}; frustum_ws.Intersects(bounds_ws)) {
          visible_light_indices.emplace_back(light_idx);
        }
        break;
      }
    }
  }
}


auto SceneRenderer::CullStaticSubmeshInstances(Frustum const& frustum_ws, std::span<MeshData const> const meshes,
                                               std::span<SubmeshData const> const submeshes,
                                               std::span<InstanceData const> const instances,
                                               std::pmr::vector<unsigned>& visible_static_submesh_instance_indices) ->
  void {
  visible_static_submesh_instance_indices.clear();

  for (unsigned i{0}; i < static_cast<unsigned>(instances.size()); i++) {
    auto const& instance{instances[i]};
    auto const& submesh{submeshes[instance.submesh_local_idx]};
    auto const& mesh{meshes[submesh.mesh_local_idx]};

    if (frustum_ws.Intersects(mesh.bounds.Transform(instance.local_to_world_mtx)) && frustum_ws.Intersects(
          submesh.bounds.Transform(instance.local_to_world_mtx))) {
      visible_static_submesh_instance_indices.emplace_back(i);
    }
  }
}


auto SceneRenderer::SetPerFrameConstants(ConstantBuffer<ShaderPerFrameConstants>& cb, int const rt_width,
                                         int const rt_height, Vector3 const& ambient_light,
                                         ShadowParams const& shadow_params) -> void {
  cb.Update(ShaderPerFrameConstants{
    .ambientLightColor = ambient_light, .shadowCascadeCount = shadow_params.cascade_count,
    .screenSize = Vector2{rt_width, rt_height}, .visualizeShadowCascades = shadow_params.visualize_cascades,
    .shadowFilteringMode = static_cast<int>(shadow_params.filtering_mode)
  });
}


auto SceneRenderer::SetPerViewConstants(ConstantBuffer<ShaderPerViewConstants>& cb, Matrix4 const& view_mtx,
                                        Matrix4 const& proj_mtx, ShadowCascadeBoundaries const& cascade_bounds,
                                        Vector3 const& view_pos) -> void {
  ShaderPerViewConstants data;
  data.viewMtx = view_mtx;
  data.invViewMtx = view_mtx.Inverse();
  data.projMtx = proj_mtx;
  data.invProjMtx = proj_mtx.Inverse();
  data.viewProjMtx = view_mtx * proj_mtx;
  data.invViewProjMtx = data.viewProjMtx.Inverse();
  data.viewPos = view_pos;

  for (auto i = 0; i < MAX_CASCADE_COUNT; i++) {
    data.shadowCascadeSplitDistances[i] = cascade_bounds[i].farClip;
  }

  cb.Update(data);
}


auto SceneRenderer::SetPerDrawConstants(ConstantBuffer<ShaderPerDrawConstants>& cb, Matrix4 const& model_mtx,
                                        Matrix4 const& view_mtx, Matrix4 const& proj_mtx) -> void {
  cb.Update(ShaderPerDrawConstants{
    .modelMtx = model_mtx, .invTranspModelMtx = model_mtx.Inverse().Transpose(), .model_view_mtx = model_mtx * view_mtx,
    .model_view_proj_mtx = model_mtx * view_mtx * proj_mtx
  });
}


auto SceneRenderer::UpdatePunctualShadowAtlas(PunctualShadowAtlas& atlas,
                                              std::span<SceneRenderer::LightData const> const lights,
                                              std::span<unsigned const> visible_light_indices,
                                              SceneRenderer::CameraData const& cam_data,
                                              Matrix4 const& cam_view_proj_mtx, float const shadow_distance) -> void {
  struct LightCascadeIndex {
    int lightIdxIdx;
    int shadowIdx;
  };

  std::array lightIndexIndicesInCell{
    std::vector<LightCascadeIndex>{}, std::vector<LightCascadeIndex>{},
    std::vector<LightCascadeIndex>{}, std::vector<LightCascadeIndex>{}
  };

  auto const& camPos{cam_data.position};

  auto const determineScreenCoverage{
    [&camPos, &cam_view_proj_mtx](std::span<Vector3 const> const vertices) -> std::optional<int> {
      std::optional<int> cellIdx;

      if (auto const [worldMin, worldMax]{AABB::FromVertices(vertices)};
        worldMin[0] <= camPos[0] && worldMin[1] <= camPos[1] && worldMin[2] <= camPos[2] && worldMax[0] >= camPos[0] &&
        worldMax[1] >= camPos[1] && worldMax[2] >= camPos[2]) {
        cellIdx = 0;
      } else {
        Vector2 const bottomLeft{-1, -1};
        Vector2 const topRight{1, 1};

        Vector2 min{std::numeric_limits<float>::max()};
        Vector2 max{std::numeric_limits<float>::lowest()};

        for (auto& vertex : vertices) {
          Vector4 vertex4{vertex, 1};
          vertex4 *= cam_view_proj_mtx;
          auto const projected{Vector2{vertex4} / vertex4[3]};
          min = Clamp(Min(min, projected), bottomLeft, topRight);
          max = Clamp(Max(max, projected), bottomLeft, topRight);
        }

        auto const width{max[0] - min[0]};
        auto const height{max[1] - min[1]};

        auto const area{width * height};
        auto const coverage{area / 4};

        if (coverage >= 1) {
          cellIdx = 0;
        } else if (coverage >= 0.25f) {
          cellIdx = 1;
        } else if (coverage >= 0.0625f) {
          cellIdx = 2;
        } else if (coverage >= 0.015625f) {
          cellIdx = 3;
        }
      }

      return cellIdx;
    }
  };

  for (auto i = 0; i < static_cast<int>(visible_light_indices.size()); i++) {
    if (auto const light{lights[visible_light_indices[i]]};
      light.casts_shadow && (light.type == LightComponent::Type::Spot || light.type == LightComponent::Type::Point)) {
      Vector3 const& lightPos{light.position};
      float const lightRange{light.range};

      // Skip the light if its bounding sphere is farther than the shadow distance
      if (Vector3 const camToLightDir{Normalize(lightPos - camPos)}; Distance(lightPos - camToLightDir * lightRange,
                                                                       cam_data.position) > shadow_distance) {
        continue;
      }

      if (light.type == LightComponent::Type::Spot) {
        auto lightVertices{CalculateSpotLightLocalVertices(light.range, light.outer_angle)};

        for (auto const modelMtxNoScale{light.local_to_world_mtx_no_scale}; auto& vertex : lightVertices) {
          vertex = Vector3{Vector4{vertex, 1} * modelMtxNoScale};
        }

        if (auto const cellIdx{determineScreenCoverage(lightVertices)}) {
          lightIndexIndicesInCell[*cellIdx].emplace_back(i, 0);
        }
      } else if (light.type == LightComponent::Type::Point) {
        for (auto j = 0; j < 6; j++) {
          std::array static const faceBoundsRotations{
            Quaternion::FromAxisAngle(Vector3::Up(), ToRadians(90)), // +X
            Quaternion::FromAxisAngle(Vector3::Up(), ToRadians(-90)), // -X
            Quaternion::FromAxisAngle(Vector3::Right(), ToRadians(-90)), // +Y
            Quaternion::FromAxisAngle(Vector3::Right(), ToRadians(90)), // -Y
            Quaternion{}, // +Z
            Quaternion::FromAxisAngle(Vector3::Up(), ToRadians(180)) // -Z
          };

          std::array const shadowFrustumVertices{
            faceBoundsRotations[i].Rotate(Vector3{lightRange, lightRange, lightRange}) + lightPos,
            faceBoundsRotations[i].Rotate(Vector3{-lightRange, lightRange, lightRange}) + lightPos,
            faceBoundsRotations[i].Rotate(Vector3{-lightRange, -lightRange, lightRange}) + lightPos,
            faceBoundsRotations[i].Rotate(Vector3{lightRange, -lightRange, lightRange}) + lightPos, lightPos,
          };

          if (auto const cellIdx{determineScreenCoverage(shadowFrustumVertices)}) {
            lightIndexIndicesInCell[*cellIdx].emplace_back(i, j);
          }
        }
      }
    }
  }

  for (auto i = 0; i < 4; i++) {
    std::ranges::sort(lightIndexIndicesInCell[i],
      [&visible_light_indices, &camPos, &lights](LightCascadeIndex const lhs, LightCascadeIndex const rhs) {
        auto const leftLight{lights[visible_light_indices[lhs.lightIdxIdx]]};
        auto const rightLight{lights[visible_light_indices[rhs.lightIdxIdx]]};

        auto const leftLightPos{leftLight.position};
        auto const rightLightPos{rightLight.position};

        auto const leftDist{Distance(leftLightPos, camPos)};
        auto const rightDist{Distance(camPos, rightLightPos)};

        return leftDist > rightDist;
      });

    for (auto j = 0; j < atlas.GetCell(i).GetElementCount(); j++) {
      auto& subcell{atlas.GetCell(i).GetSubcell(j)};
      subcell.reset();

      if (lightIndexIndicesInCell[i].empty()) {
        continue;
      }

      auto const [lightIdxIdx, shadowIdx]{lightIndexIndicesInCell[i].back()};
      auto const light{lights[visible_light_indices[lightIdxIdx]]};
      lightIndexIndicesInCell[i].pop_back();

      if (light.type == LightComponent::Type::Spot) {
        auto const shadowViewMtx{Matrix4::LookTo(light.position, light.direction, Vector3::Up())};
        auto const shadowProjMtx{
          Matrix4::PerspectiveFov(ToRadians(light.outer_angle), 1.f, light.range, light.shadow_near_plane)
        };

        subcell.emplace(shadowViewMtx * shadowProjMtx, lightIdxIdx, shadowIdx);
      } else if (light.type == LightComponent::Type::Point) {
        auto const lightPos{light.position};

        auto const faceViewMatrices{MakeCubeFaceViewMatrices(lightPos)};
        auto const shadowViewMtx{faceViewMatrices[shadowIdx]};
        auto const shadowProjMtx{
          TransformProjectionMatrixForRendering(Matrix4::PerspectiveFov(ToRadians(90), 1, light.shadow_near_plane,
            light.range))
        };

        subcell.emplace(shadowViewMtx * shadowProjMtx, lightIdxIdx, shadowIdx);
      }
    }

    if (i + 1 < 4) {
      std::ranges::copy(lightIndexIndicesInCell[i], std::back_inserter(lightIndexIndicesInCell[i + 1]));
    }
  }
}


auto SceneRenderer::DrawDirectionalShadowMaps(FramePacket const& frame_packet,
                                              std::span<unsigned const> const visible_light_indices,
                                              CameraData const& cam_data, float rt_aspect, int const cascade_count,
                                              ShadowCascadeBoundaries const& shadow_cascade_boundaries,
                                              std::array<Matrix4, MAX_CASCADE_COUNT>& shadow_view_proj_matrices,
                                              graphics::CommandList& cmd) -> void {
  cmd.SetPipelineState(*shadow_pso_);
  cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, samp_idx), samp_af16_wrap_.Get());
  cmd.SetRenderTargets({}, dir_shadow_map_arr_->GetTex().get());
  cmd.ClearDepthStencil(*dir_shadow_map_arr_->GetTex(), D3D12_CLEAR_FLAG_DEPTH, DEPTH_CLEAR_VALUE, 0, {});

  for (auto const lightIdx : visible_light_indices) {
    if (auto const light{frame_packet.light_data[lightIdx]};
      light.type == LightComponent::Type::Directional && light.casts_shadow) {
      float const camNear{cam_data.near_plane};
      float const camFar{cam_data.far_plane};

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
        [&cam_data, rt_aspect, camNear, camFar] {
          std::array<Vector3, 8> ret;

          Vector3 const nearWorldForward{cam_data.position + cam_data.forward * camNear};
          Vector3 const farWorldForward{cam_data.position + cam_data.forward * camFar};

          switch (cam_data.type) {
            case Camera::Type::Perspective: {
              float const tanHalfFov{std::tan(ToRadians(cam_data.fov_vert_deg / 2.0f))};
              float const nearExtentY{camNear * tanHalfFov};
              float const nearExtentX{nearExtentY * rt_aspect};
              float const farExtentY{camFar * tanHalfFov};
              float const farExtentX{farExtentY * rt_aspect};

              ret[FrustumVertex_NearTopRight] = nearWorldForward + cam_data.right * nearExtentX + cam_data.up *
                                                nearExtentY;
              ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam_data.right * nearExtentX + cam_data.up *
                                               nearExtentY;
              ret[FrustumVertex_NearBottomLeft] =
                nearWorldForward - cam_data.right * nearExtentX - cam_data.up * nearExtentY;
              ret[FrustumVertex_NearBottomRight] =
                nearWorldForward + cam_data.right * nearExtentX - cam_data.up * nearExtentY;
              ret[FrustumVertex_FarTopRight] = farWorldForward + cam_data.right * farExtentX + cam_data.up * farExtentY;
              ret[FrustumVertex_FarTopLeft] = farWorldForward - cam_data.right * farExtentX + cam_data.up * farExtentY;
              ret[FrustumVertex_FarBottomLeft] =
                farWorldForward - cam_data.right * farExtentX - cam_data.up * farExtentY;
              ret[FrustumVertex_FarBottomRight] =
                farWorldForward + cam_data.right * farExtentX - cam_data.up * farExtentY;
              break;
            }
            case Camera::Type::Orthographic: {
              float const extentX{cam_data.size_vert / 2.0f};
              float const extentY{extentX / rt_aspect};

              ret[FrustumVertex_NearTopRight] = nearWorldForward + cam_data.right * extentX + cam_data.up * extentY;
              ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam_data.right * extentX + cam_data.up * extentY;
              ret[FrustumVertex_NearBottomLeft] = nearWorldForward - cam_data.right * extentX - cam_data.up * extentY;
              ret[FrustumVertex_NearBottomRight] = nearWorldForward + cam_data.right * extentX - cam_data.up * extentY;
              ret[FrustumVertex_FarTopRight] = farWorldForward + cam_data.right * extentX + cam_data.up * extentY;
              ret[FrustumVertex_FarTopLeft] = farWorldForward - cam_data.right * extentX + cam_data.up * extentY;
              ret[FrustumVertex_FarBottomLeft] = farWorldForward - cam_data.right * extentX - cam_data.up * extentY;
              ret[FrustumVertex_FarBottomRight] = farWorldForward + cam_data.right * extentX - cam_data.up * extentY;
              break;
            }
          }

          return ret;
        }()
      };

      auto const frustumDepth{camFar - camNear};

      for (auto cascadeIdx{0}; cascadeIdx < cascade_count; cascadeIdx++) {
        // cascade vertices in world space
        auto const cascadeVertsWS{
          [&frustumVertsWS, &shadow_cascade_boundaries, cascadeIdx, camNear, frustumDepth] {
            auto const [cascadeNear, cascadeFar]{shadow_cascade_boundaries[cascadeIdx]};

            float const cascadeNearNorm{(cascadeNear - camNear) / frustumDepth};
            float const cascadeFarNorm{(cascadeFar - camNear) / frustumDepth};

            std::array<Vector3, 8> ret;

            for (auto j = 0; j < 4; j++) {
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

        auto sphereRadius{0.0f};

        for (Vector3 const& cascadeVertWS : cascadeVertsWS) {
          sphereRadius = std::max(sphereRadius, Distance(cascadeCenterWS, cascadeVertWS));
        }

        auto const shadowMapSize{dir_shadow_map_arr_->GetSize()};
        auto const worldUnitsPerTexel{sphereRadius * 2.0f / static_cast<float>(shadowMapSize)};

        Matrix4 shadowViewMtx{Matrix4::LookTo(Vector3::Zero(), light.direction, Vector3::Up())};
        cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx};
        cascadeCenterWS /= worldUnitsPerTexel;
        cascadeCenterWS[0] = std::floor(cascadeCenterWS[0]);
        cascadeCenterWS[1] = std::floor(cascadeCenterWS[1]);
        cascadeCenterWS *= worldUnitsPerTexel;
        // shadowViewMtx is only rotation, transpose is its inverse
        cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx.Transpose()};

        shadowViewMtx = Matrix4::LookTo(cascadeCenterWS, light.direction, Vector3::Up());
        auto const shadowProjMtx{
          TransformProjectionMatrixForRendering(Matrix4::OrthographicOffCenter(-sphereRadius, sphereRadius,
            sphereRadius, -sphereRadius, -sphereRadius - light.shadow_extension, sphereRadius))
        };

        shadow_view_proj_matrices[cascadeIdx] = shadowViewMtx * shadowProjMtx;

        cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, rt_idx), cascadeIdx);

        D3D12_VIEWPORT const shadowViewport{
          0, 0, static_cast<float>(shadowMapSize), static_cast<float>(shadowMapSize), 0, 1
        };

        D3D12_RECT const shadow_scissor{0, 0, static_cast<LONG>(shadowMapSize), static_cast<LONG>(shadowMapSize)};

        cmd.SetViewports(std::array{shadowViewport});
        cmd.SetScissorRects(std::array{shadow_scissor});

        auto& per_view_cb{AcquirePerViewConstantBuffer()};
        SetPerViewConstants(per_view_cb, shadowViewMtx, shadowProjMtx, ShadowCascadeBoundaries{}, Vector3{});
        cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_view_cb_idx), *per_view_cb.GetBuffer());

        Frustum const shadow_frustum_ws{shadow_view_proj_matrices[cascadeIdx]};

        std::pmr::vector<unsigned> visible_static_submesh_instance_indices;
        CullStaticSubmeshInstances(shadow_frustum_ws, frame_packet.mesh_data, frame_packet.submesh_data,
          frame_packet.instance_data, visible_static_submesh_instance_indices);

        for (auto const instance_idx : visible_static_submesh_instance_indices) {
          auto const& instance{frame_packet.instance_data[instance_idx]};
          auto const& submesh{frame_packet.submesh_data[instance.submesh_local_idx]};
          auto const& mesh{frame_packet.mesh_data[submesh.mesh_local_idx]};
          auto const& mtl_buf{frame_packet.buffers[submesh.mtl_buf_local_idx]};

          auto& per_draw_cb{AcquirePerDrawConstantBuffer()};
          SetPerDrawConstants(per_draw_cb, instance.local_to_world_mtx, shadowViewMtx, shadowProjMtx);

          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, pos_buf_idx),
            *frame_packet.buffers[mesh.pos_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, uv_buf_idx),
            *frame_packet.buffers[mesh.uv_buf_local_idx]);
          cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, mtl_idx), *mtl_buf);
          cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_draw_cb_idx),
            *per_draw_cb.GetBuffer());
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, vertex_idx_buf_idx),
            *frame_packet.buffers[mesh.vtx_idx_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, prim_idx_buf_idx),
            *frame_packet.buffers[mesh.prim_idx_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_buf_idx),
            *frame_packet.buffers[mesh.meshlet_buf_local_idx]);
          cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, idx32), mesh.idx32);

          DrawSubmesh(submesh, PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_count),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_offset),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, instance_count),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, instance_offset),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, base_vertex), cmd);
        }
      }

      break;
    }
  }
}


auto SceneRenderer::DrawPunctualShadowMaps(PunctualShadowAtlas const& atlas,
                                           SceneRenderer::FramePacket const& frame_packet,
                                           graphics::CommandList& cmd) -> void {
  cmd.SetPipelineState(*shadow_pso_);
  cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, rt_idx), 0);
  cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, samp_idx), samp_af16_wrap_.Get());
  cmd.SetRenderTargets({}, atlas.GetTex().get());
  cmd.ClearDepthStencil(*atlas.GetTex(), D3D12_CLEAR_FLAG_DEPTH, DEPTH_CLEAR_VALUE, 0, {});

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
        D3D12_RECT const scissor{
          static_cast<LONG>(subcell_offset[0]), static_cast<LONG>(subcell_offset[1]),
          static_cast<LONG>(subcell_offset[0] + subcell_size), static_cast<LONG>(subcell_offset[1] + subcell_size)
        };

        cmd.SetViewports(std::span{&viewport, 1});
        cmd.SetScissorRects(std::array{scissor});

        auto& per_view_cb{AcquirePerViewConstantBuffer()};
        SetPerViewConstants(per_view_cb, Matrix4::Identity(), subcell->shadowViewProjMtx, ShadowCascadeBoundaries{},
          Vector3{});
        cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_view_cb_idx), *per_view_cb.GetBuffer());

        Frustum const shadow_frustum_ws{subcell->shadowViewProjMtx};

        std::pmr::vector<unsigned> visible_static_submesh_instance_indices;
        CullStaticSubmeshInstances(shadow_frustum_ws, frame_packet.mesh_data, frame_packet.submesh_data,
          frame_packet.instance_data, visible_static_submesh_instance_indices);

        for (auto const instance_idx : visible_static_submesh_instance_indices) {
          auto const& instance{frame_packet.instance_data[instance_idx]};
          auto const& submesh{frame_packet.submesh_data[instance.submesh_local_idx]};
          auto const& mesh{frame_packet.mesh_data[submesh.mesh_local_idx]};
          auto const& mtl_buf{frame_packet.buffers[submesh.mtl_buf_local_idx]};

          auto& per_draw_cb{AcquirePerDrawConstantBuffer()};
          SetPerDrawConstants(per_draw_cb, instance.local_to_world_mtx, Matrix4::Identity(),
            subcell->shadowViewProjMtx);

          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, pos_buf_idx),
            *frame_packet.buffers[mesh.pos_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, uv_buf_idx),
            *frame_packet.buffers[mesh.uv_buf_local_idx]);
          cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, mtl_idx), *mtl_buf);
          cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_draw_cb_idx),
            *per_draw_cb.GetBuffer());
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, vertex_idx_buf_idx),
            *frame_packet.buffers[mesh.vtx_idx_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, prim_idx_buf_idx),
            *frame_packet.buffers[mesh.prim_idx_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_buf_idx),
            *frame_packet.buffers[mesh.meshlet_buf_local_idx]);
          cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, idx32), mesh.idx32);

          DrawSubmesh(submesh, PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_count),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_offset),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, instance_count),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, instance_offset),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, base_vertex), cmd);
        }
      }
    }
  }
}


auto SceneRenderer::ClearGizmoDrawQueue() noexcept -> void {
  gizmo_colors_.clear();
  line_gizmo_vertex_data_.clear();
}


auto SceneRenderer::RecreateSsaoSamples(int const sample_count) noexcept -> void {
  ssao_samples_buffer_.Resize(sample_count);
  auto const ssao_samples{ssao_samples_buffer_.GetData()};

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


auto SceneRenderer::RecreatePipelines() -> void {
  CD3DX12_DEPTH_STENCIL_DESC1 const depth_stencil_write{
    TRUE, D3D12_DEPTH_WRITE_MASK_ALL,
#ifdef REVERSE_Z
    D3D12_COMPARISON_FUNC_GREATER_EQUAL,
#else
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
#endif
    FALSE, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, FALSE
  };

  CD3DX12_DEPTH_STENCIL_DESC1 const depth_stencil_read_not_equal{
    TRUE, D3D12_DEPTH_WRITE_MASK_ZERO, D3D12_COMPARISON_FUNC_NOT_EQUAL, FALSE, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    FALSE
  };

  CD3DX12_DEPTH_STENCIL_DESC1 const depth_stencil_disabled{
    FALSE, D3D12_DEPTH_WRITE_MASK_ZERO, D3D12_COMPARISON_FUNC_NONE, FALSE, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, FALSE
  };

  auto constexpr depth_bias_multiplier{
#ifdef REVERSE_Z
    -1
#else
    1
#endif
  };

  CD3DX12_RASTERIZER_DESC const shadow_rasterizer_desc{
    D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, FALSE, depth_bias_multiplier * 1, 0.f, depth_bias_multiplier * 2.5f,
    TRUE, FALSE, FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
  };

  CD3DX12_RASTERIZER_DESC const skybox_rasterizer_desc{
    D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_FRONT, FALSE, D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
    D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, TRUE, FALSE, 0, {}
  };

  CD3DX12_RT_FORMAT_ARRAY const render_target_format{
    D3D12_RT_FORMAT_ARRAY{.RTFormats = {render_target_format_}, .NumRenderTargets = 1}
  };
  CD3DX12_RT_FORMAT_ARRAY const color_format{
    D3D12_RT_FORMAT_ARRAY{.RTFormats = {color_buffer_format_}, .NumRenderTargets = 1}
  };
  CD3DX12_RT_FORMAT_ARRAY const ssao_format{
    D3D12_RT_FORMAT_ARRAY{.RTFormats = {ssao_buffer_format_}, .NumRenderTargets = 1}
  };
  CD3DX12_RT_FORMAT_ARRAY const gbuffer_format{
    D3D12_RT_FORMAT_ARRAY{.RTFormats = {gbuffer0_format_, gbuffer1_format_, gbuffer2_format_}, .NumRenderTargets = 3}
  };

  graphics::PipelineDesc const shadow_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{g_depth_only_ps_bytes, ARRAYSIZE(g_depth_only_ps_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{g_depth_only_ms_bytes, ARRAYSIZE(g_depth_only_ms_bytes)},
    .depth_stencil_state = depth_stencil_write, .ds_format = depth_format_,
    .rasterizer_state = shadow_rasterizer_desc
  };

  shadow_pso_ = device_->CreatePipelineState(shadow_pso_desc, sizeof(DepthOnlyDrawParams) / 4);

  graphics::PipelineDesc const depth_resolve_pso_desc{
    .cs = CD3DX12_SHADER_BYTECODE{g_depth_resolve_cs_bytes, ARRAYSIZE(g_depth_resolve_cs_bytes)}
  };

  depth_resolve_pso_ = device_->CreatePipelineState(depth_resolve_pso_desc, sizeof(DepthResolveDrawParams) / 4);

  graphics::PipelineDesc const line_gizmo_pso_desc{
    .primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
    .vs = CD3DX12_SHADER_BYTECODE{g_gizmos_line_vs_bytes, ARRAYSIZE(g_gizmos_line_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{g_gizmos_ps_bytes, ARRAYSIZE(g_gizmos_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = render_target_format
  };

  line_gizmo_pso_ = device_->CreatePipelineState(line_gizmo_pso_desc, sizeof(GizmoDrawParams) / 4);

  graphics::PipelineDesc const gbuffer_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{g_gbuffer_ps_bytes, ARRAYSIZE(g_gbuffer_ps_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{g_gbuffer_ms_bytes, ARRAYSIZE(g_gbuffer_ms_bytes)},
    .depth_stencil_state = depth_stencil_write, .ds_format = depth_format_,
    .rt_formats = gbuffer_format
  };

  gbuffer_pso_ = device_->CreatePipelineState(gbuffer_pso_desc, sizeof(GBufferDrawParams) / 4);

  graphics::PipelineDesc const deferred_lighting_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{g_deferred_lighting_vs_bytes, ARRAYSIZE(g_deferred_lighting_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{g_deferred_lighting_ps_bytes, ARRAYSIZE(g_deferred_lighting_ps_bytes)},
    .depth_stencil_state = depth_stencil_read_not_equal, .ds_format = depth_format_, .rt_formats = color_format
  };

  deferred_lighting_pso_ = device_->CreatePipelineState(deferred_lighting_pso_desc,
    sizeof(DeferredLightingDrawParams) / 4);

  graphics::PipelineDesc const post_process_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_post_process_vs_bytes, ARRAYSIZE(g_post_process_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_post_process_ps_bytes, ARRAYSIZE(g_post_process_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = render_target_format,
  };

  post_process_pso_ = device_->CreatePipelineState(post_process_pso_desc, sizeof(PostProcessDrawParams) / 4);

  graphics::PipelineDesc const skybox_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{&g_skybox_ps_bytes, ARRAYSIZE(g_skybox_ps_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{&g_skybox_ms_bytes, ARRAYSIZE(g_skybox_ms_bytes)},
    .depth_stencil_state = depth_stencil_write, .ds_format = depth_format_, .rasterizer_state = skybox_rasterizer_desc,
    .rt_formats = color_format
  };

  skybox_pso_ = device_->CreatePipelineState(skybox_pso_desc, sizeof(SkyboxDrawParams) / 4);

  graphics::PipelineDesc const ssao_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssao_main_ps_bytes, ARRAYSIZE(g_ssao_main_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = ssao_format,
  };

  ssao_pso_ = device_->CreatePipelineState(ssao_pso_desc, sizeof(SsaoDrawParams) / 4);

  graphics::PipelineDesc const ssao_blur_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssao_blur_ps_bytes, ARRAYSIZE(g_ssao_blur_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = ssao_format
  };

  ssao_blur_pso_ = device_->CreatePipelineState(ssao_blur_pso_desc, sizeof(SsaoBlurDrawParams) / 4);

  graphics::PipelineDesc const ssr_compose_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssr_vs_bytes, ARRAYSIZE(g_ssr_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssr_compose_ps_bytes, ARRAYSIZE(g_ssr_compose_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = color_format
  };

  ssr_compose_pso_ = device_->CreatePipelineState(ssr_compose_pso_desc, sizeof(SsrComposeDrawParams) / 4);

  graphics::PipelineDesc const ssr_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssr_vs_bytes, ARRAYSIZE(g_ssr_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssr_ps_bytes, ARRAYSIZE(g_ssr_ps_bytes)},
    .depth_stencil_state = depth_stencil_read_not_equal, .ds_format = depth_format_,
    .rt_formats = color_format
  };

  ssr_pso_ = device_->CreatePipelineState(ssr_pso_desc, sizeof(SsrDrawParams) / 4);

  graphics::PipelineDesc const vtx_skinning_pso_desc{
    .cs = CD3DX12_SHADER_BYTECODE{g_vtx_skinning_cs_bytes, ARRAYSIZE(g_vtx_skinning_cs_bytes)}
  };

  vtx_skinning_pso_ = device_->CreatePipelineState(vtx_skinning_pso_desc, sizeof(VertexSkinningDrawParams) / 4);
}


auto SceneRenderer::CreatePerViewConstantBuffers(UINT const count) -> void {
  per_view_cbs_.reserve(per_view_cbs_.size() + count);

  for (UINT i{0}; i < count; i++) {
    auto& arr{per_view_cbs_.emplace_back()};

    for (UINT j{0}; j < RenderManager::GetMaxFramesInFlight(); j++) {
      if (auto opt{ConstantBuffer<ShaderPerViewConstants>::New(*device_, true)}) {
        arr[j] = std::move(*opt);
      }
    }
  }
}


auto SceneRenderer::CreatePerDrawConstantBuffers(UINT const count) -> void {
  per_draw_cbs_.reserve(per_draw_cbs_.size() + count);

  for (UINT i{0}; i < count; i++) {
    auto& arr{per_draw_cbs_.emplace_back()};

    for (UINT j{0}; j < RenderManager::GetMaxFramesInFlight(); j++) {
      if (auto opt{ConstantBuffer<ShaderPerDrawConstants>::New(*device_, true)}) {
        arr[j] = std::move(*opt);
      }
    }
  }
}


auto SceneRenderer::AcquirePerViewConstantBuffer() -> ConstantBuffer<ShaderPerViewConstants>& {
  if (next_per_view_cb_idx_ >= per_view_cbs_.size()) {
    CreatePerViewConstantBuffers(1);
  }

  return per_view_cbs_[next_per_view_cb_idx_++][render_manager_->GetCurrentFrameIndex()];
}


auto SceneRenderer::AcquirePerDrawConstantBuffer() -> ConstantBuffer<ShaderPerDrawConstants>& {
  if (next_per_draw_cb_idx_ >= per_draw_cbs_.size()) {
    CreatePerDrawConstantBuffers(1);
  }

  return per_draw_cbs_[next_per_draw_cb_idx_++][render_manager_->GetCurrentFrameIndex()];
}


auto SceneRenderer::OnWindowSize(Extent2D<std::uint32_t> const size) -> void {
  if (size.width != 0 && size.height != 0) {
    RenderTarget::Desc desc{main_rt_->GetDesc()};
    desc.width = size.width;
    desc.height = size.height;
    main_rt_ = RenderTarget::New(*device_, desc);
  }
}


auto SceneRenderer::DrawSubmesh(SubmeshData const& submesh, std::optional<UINT> const meshlet_count_param_idx,
                                std::optional<UINT> const meshlet_offset_param_idx,
                                std::optional<UINT> const instance_count_param_idx,
                                std::optional<UINT> const instance_offset_param_idx,
                                std::optional<UINT> const base_vertex_param_idx,
                                graphics::CommandList const& cmd) -> void {
  DrawSubmesh(submesh.meshlet_count, submesh.first_meshlet, submesh.base_vertex, meshlet_count_param_idx,
    meshlet_offset_param_idx, instance_count_param_idx, instance_offset_param_idx, base_vertex_param_idx, cmd);
}


auto SceneRenderer::DrawSubmesh(UINT const submesh_meshlet_count, UINT const submesh_meshlet_offset,
                                UINT submesh_base_vertex,
                                std::optional<UINT> const meshlet_count_param_idx,
                                std::optional<UINT> const meshlet_offset_param_idx,
                                std::optional<UINT> const instance_count_param_idx,
                                std::optional<UINT> const instance_offset_param_idx,
                                std::optional<UINT> const base_vertex_param_idx,
                                graphics::CommandList const& cmd) -> void {
  UINT constexpr max_dispatch_thread_group_count{65535};

  auto const submesh_meshlet_end{submesh_meshlet_offset + submesh_meshlet_count};

  for (auto meshlet_offset{submesh_meshlet_offset}; meshlet_offset < submesh_meshlet_end;
       meshlet_offset += max_dispatch_thread_group_count) {
    auto const meshlet_count{
      std::min(submesh_meshlet_end - meshlet_offset, max_dispatch_thread_group_count)
    };

    if (meshlet_count_param_idx) {
      cmd.SetPipelineParameter(*meshlet_count_param_idx, meshlet_count);
    }

    if (meshlet_offset_param_idx) {
      cmd.SetPipelineParameter(*meshlet_offset_param_idx, meshlet_offset);
    }

    if (base_vertex_param_idx) {
      cmd.SetPipelineParameter(*base_vertex_param_idx, submesh_base_vertex);
    }

    // Parts commented out are needed for instancing

    /* auto const& last_meshlet{
       mesh.meshlets[meshlet_offset + meshlet_count - 1]
     };*/

    auto const pack_count{
      /*std::min(MESHLET_MAX_VERTS / last_meshlet.vert_count,
        MESHLET_MAX_PRIMS / last_meshlet.prim_count)*/
      1u
    };

    auto const group_count_per_instance{
      static_cast<float>(meshlet_count - 1) + 1.0f / static_cast<float>(
        pack_count)
    };

    auto const max_instance_count_per_batch{
      static_cast<std::uint32_t>(static_cast<float>(
                                   max_dispatch_thread_group_count) / group_count_per_instance)
    };

    auto const dispatch_count{
      //DivRoundUp(mesh.instance_count, max_instance_count_per_batch)
      DivRoundUp(1u, max_instance_count_per_batch)
    };

    for (std::size_t i{0}; i < dispatch_count; i++) {
      auto const batch_instance_offset{
        static_cast<UINT>(i * max_instance_count_per_batch)
      };

      auto const batch_instance_count{
        /*std::min(mesh.instance_count - batch_instance_offset,
          max_instance_count_per_batch)*/
        std::min(1 - batch_instance_offset,
          max_instance_count_per_batch)
      };

      if (instance_count_param_idx) {
        cmd.SetPipelineParameter(*instance_count_param_idx, batch_instance_count);
      }

      if (instance_offset_param_idx) {
        cmd.SetPipelineParameter(*instance_offset_param_idx, batch_instance_offset);
      }

      auto const group_count{
        static_cast<std::uint32_t>(std::ceilf(
          group_count_per_instance * batch_instance_count))
      };

      cmd.DispatchMesh(group_count, 1, 1);
    }
  }
}


SceneRenderer::SceneRenderer(Window& window, graphics::GraphicsDevice& device, RenderManager& render_manager) :
  render_manager_{&render_manager},
  window_{&window},
  device_{&device} {
  light_buffer_ = StructuredBuffer<ShaderLight>::New(*device_, *render_manager_, false, true, false);

  gizmo_color_buffer_ = StructuredBuffer<Vector4>::New(*device_, *render_manager_, true);

  line_gizmo_vertex_data_buffer_ = StructuredBuffer<ShaderLineGizmoVertexData>::New(*device_, *render_manager_, true);

  main_rt_ = RenderTarget::New(*device_, RenderTarget::Desc{
    static_cast<UINT>(window_->GetClientAreaSize().width), static_cast<UINT>(window_->GetClientAreaSize().height),
    DXGI_FORMAT_R8G8B8A8_UNORM, std::nullopt, 1, L"Main RT", false
  });

  dir_shadow_map_arr_ = std::make_unique<DirectionalShadowMapArray>(device_.Get(), depth_format_, 4096);
  dir_shadow_map_arr_->GetTex()->SetDebugName(L"Directional Shadow Map Array");

  punctual_shadow_atlas_ = std::make_unique<PunctualShadowAtlas>(device_.Get(), depth_format_, 4096);
  punctual_shadow_atlas_->GetTex()->SetDebugName(L"Punctual Shadow Atlas");

  RecreatePipelines();

  for (auto& cb : per_frame_cbs_) {
    if (auto opt{ConstantBuffer<ShaderPerFrameConstants>::New(*device_, true)}) {
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
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 16, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af8_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 8, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af4_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 4, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af2_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 2, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_tri_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_bi_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_point_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af16_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 16, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af8_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 8, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af4_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 4, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_af2_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 2, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_tri_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 1, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_bi_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 1, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  samp_point_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 1, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  window_size_event_listener_ = window_->OnWindowSize.add_listener([this](Extent2D<unsigned> const size) {
    OnWindowSize(size);
  });

  ssao_samples_buffer_ = StructuredBuffer<Vector4>::New(*device_, *render_manager_, true);
  RecreateSsaoSamples(ssao_params_.sample_count);

  ssao_noise_tex_ = device_->CreateTexture(graphics::TextureDesc{
    graphics::TextureDimension::k2D, SSAO_NOISE_TEX_DIM, SSAO_NOISE_TEX_DIM, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
    false, false, true, false
  }, graphics::CpuAccess::kNone, nullptr);
  ssao_noise_tex_->SetDebugName(L"SSAO Noise");

  std::vector<Vector4> ssao_noise;
  std::uniform_real_distribution dist{0.0f, 1.0f};
  std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

  for (auto i{0}; i < SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM; i++) {
    ssao_noise.emplace_back(dist(gen) * 2 - 1, dist(gen) * 2 - 1, 0, 0);
  }

  render_manager_->UpdateTexture(*ssao_noise_tex_, 0, std::array{
    D3D12_SUBRESOURCE_DATA{
      ssao_noise.data(), SSAO_NOISE_TEX_DIM * sizeof(Vector4), SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM * sizeof(Vector4)
    }
  });


  white_tex_ = device_->CreateTexture(graphics::TextureDesc{
    graphics::TextureDimension::k2D, 1, 1, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1, false, false, true, false
  }, graphics::CpuAccess::kNone, nullptr);

  std::array<std::uint8_t, 4> constexpr white_tex_data{255, 255, 255, 255};

  render_manager_->UpdateTexture(*white_tex_, 0, std::array{
    D3D12_SUBRESOURCE_DATA{white_tex_data.data(), sizeof(white_tex_data), sizeof(white_tex_data)}
  });
}


SceneRenderer::~SceneRenderer() {
  window_->OnWindowSize.remove_listener(window_size_event_listener_);
}


auto SceneRenderer::ExtractCurrentState() -> void {
  auto& packet{frame_packets_[render_manager_->GetCurrentFrameIndex()]};

  packet.buffers.clear();
  packet.textures.clear();
  packet.light_data.clear();
  packet.mesh_data.clear();
  packet.submesh_data.clear();
  packet.instance_data.clear();
  packet.cam_data.clear();
  packet.render_targets.clear();
  packet.anim_pos_keys.clear();
  packet.anim_rot_keys.clear();
  packet.anim_scaling_keys.clear();
  packet.node_anim_data.clear();
  packet.skeleton_node_data.clear();
  packet.bone_data.clear();
  packet.skinned_mesh_data.clear();

  packet.light_data.reserve(lights_.size());

  for (auto const light : lights_) {
    packet.light_data.emplace_back(light->GetColor(), light->GetIntensity(), light->GetDirection(),
      light->GetEntity()->GetTransform().GetWorldPosition(), light->GetType(), light->GetRange(),
      light->GetInnerAngle(),
      light->GetOuterAngle(), light->IsCastingShadow(), light->GetShadowNearPlane(), light->GetShadowNormalBias(),
      light->GetShadowDepthBias(), light->GetShadowExtension(),
      light->GetEntity()->GetTransform().CalculateLocalToWorldMatrixWithoutScale());
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

  auto const extract_from_mesh_comp{
    [&find_or_emplace_back_buffer, &packet, &find_or_emplace_back_texture](MeshComponentBase const* const comp) {
      auto const mesh{comp->GetMesh()};

      if (!mesh) {
        return;
      }

      auto const pos_buf_local_idx{find_or_emplace_back_buffer(mesh->GetPositionBuffer())};
      auto const norm_buf_local_idx{find_or_emplace_back_buffer(mesh->GetNormalBuffer())};
      auto const tan_buf_local_idx{find_or_emplace_back_buffer(mesh->GetTangentBuffer())};
      auto const uv_buf_local_idx{find_or_emplace_back_buffer(mesh->GetUvBuffer())};
      auto const meshlet_buf_local_idx{find_or_emplace_back_buffer(mesh->GetMeshletBuffer())};
      auto const vtx_idx_buf_local_idx{find_or_emplace_back_buffer(mesh->GetVertexIndexBuffer())};
      auto const prim_idx_buf_local_idx{find_or_emplace_back_buffer(mesh->GetPrimitiveIndexBuffer())};
      packet.mesh_data.emplace_back(pos_buf_local_idx, norm_buf_local_idx, tan_buf_local_idx, uv_buf_local_idx,
        meshlet_buf_local_idx, vtx_idx_buf_local_idx, prim_idx_buf_local_idx, mesh->GetBounds(),
        static_cast<unsigned>(mesh->GetVertexCount()), mesh->Has32BitVertexIndices());

      packet.submesh_data.reserve(packet.submesh_data.size() + mesh->GetSubmeshes().size());

      for (auto const& submesh : mesh->GetSubmeshes()) {
        auto const mtl{comp->GetMaterials()[submesh.GetMaterialIndex()]};

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

        packet.submesh_data.emplace_back(static_cast<unsigned>(packet.mesh_data.size() - 1), submesh.GetFirstMeshlet(),
          submesh.GetMeshletCount(), submesh.GetBaseVertex(), mtl_buf_local_idx, submesh.GetBounds());

        packet.instance_data.emplace_back(static_cast<unsigned>(packet.submesh_data.size() - 1),
          comp->GetEntity()->GetTransform().GetLocalToWorldMatrix());
      }
    }
  };

  std::ranges::for_each(static_mesh_components_, extract_from_mesh_comp);

  std::ranges::for_each(skinned_mesh_components_,
    [&extract_from_mesh_comp, &packet, this](SkinnedMeshComponent const* const comp) {
      auto const mesh{comp->GetMesh()};

      if (!mesh) {
        return;
      }

      extract_from_mesh_comp(comp);

      auto const anim{comp->GetCurrentAnimation()};

      if (!anim) {
        return;
      }

      // TODO add proper skinned mesh culling with GPU culling

      AABB const inf_aabb{
        Vector3{-std::numeric_limits<float>::infinity()},
        Vector3{std::numeric_limits<float>::infinity()}
      };

      // Set mesh AABB to infinity to prevent culling
      packet.mesh_data.back().bounds = inf_aabb;

      packet.buffers.emplace_back(mesh->GetBoneWeightBuffer());
      auto const bone_weight_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

      packet.buffers.emplace_back(mesh->GetBoneIndexBuffer());
      auto const bone_index_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

      packet.buffers.emplace_back(comp->GetSkinnedVertexBuffers()[render_manager_->GetCurrentFrameIndex()]);
      auto const skinned_pos_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

      packet.buffers.emplace_back(comp->GetSkinnedNormalBuffers()[render_manager_->GetCurrentFrameIndex()]);
      auto const skinned_norm_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

      packet.buffers.emplace_back(comp->GetSkinnedTangentBuffers()[render_manager_->GetCurrentFrameIndex()]);
      auto const skinned_tan_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

      packet.buffers.emplace_back(comp->GetBoneMatrixBuffers()[render_manager_->GetCurrentFrameIndex()]);
      auto const bone_mtx_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

      // Switch the original and skinned buffer indices so that the renderer can treat the skinned mesh as static after
      // the skinning is done

      auto const orig_pos_buf_local_idx{packet.mesh_data.back().pos_buf_local_idx};
      auto const orig_norm_buf_local_idx{packet.mesh_data.back().norm_buf_local_idx};
      auto const orig_tan_buf_local_idx{packet.mesh_data.back().tan_buf_local_idx};

      packet.mesh_data.back().pos_buf_local_idx = skinned_pos_buf_local_idx;
      packet.mesh_data.back().norm_buf_local_idx = skinned_norm_buf_local_idx;
      packet.mesh_data.back().tan_buf_local_idx = skinned_tan_buf_local_idx;

      // Extract animation data

      auto const node_anim_begin_local_idx{static_cast<unsigned>(packet.node_anim_data.size())};

      for (auto const& [pos_keys, rot_keys, scaling_keys, node_idx] : anim->node_anims) {
        auto const pos_key_begin_local_idx{static_cast<unsigned>(packet.anim_pos_keys.size())};
        auto const rot_key_begin_local_idx{static_cast<unsigned>(packet.anim_rot_keys.size())};
        auto const scaling_key_begin_local_idx{static_cast<unsigned>(packet.anim_scaling_keys.size())};

        std::ranges::copy(pos_keys, std::back_inserter(packet.anim_pos_keys));
        std::ranges::copy(rot_keys, std::back_inserter(packet.anim_rot_keys));
        std::ranges::copy(scaling_keys, std::back_inserter(packet.anim_scaling_keys));

        packet.node_anim_data.emplace_back(pos_key_begin_local_idx, static_cast<unsigned>(pos_keys.size()),
          rot_key_begin_local_idx, static_cast<unsigned>(rot_keys.size()), scaling_key_begin_local_idx,
          static_cast<unsigned>(scaling_keys.size()), node_idx);
      }

      // Extract skeleton data

      auto const skeleton_begin_local_idx{static_cast<unsigned>(packet.skeleton_node_data.size())};
      std::ranges::transform(mesh->GetSkeleton(), std::back_inserter(packet.skeleton_node_data),
        [&comp](SkeletonNode const& node) {
          return SkeletonNodeData{node.transform, node.parent_idx};
        });

      // Extract bone data

      auto const bone_begin_local_idx{static_cast<unsigned>(packet.bone_data.size())};
      std::ranges::transform(mesh->GetBones(), std::back_inserter(packet.bone_data),
        [&comp](Bone const& bone) {
          return BoneData{bone.offset_mtx, bone.skeleton_node_idx};
        });

      packet.skinned_mesh_data.emplace_back(static_cast<unsigned>(packet.mesh_data.size() - 1),
        orig_pos_buf_local_idx, orig_norm_buf_local_idx, orig_tan_buf_local_idx, bone_weight_buf_local_idx,
        bone_index_buf_local_idx, bone_mtx_buf_local_idx, comp->GetCurrentAnimationTime(), node_anim_begin_local_idx,
        static_cast<unsigned>(anim->node_anims.size()), skeleton_begin_local_idx,
        static_cast<unsigned>(mesh->GetSkeleton().size()), bone_begin_local_idx,
        static_cast<unsigned>(mesh->GetBones().size()));
    });

  auto const find_or_emplace_back_rt{
    [&packet](std::shared_ptr<RenderTarget> const& rt) -> unsigned {
      unsigned idx;

      if (auto const it{std::ranges::find(packet.render_targets, rt)}; it != std::ranges::end(packet.render_targets)) {
        idx = static_cast<unsigned>(it - packet.render_targets.begin());
      } else {
        idx = static_cast<unsigned>(packet.render_targets.size());
        packet.render_targets.emplace_back(rt);
      }

      return idx;
    }
  };

  packet.render_targets.emplace_back(rt_override_ ? rt_override_ : main_rt_); // The global RT is always at index 0!

  packet.cam_data.reserve(cameras_.size());

  for (auto const cam : cameras_) {
    unsigned rt_local_idx;

    if (auto const rt{cam->GetRenderTarget()}) {
      rt_local_idx = find_or_emplace_back_rt(rt);
    } else {
      rt_local_idx = 0; // The global RT is always at index 0!
    }

    packet.cam_data.emplace_back(cam->GetPosition(), cam->GetRightAxis(), cam->GetUpAxis(), cam->GetForwardAxis(),
      cam->GetNearClipPlane(), cam->GetFarClipPlane(), cam->GetType(), cam->GetVerticalPerspectiveFov(),
      cam->GetVerticalOrthographicSize(), cam->GetViewport(), rt_local_idx);
  }

  packet.gizmo_colors = gizmo_colors_;
  packet.line_gizmo_vertex_data = line_gizmo_vertex_data_;

  // This has to be cleared here so that the game, while rendering the frame,
  // can queue new gizmos without a race condition.
  ClearGizmoDrawQueue();

  packet.ssao_params = ssao_params_;
  packet.ssr_params = ssr_params_;
  packet.shadow_params = shadow_params_;

  packet.inv_gamma = inv_gamma_;

  packet.ssao_enabled = ssao_enabled_;
  packet.ssr_enabled = ssr_enabled_;

  packet.color_buffer_format = color_buffer_format_;

  packet.background_color = {0, 0, 0, 1};
  packet.skybox_cubemap = nullptr;

  if (auto const active_scene{Scene::GetActiveScene()}) {
    packet.ambient_light = active_scene->GetAmbientLightVector();

    if (active_scene->GetSkyMode() == SkyMode::Color) {
      auto const sky_color{active_scene->GetSkyColor()};
      packet.background_color = {sky_color[0], sky_color[1], sky_color[2], 1.0f};
    }

    if (active_scene->GetSkyMode() == SkyMode::Skybox) {
      if (auto const cubemap{active_scene->GetSkybox()}) {
        packet.skybox_cubemap = cubemap->GetTex();
      }
    }
  }

  packet.shadow_pso = shadow_pso_;
  packet.gbuffer_pso = gbuffer_pso_;
  packet.depth_resolve_pso = depth_resolve_pso_;
  packet.line_gizmo_pso = line_gizmo_pso_;
  packet.deferred_lighting_pso = deferred_lighting_pso_;
  packet.post_process_pso = post_process_pso_;
  packet.skybox_pso = skybox_pso_;
  packet.ssao_pso = ssao_pso_;
  packet.ssao_blur_pso = ssao_blur_pso_;
  packet.ssr_compose_pso = ssr_compose_pso_;
  packet.ssr_pso = ssr_pso_;
  packet.vtx_skinning_pso = vtx_skinning_pso_;
}


auto SceneRenderer::Render() -> void {
  next_per_draw_cb_idx_ = 0;
  next_per_view_cb_idx_ = 0;

  auto const frame_idx{render_manager_->GetCurrentFrameIndex()};

  auto& frame_packet{frame_packets_[frame_idx]};

  gizmo_color_buffer_.Resize(static_cast<int>(std::ssize(frame_packet.gizmo_colors)));
  std::ranges::copy(frame_packet.gizmo_colors, std::begin(gizmo_color_buffer_.GetData()));

  line_gizmo_vertex_data_buffer_.Resize(static_cast<int>(std::ssize(frame_packet.line_gizmo_vertex_data)));
  std::ranges::copy(frame_packet.line_gizmo_vertex_data, std::begin(line_gizmo_vertex_data_buffer_.GetData()));

  // Clears all render targets and dispatches skinning
  auto& prepare_cmd{render_manager_->AcquireCommandList()};
  prepare_cmd.Begin(nullptr);

  std::ranges::for_each(frame_packet.render_targets, [&prepare_cmd](std::shared_ptr<RenderTarget> const& rt) {
    prepare_cmd.ClearRenderTarget(*rt->GetColorTex(), rt->GetDesc().color_clear_value, {});
  });

  if (!frame_packet.skinned_mesh_data.empty()) {
    prepare_cmd.SetPipelineState(*frame_packet.vtx_skinning_pso);
  }

  for (auto& [mesh_data_local_idx, original_vertex_buf_local_idx, original_normal_buf_local_idx,
         original_tangent_buf_local_idx, bone_weight_buf_local_idx, bone_index_buf_local_idx, bone_matrix_buf_local_idx,
         cur_animation_time, node_anim_begin_local_idx, node_anim_count, skeleton_begin_local_idx, skeleton_size,
         bone_begin_local_idx, bone_count] : frame_packet.skinned_mesh_data) {
    // Skip skinning when we are sitting at 0 time.
    // This happens for example in the editor scene view.
    if (cur_animation_time == 0) {
      prepare_cmd.CopyBuffer(*frame_packet.buffers[frame_packet.mesh_data[mesh_data_local_idx].pos_buf_local_idx],
        *frame_packet.buffers[original_vertex_buf_local_idx]);
      prepare_cmd.CopyBuffer(*frame_packet.buffers[frame_packet.mesh_data[mesh_data_local_idx].norm_buf_local_idx],
        *frame_packet.buffers[original_normal_buf_local_idx]);
      prepare_cmd.CopyBuffer(*frame_packet.buffers[frame_packet.mesh_data[mesh_data_local_idx].tan_buf_local_idx],
        *frame_packet.buffers[original_tangent_buf_local_idx]);
      continue;
    }

    // Compute local node transforms

    for (unsigned i{0}; i < node_anim_count; i++) {
      auto const& [pos_key_begin_local_idx, pos_key_count, rot_key_begin_local_idx, rot_key_count,
        scaling_key_begin_local_idx, scaling_key_count, node_idx]{
        frame_packet.node_anim_data[node_anim_begin_local_idx + i]
      };

      Vector3 pos{};
      Quaternion rot{};
      Vector3 scale{1};

      auto const calc_interpolation_factor{
        [](float const from_time, float const to_time, float const current_time) {
          return (current_time - from_time) / (to_time - from_time);
        }
      };

      if (pos_key_count == 1) {
        pos = frame_packet.anim_pos_keys[pos_key_begin_local_idx].value;
      } else {
        for (unsigned j{0}; j < pos_key_count; j++) {
          if (auto const& [timestamp, value]{frame_packet.anim_pos_keys[pos_key_begin_local_idx + j]};
            timestamp > cur_animation_time) {
            auto const& [prev_timestamp, prev_value]{
              pos_key_begin_local_idx + j == 0
                ? frame_packet.anim_pos_keys.back()
                : frame_packet.anim_pos_keys[pos_key_begin_local_idx + j - 1]
            };
            pos = Lerp(prev_value, value, calc_interpolation_factor(prev_timestamp, timestamp, cur_animation_time));
            break;
          }
        }
      }

      if (rot_key_count == 1) {
        rot = frame_packet.anim_rot_keys[rot_key_begin_local_idx].value;
      } else {
        for (unsigned j{0}; j < rot_key_count; j++) {
          if (auto const& [timestamp, value]{frame_packet.anim_rot_keys[rot_key_begin_local_idx + j]};
            timestamp > cur_animation_time) {
            auto const& [prev_timestamp, prev_value]{
              rot_key_begin_local_idx + j == 0
                ? frame_packet.anim_rot_keys.back()
                : frame_packet.anim_rot_keys[rot_key_begin_local_idx + j - 1]
            };
            rot = Slerp(prev_value, value, calc_interpolation_factor(prev_timestamp, timestamp, cur_animation_time));
            break;
          }
        }
      }

      if (scaling_key_count == 1) {
        scale = frame_packet.anim_scaling_keys[scaling_key_begin_local_idx].value;
      } else {
        for (unsigned j{0}; j < scaling_key_count; j++) {
          if (auto const& [timestamp, value]{frame_packet.anim_scaling_keys[scaling_key_begin_local_idx + j]};
            timestamp > cur_animation_time) {
            auto const& [prev_timestamp, prev_value]{
              scaling_key_begin_local_idx + j == 0
                ? frame_packet.anim_scaling_keys.back()
                : frame_packet.anim_scaling_keys[scaling_key_begin_local_idx + j - 1]
            };
            scale = Lerp(prev_value, value, calc_interpolation_factor(prev_timestamp, timestamp, cur_animation_time));
            break;
          }
        }
      }

      frame_packet.skeleton_node_data[skeleton_begin_local_idx + node_idx].transform =
        Matrix4::Scale(scale) * static_cast<Matrix4>(rot) * Matrix4::Translate(pos);
    }

    // Accumulate node transforms

    for (unsigned i{0}; i < skeleton_size; i++) {
      if (auto& [transform, parent_idx]{frame_packet.skeleton_node_data[skeleton_begin_local_idx + i]}; parent_idx) {
        transform = transform * frame_packet.skeleton_node_data[skeleton_begin_local_idx + *parent_idx].transform;
      }
    }

    // Update bone matrices

    std::vector<Matrix4> bone_matrices(bone_count);

    for (unsigned i{0}; i < bone_count; i++) {
      bone_matrices[i] = frame_packet.bone_data[bone_begin_local_idx + i].offset_mtx * frame_packet.skeleton_node_data[
                           skeleton_begin_local_idx + frame_packet.bone_data[bone_begin_local_idx + i].
                           skeleton_node_idx].transform;
    }

    render_manager_->UpdateBuffer(*frame_packet.buffers[bone_matrix_buf_local_idx], 0,
      as_bytes(std::span{bone_matrices}));

    auto const& mesh_data{frame_packet.mesh_data[mesh_data_local_idx]};

    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, vtx_buf_idx),
      *frame_packet.buffers[original_vertex_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, norm_buf_idx),
      *frame_packet.buffers[original_normal_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, tan_buf_idx),
      *frame_packet.buffers[original_tangent_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, bone_weight_buf_idx),
      *frame_packet.buffers[bone_weight_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, bone_idx_buf_idx),
      *frame_packet.buffers[bone_index_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, bone_buf_idx),
      *frame_packet.buffers[bone_matrix_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, skinned_vtx_buf_idx),
      *frame_packet.buffers[mesh_data.pos_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, skinned_norm_buf_idx),
      *frame_packet.buffers[mesh_data.norm_buf_local_idx]);
    prepare_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, skinned_tan_buf_idx),
      *frame_packet.buffers[mesh_data.tan_buf_local_idx]);
    prepare_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, vtx_count), mesh_data.vtx_count);
    prepare_cmd.Dispatch(
      static_cast<UINT>(std::ceil(static_cast<float>(mesh_data.vtx_count) / static_cast<float>(SKINNING_CS_THREADS))),
      1, 1);
  }

  prepare_cmd.End();
  device_->ExecuteCommandLists(std::span{&prepare_cmd, 1});

  for (auto const& cam_data : frame_packet.cam_data) {
    // Compute render target dimensions

    auto& target_rt{*frame_packet.render_targets[cam_data.rt_local_idx]};
    auto const& target_rt_desc{target_rt.GetDesc()};

    auto const target_rt_width{target_rt_desc.width};
    auto const target_rt_height{target_rt_desc.height};

    CD3DX12_VIEWPORT const cam_viewport{
      cam_data.viewport.left * static_cast<FLOAT>(target_rt_width),
      cam_data.viewport.top * static_cast<float>(target_rt_height),
      std::max(
        cam_data.viewport.right * static_cast<float>(target_rt_width) - cam_data.viewport.left * static_cast<FLOAT>(
          target_rt_width), 1.0f),
      std::max(
        cam_data.viewport.bottom * static_cast<float>(target_rt_height) - cam_data.viewport.top * static_cast<float>(
          target_rt_height), 1.0f),
    };

    CD3DX12_RECT const cam_scissor{
      static_cast<LONG>(cam_data.viewport.left * static_cast<FLOAT>(target_rt_width)),
      static_cast<LONG>(cam_data.viewport.top * static_cast<FLOAT>(target_rt_height)),
      std::max(static_cast<LONG>(cam_data.viewport.right * static_cast<FLOAT>(target_rt_width)), 1l),
      std::max(static_cast<LONG>(cam_data.viewport.bottom * static_cast<FLOAT>(target_rt_height)), 1l),
    };

    auto const viewport_aspect{cam_viewport.Width / cam_viewport.Height};

    auto const transient_rt_width{static_cast<UINT>(cam_viewport.Width)};
    auto const transient_rt_height{static_cast<UINT>(cam_viewport.Height)};

    CD3DX12_VIEWPORT const transient_viewport{
      0.0f, 0.0f, static_cast<FLOAT>(transient_rt_width), static_cast<FLOAT>(transient_rt_height)
    };

    CD3DX12_RECT const transient_scissor{
      0, 0, static_cast<LONG>(transient_rt_width), static_cast<LONG>(transient_rt_height)
    };

    // Allocate render targets

    RenderTarget::Desc const depth_rt_desc{
      target_rt_width, target_rt_height, std::nullopt, depth_format_, 1, L"Camera Depth RenderTarget", false,
      {0.0f, 0.0f, 0.0f, 0.0f}, DEPTH_CLEAR_VALUE
    };

    RenderTarget::Desc const depth_sample_rt_desc{
      target_rt_width, target_rt_height, std::nullopt, depth_format_, 1, L"Camera Depth Sample RenderTarget", false,
      {0.0f, 0.0f, 0.0f, 0.0f}, DEPTH_CLEAR_VALUE
    };

    RenderTarget::Desc const gbuffer0_rt_desc{
      transient_rt_width, transient_rt_height, gbuffer0_format_, std::nullopt, 1,
      L"Camera GBuffer0 RenderTarget", false, {0.0f, 0.0f, 0.0f, 0.0f}
    };

    RenderTarget::Desc const gbuffer1_rt_desc{
      transient_rt_width, transient_rt_height, gbuffer1_format_, std::nullopt, 1,
      L"Camera GBuffer1 RenderTarget", false, {0.0f, 0.0f, 0.0f, 0.0f}
    };

    RenderTarget::Desc const gbuffer2_rt_desc{
      transient_rt_width, transient_rt_height, gbuffer2_format_, std::nullopt, 1,
      L"Camera GBuffer2 RenderTarget", false, {0.0f, 0.0f, 0.0f, 0.0f}
    };

    RenderTarget::Desc const color_hdr_rt_desc{
      transient_rt_width, transient_rt_height, frame_packet.color_buffer_format, std::nullopt,
      1, L"Camera HDR RenderTarget", false, frame_packet.background_color
    };

    auto const depth_rt{render_manager_->AcquireTemporaryRenderTarget(depth_rt_desc)};
    auto const depth_sample_rt{render_manager_->AcquireTemporaryRenderTarget(depth_sample_rt_desc)};
    auto const gbuffer0_rt{render_manager_->AcquireTemporaryRenderTarget(gbuffer0_rt_desc)};
    auto const gbuffer1_rt{render_manager_->AcquireTemporaryRenderTarget(gbuffer1_rt_desc)};
    auto const gbuffer2_rt{render_manager_->AcquireTemporaryRenderTarget(gbuffer2_rt_desc)};
    auto const color_hdr_rt{render_manager_->AcquireTemporaryRenderTarget(color_hdr_rt_desc)};

    // Fill constant buffers

    auto& per_frame_cb{per_frame_cbs_[frame_idx]};
    SetPerFrameConstants(per_frame_cb, static_cast<int>(transient_rt_width), static_cast<int>(transient_rt_height),
      frame_packet.ambient_light, frame_packet.shadow_params);

    auto const cam_view_mtx{
      Camera::CalculateViewMatrix(cam_data.position, cam_data.right, cam_data.up, cam_data.forward)
    };
    auto const cam_proj_mtx{
      TransformProjectionMatrixForRendering(Camera::CalculateProjectionMatrix(cam_data.type, cam_data.fov_vert_deg,
        cam_data.size_vert, viewport_aspect, cam_data.near_plane, cam_data.far_plane))
    };
    auto const cam_view_proj_mtx{cam_view_mtx * cam_proj_mtx};
    Frustum const cam_frust_ws{cam_view_proj_mtx};

    std::pmr::vector<unsigned> visible_light_indices;
    CullLights(cam_frust_ws, frame_packet.light_data, visible_light_indices);

    // Command list for the camera
    auto& cam_cmd{render_manager_->AcquireCommandList()};
    cam_cmd.Begin(nullptr);
    cam_cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Shadow pass
    std::array<Matrix4, MAX_CASCADE_COUNT> shadow_view_proj_matrices;
    auto const shadow_cascade_boundaries{CalculateCameraShadowCascadeBoundaries(cam_data, frame_packet.shadow_params)};
    DrawDirectionalShadowMaps(frame_packet, visible_light_indices, cam_data, viewport_aspect,
      frame_packet.shadow_params.cascade_count, shadow_cascade_boundaries, shadow_view_proj_matrices, cam_cmd);

    UpdatePunctualShadowAtlas(*punctual_shadow_atlas_, frame_packet.light_data, visible_light_indices, cam_data,
      cam_view_proj_mtx, frame_packet.shadow_params.distance);
    DrawPunctualShadowMaps(*punctual_shadow_atlas_, frame_packet, cam_cmd);

    std::pmr::vector<unsigned> visible_static_submesh_instance_indices;
    CullStaticSubmeshInstances(cam_frust_ws, frame_packet.mesh_data, frame_packet.submesh_data,
      frame_packet.instance_data, visible_static_submesh_instance_indices);

    auto& cam_per_view_cb{AcquirePerViewConstantBuffer()};
    SetPerViewConstants(cam_per_view_cb, cam_view_mtx, cam_proj_mtx, shadow_cascade_boundaries, cam_data.position);

    cam_cmd.SetViewports(std::span{static_cast<D3D12_VIEWPORT const*>(&transient_viewport), 1});
    cam_cmd.SetScissorRects(std::span{static_cast<D3D12_RECT const*>(&transient_scissor), 1});

    // GBuffer pass

    cam_cmd.SetPipelineState(*frame_packet.gbuffer_pso);
    std::array<graphics::Texture const*, 3> gbuffer_textures{
      gbuffer0_rt->GetColorTex().get(), gbuffer1_rt->GetColorTex().get(), gbuffer2_rt->GetColorTex().get()
    };
    cam_cmd.SetRenderTargets(std::span{gbuffer_textures},
      depth_rt->GetDepthStencilTex().get());
    cam_cmd.ClearRenderTarget(*gbuffer0_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cam_cmd.ClearRenderTarget(*gbuffer1_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cam_cmd.ClearRenderTarget(*gbuffer2_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cam_cmd.ClearDepthStencil(*depth_rt->GetDepthStencilTex(), D3D12_CLEAR_FLAG_DEPTH, DEPTH_CLEAR_VALUE, 0, {});

    for (auto const instance_idx : visible_static_submesh_instance_indices) {
      auto const& instance{frame_packet.instance_data[instance_idx]};
      auto const& submesh{frame_packet.submesh_data[instance.submesh_local_idx]};
      auto const& mesh{frame_packet.mesh_data[submesh.mesh_local_idx]};
      auto const& mtl_buf{frame_packet.buffers[submesh.mtl_buf_local_idx]};

      auto& per_draw_cb{AcquirePerDrawConstantBuffer()};
      SetPerDrawConstants(per_draw_cb, instance.local_to_world_mtx, cam_view_mtx, cam_proj_mtx);

      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, pos_buf_idx),
        *frame_packet.buffers[mesh.pos_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, norm_buf_idx),
        *frame_packet.buffers[mesh.norm_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, tan_buf_idx),
        *frame_packet.buffers[mesh.tan_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, uv_buf_idx),
        *frame_packet.buffers[mesh.uv_buf_local_idx]);
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GBufferDrawParams, mtl_idx), *mtl_buf);
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, mtl_samp_idx), samp_af16_wrap_.Get());
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GBufferDrawParams, per_draw_cb_idx),
        *per_draw_cb.GetBuffer());
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GBufferDrawParams, per_view_cb_idx),
        *cam_per_view_cb.GetBuffer());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, vertex_idx_buf_idx),
        *frame_packet.buffers[mesh.vtx_idx_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, prim_idx_buf_idx),
        *frame_packet.buffers[mesh.prim_idx_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, meshlet_buf_idx),
        *frame_packet.buffers[mesh.meshlet_buf_local_idx]);
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, idx32), mesh.idx32);

      DrawSubmesh(submesh, PIPELINE_PARAM_INDEX(GBufferDrawParams, meshlet_count),
        PIPELINE_PARAM_INDEX(GBufferDrawParams, meshlet_offset),
        PIPELINE_PARAM_INDEX(GBufferDrawParams, instance_count),
        PIPELINE_PARAM_INDEX(GBufferDrawParams, instance_offset),
        PIPELINE_PARAM_INDEX(GBufferDrawParams, base_vertex), cam_cmd);
    }

    // Duplicate depth texture so that we can sample it and use it for stencil test at the same time.
    // This is only a workaround, the ideal would be to transition the depth texture to a simultaneous
    // shader reource/depth read state, but the current architecture doesn't allow that.
    cam_cmd.CopyTexture(*depth_sample_rt->GetDepthStencilTex(), *depth_rt->GetDepthStencilTex());

    auto ssao_tex{white_tex_.get()};

    // SSAO pass
    if (frame_packet.ssao_enabled) {
      auto const ssao_rt{
        render_manager_->AcquireTemporaryRenderTarget(RenderTarget::Desc{
          transient_rt_width, transient_rt_height, ssao_buffer_format_, std::nullopt, 1, L"SSAO RT"
        })
      };

      cam_cmd.SetPipelineState(*frame_packet.ssao_pso);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, noise_tex_idx),
        *ssao_noise_tex_);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, depth_tex_idx),
        *depth_sample_rt->GetDepthStencilTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, gbuffer1_tex_idx),
        *gbuffer1_rt->GetColorTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, samp_buf_idx),
        *ssao_samples_buffer_.GetBuffer());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, point_clamp_samp_idx), samp_point_clamp_.Get());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, point_wrap_samp_idx), samp_point_wrap_.Get());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, radius),
        *std::bit_cast<UINT*>(&frame_packet.ssao_params.radius));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, bias),
        *std::bit_cast<UINT*>(&frame_packet.ssao_params.bias));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, power),
        *std::bit_cast<UINT*>(&frame_packet.ssao_params.power));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoDrawParams, sample_count),
        frame_packet.ssao_params.sample_count);
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(SsaoDrawParams, per_view_cb_idx),
        *cam_per_view_cb.GetBuffer());
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(SsaoDrawParams, per_frame_cb_idx),
        *per_frame_cb.GetBuffer());
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<graphics::Texture const*>(ssao_rt->GetColorTex().get())}.data(), 1
      }, nullptr);
      cam_cmd.ClearRenderTarget(*ssao_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});
      cam_cmd.DrawInstanced(3, 1, 0, 0);

      auto const ssao_blur_rt{
        render_manager_->AcquireTemporaryRenderTarget([&ssao_rt] {
          auto ret{ssao_rt->GetDesc()};
          ret.debug_name = L"SSAO Blur RT";
          return ret;
        }())
      };

      cam_cmd.SetPipelineState(*frame_packet.ssao_blur_pso);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoBlurDrawParams, in_tex_idx),
        *ssao_rt->GetColorTex());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsaoBlurDrawParams, point_clamp_samp_idx),
        samp_point_clamp_.Get());
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<graphics::Texture const*>(ssao_blur_rt->GetColorTex().get())}.data(), 1
      }, nullptr);
      cam_cmd.ClearRenderTarget(*ssao_blur_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});
      cam_cmd.DrawInstanced(3, 1, 0, 0);

      ssao_tex = ssao_blur_rt->GetColorTex().get();
    }

    // Deferred lighting pass

    auto const light_count{std::ssize(visible_light_indices)};
    std::vector<ShaderLight> light_data(light_count);

    for (auto i = 0; i < light_count; i++) {
      light_data[i].color = frame_packet.light_data[visible_light_indices[i]].color;
      light_data[i].intensity = frame_packet.light_data[visible_light_indices[i]].intensity;
      light_data[i].type = static_cast<int>(frame_packet.light_data[visible_light_indices[i]].type);
      light_data[i].direction = frame_packet.light_data[visible_light_indices[i]].direction;
      light_data[i].isCastingShadow = FALSE;
      light_data[i].range = frame_packet.light_data[visible_light_indices[i]].range;
      light_data[i].halfInnerAngleCos = std::cos(
        ToRadians(frame_packet.light_data[visible_light_indices[i]].inner_angle / 2.0f));
      light_data[i].halfOuterAngleCos = std::cos(
        ToRadians(frame_packet.light_data[visible_light_indices[i]].outer_angle / 2.0f));
      light_data[i].position = frame_packet.light_data[visible_light_indices[i]].position;
      light_data[i].depthBias = frame_packet.light_data[visible_light_indices[i]].shadow_depth_bias;
      light_data[i].normalBias = frame_packet.light_data[visible_light_indices[i]].shadow_normal_bias;

      for (auto& sample : light_data[i].sampleShadowMap) {
        sample = FALSE;
      }
    }

    for (auto i{0}; i < light_count; i++) {
      if (auto const light{frame_packet.light_data[visible_light_indices[i]]};
        light.type == LightComponent::Type::Directional && light.casts_shadow) {
        light_data[i].isCastingShadow = TRUE;

        for (auto cascade_idx{0u}; cascade_idx < frame_packet.shadow_params.cascade_count; cascade_idx++) {
          light_data[i].sampleShadowMap[cascade_idx] = TRUE;
          light_data[i].shadowViewProjMatrices[cascade_idx] = shadow_view_proj_matrices[cascade_idx];
        }

        break;
      }
    }

    punctual_shadow_atlas_->SetLookUpInfo(light_data);

    auto& light_buffer{light_buffer_};
    light_buffer.Resize(static_cast<int>(light_count));
    render_manager_->UpdateBuffer(*light_buffer.GetBuffer(), 0, as_bytes(std::span{light_data}));

    cam_cmd.SetPipelineState(*frame_packet.deferred_lighting_pso);
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, gbuffer0_idx),
      *gbuffer0_rt->GetColorTex());
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, gbuffer1_idx),
      *gbuffer1_rt->GetColorTex());
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, gbuffer2_idx),
      *gbuffer2_rt->GetColorTex());
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, depth_tex_idx),
      *depth_sample_rt->GetDepthStencilTex());
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, ssao_tex_idx), *ssao_tex);

    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, dir_shadow_arr_idx),
      *dir_shadow_map_arr_->GetTex());
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, punc_shadow_atlas_idx),
      *punctual_shadow_atlas_->GetTex());
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, shadow_samp_idx),
#ifdef REVERSE_Z
      samp_cmp_pcf_ge_.Get()
#else
      samp_cmp_pcf_le_.Get()
#endif
    );
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, point_clamp_samp_idx),
      samp_point_clamp_.Get());

    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, light_buf_idx),
      *light_buffer.GetBuffer());
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, light_count),
      static_cast<UINT>(light_count));
    cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, per_view_cb_idx),
      *cam_per_view_cb.GetBuffer());
    cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, per_frame_cb_idx),
      *per_frame_cb.GetBuffer());

    cam_cmd.SetRenderTargets(std::span{
      std::array{static_cast<graphics::Texture const*>(color_hdr_rt->GetColorTex().get())}.data(), 1
    }, depth_rt->GetDepthStencilTex().get());
    cam_cmd.ClearRenderTarget(*color_hdr_rt->GetColorTex(), frame_packet.background_color, {});

    cam_cmd.DrawInstanced(3, 1, 0, 0);

    // SSR pass
    if (frame_packet.ssr_enabled) {
      cam_cmd.SetPipelineState(*frame_packet.ssr_pso);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsrDrawParams, depth_tex_idx),
        *depth_sample_rt->GetDepthStencilTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsrDrawParams, lit_scene_tex_idx), *color_hdr_rt->GetColorTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsrDrawParams, gbuffer1_tex_idx), *gbuffer1_rt->GetColorTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsrDrawParams, gbuffer2_tex_idx), *gbuffer2_rt->GetColorTex());

      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, point_clamp_samp_idx), samp_point_clamp_.Get());
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(SsrDrawParams, per_view_cb_idx), *cam_per_view_cb.GetBuffer());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, max_roughness),
        *std::bit_cast<UINT const*>(&frame_packet.ssr_params.max_roughness));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, ray_length_vs),
        *std::bit_cast<UINT const*>(&frame_packet.ssr_params.ray_length_vs));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, max_march_steps),
        frame_packet.ssr_params.max_march_steps);
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, depth_tolerance_ndc),
        *std::bit_cast<UINT const*>(&frame_packet.ssr_params.depth_tolerance_ndc));

      RenderTarget::Desc const ssr_rt_desc{
        transient_rt_width, transient_rt_height, frame_packet.color_buffer_format, std::nullopt,
        1, L"SSR RT", false, std::array{0.0f, 0.0f, 0.0f, 1.0f}
      };

      auto const ssr_rt{render_manager_->AcquireTemporaryRenderTarget(ssr_rt_desc)};

      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<graphics::Texture const*>(ssr_rt->GetColorTex().get())}.data(), 1
      }, depth_rt->GetDepthStencilTex().get());
      cam_cmd.ClearRenderTarget(*ssr_rt->GetColorTex(), ssr_rt_desc.color_clear_value, {});

      cam_cmd.DrawInstanced(3, 1, 0, 0);

      cam_cmd.SetPipelineState(*frame_packet.ssr_compose_pso);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsrComposeDrawParams, ssr_tex_idx),
        *ssr_rt->GetColorTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsrComposeDrawParams, lit_scene_tex_idx),
        *color_hdr_rt->GetColorTex());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrComposeDrawParams, point_clamp_samp_idx),
        samp_point_clamp_.Get());

      RenderTarget::Desc const ssr_compose_rt_desc{
        transient_rt_width, transient_rt_height, frame_packet.color_buffer_format, std::nullopt,
        1, L"SSR Compose RT", false, std::array{0.0f, 0.0f, 0.0f, 1.0f}
      };

      auto const ssr_compose_rt{render_manager_->AcquireTemporaryRenderTarget(ssr_compose_rt_desc)};
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<graphics::Texture const*>(ssr_compose_rt->GetColorTex().get())}.data(), 1
      }, nullptr);
      cam_cmd.ClearRenderTarget(*ssr_compose_rt->GetColorTex(), ssr_compose_rt_desc.color_clear_value, {});

      cam_cmd.DrawInstanced(3, 1, 0, 0);

      cam_cmd.CopyTexture(*color_hdr_rt->GetColorTex(), *ssr_compose_rt->GetColorTex());
    }

    // Skybox pass
    if (frame_packet.skybox_cubemap) {
      cam_cmd.SetPipelineState(*frame_packet.skybox_pso);
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<graphics::Texture const*>(color_hdr_rt->GetColorTex().get())}.data(), 1
      }, depth_rt->GetDepthStencilTex().get());

      auto const cube_mesh{App::Instance().GetResourceManager().GetCubeMesh()};
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, pos_buf_idx),
        *cube_mesh->GetPositionBuffer());
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(SkyboxDrawParams, per_view_cb_idx),
        *cam_per_view_cb.GetBuffer());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, cubemap_idx),
        *frame_packet.skybox_cubemap);
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SkyboxDrawParams, samp_idx), samp_af16_clamp_.Get());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, vertex_idx_buf_idx),
        *cube_mesh->GetVertexIndexBuffer());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, prim_idx_buf_idx),
        *cube_mesh->GetPrimitiveIndexBuffer());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, meshlet_buf_idx),
        *cube_mesh->GetMeshletBuffer());

      DrawSubmesh(1, 0, 0, {}, {}, {}, {}, {}, cam_cmd);
    }

    // Post-processing pass

    auto const* const post_process_input_rt{color_hdr_rt.get()};

    cam_cmd.SetViewports(std::span{static_cast<D3D12_VIEWPORT const*>(&cam_viewport), 1});
    cam_cmd.SetScissorRects(std::span{static_cast<D3D12_RECT const*>(&cam_scissor), 1});

    cam_cmd.SetPipelineState(*frame_packet.post_process_pso);
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(PostProcessDrawParams, in_tex_idx),
      *post_process_input_rt->GetColorTex());
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(PostProcessDrawParams, inv_gamma),
      *std::bit_cast<UINT*>(&frame_packet.inv_gamma));
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(PostProcessDrawParams, bi_clamp_samp_idx), samp_bi_clamp_.Get());
    cam_cmd.SetRenderTargets(std::span{
      std::array{static_cast<graphics::Texture const*>(target_rt.GetColorTex().get())}.data(), 1
    }, nullptr);
    cam_cmd.DrawInstanced(3, 1, 0, 0);

    // Gizmo pass

    if (!frame_packet.line_gizmo_vertex_data.empty()) {
      cam_cmd.SetPipelineState(*frame_packet.line_gizmo_pso);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GizmoDrawParams, vertex_buf_idx),
        *line_gizmo_vertex_data_buffer_.GetBuffer());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GizmoDrawParams, color_buf_idx),
        *gizmo_color_buffer_.GetBuffer());
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GizmoDrawParams, per_view_cb_idx),
        *cam_per_view_cb.GetBuffer());
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<graphics::Texture const*>(target_rt.GetColorTex().get())}.data(), 1
      }, nullptr);
      cam_cmd.SetScissorRects(std::span{static_cast<D3D12_RECT const*>(&cam_scissor), 1});
      cam_cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
      cam_cmd.DrawInstanced(2, static_cast<UINT>(frame_packet.line_gizmo_vertex_data.size()), 0, 0);
    }

    cam_cmd.End();
    device_->ExecuteCommandLists(std::span{&cam_cmd, 1});
  }
}


auto SceneRenderer::DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void {
  gizmo_colors_.emplace_back(color);
  line_gizmo_vertex_data_.emplace_back(from, static_cast<std::uint32_t>(gizmo_colors_.size() - 1), to, 0.0f);
}


auto SceneRenderer::GetRenderTargetOverride() -> std::shared_ptr<RenderTarget> const& {
  return rt_override_;
}


auto SceneRenderer::SetRenderTargetOverride(std::shared_ptr<RenderTarget> rt_override) -> void {
  rt_override_ = std::move(rt_override);
}


auto SceneRenderer::GetCurrentRenderTarget() const -> RenderTarget const& {
  return rt_override_ ? *rt_override_ : *main_rt_;
}


auto SceneRenderer::IsUsingPreciseColorFormat() const noexcept -> bool {
  return color_buffer_format_ == precise_color_buffer_format_;
}


auto SceneRenderer::SetUsePreciseColorFormat(bool const precise) noexcept -> void {
  color_buffer_format_ = precise ? precise_color_buffer_format_ : imprecise_color_buffer_format_;
  RecreatePipelines();
}


auto SceneRenderer::GetShadowDistance() const noexcept -> float {
  return shadow_params_.distance;
}


auto SceneRenderer::SetShadowDistance(float const distance) noexcept -> void {
  shadow_params_.distance = std::max(0.0f, distance);
}


auto SceneRenderer::GetShadowCascadeCount() const noexcept -> unsigned {
  return shadow_params_.cascade_count;
}


auto SceneRenderer::SetShadowCascadeCount(unsigned cascade_count) noexcept -> void {
  shadow_params_.cascade_count = std::clamp(cascade_count, 1u, MAX_CASCADE_COUNT);
  unsigned const splitCount{shadow_params_.cascade_count - 1};

  for (auto i = 1u; i < splitCount; i++) {
    shadow_params_.normalized_cascade_splits[i] = std::max(shadow_params_.normalized_cascade_splits[i - 1],
      shadow_params_.normalized_cascade_splits[i]);
  }
}


auto SceneRenderer::GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const> {
  return {
    std::begin(shadow_params_.normalized_cascade_splits), static_cast<std::size_t>(shadow_params_.cascade_count - 1)
  };
}


auto SceneRenderer::SetNormalizedShadowCascadeSplit(int const idx, float const split) noexcept -> void {
  auto const splitCount{shadow_params_.cascade_count - 1};

  if (idx < 0 || static_cast<unsigned>(idx) >= splitCount) {
    return;
  }

  float const clampMin{idx == 0 ? 0.0f : shadow_params_.normalized_cascade_splits[idx - 1]};
  float const clampMax{
    static_cast<unsigned>(idx) == splitCount - 1 ? 1.0f : shadow_params_.normalized_cascade_splits[idx + 1]
  };

  shadow_params_.normalized_cascade_splits[idx] = std::clamp(split, clampMin, clampMax);
}


auto SceneRenderer::IsVisualizingShadowCascades() const noexcept -> bool {
  return shadow_params_.visualize_cascades;
}


auto SceneRenderer::VisualizeShadowCascades(bool const visualize) noexcept -> void {
  shadow_params_.visualize_cascades = visualize;
}


auto SceneRenderer::GetShadowFilteringMode() const noexcept -> ShadowFilteringMode {
  return shadow_params_.filtering_mode;
}


auto SceneRenderer::SetShadowFilteringMode(ShadowFilteringMode const filtering_mode) noexcept -> void {
  shadow_params_.filtering_mode = filtering_mode;
}


auto SceneRenderer::IsSsaoEnabled() const noexcept -> bool {
  return ssao_enabled_;
}


auto SceneRenderer::SetSsaoEnabled(bool const enabled) noexcept -> void {
  ssao_enabled_ = enabled;
}


auto SceneRenderer::GetSsaoParams() const noexcept -> SsaoParams const& {
  return ssao_params_;
}


auto SceneRenderer::SetSsaoParams(SsaoParams const& ssao_params) noexcept -> void {
  if (ssao_params_.sample_count != ssao_params.sample_count) {
    RecreateSsaoSamples(ssao_params.sample_count);
  }

  ssao_params_ = ssao_params;
}


auto SceneRenderer::IsSsrEnabled() const noexcept -> bool {
  return ssr_enabled_;
}


auto SceneRenderer::SetSsrEnabled(bool const enabled) noexcept -> void {
  ssr_enabled_ = enabled;
}


auto SceneRenderer::GetSsrParams() const noexcept -> SsrParams const& {
  return ssr_params_;
}


auto SceneRenderer::SetSsrParams(SsrParams const& ssr_params) -> void {
  ssr_params_ = ssr_params;
}


auto SceneRenderer::GetGamma() const noexcept -> f32 {
  return 1.f / inv_gamma_;
}


auto SceneRenderer::SetGamma(f32 const gamma) noexcept -> void {
  inv_gamma_ = 1.f / gamma;
}


auto SceneRenderer::Register(StaticMeshComponent const& static_mesh_component) noexcept -> void {
  static_mesh_components_.emplace_back(std::addressof(static_mesh_component));
}


auto SceneRenderer::Unregister(StaticMeshComponent const& static_mesh_component) noexcept -> void {
  std::erase(static_mesh_components_, std::addressof(static_mesh_component));
}


auto SceneRenderer::Register(SkinnedMeshComponent const& skinned_mesh_component) noexcept -> void {
  skinned_mesh_components_.emplace_back(std::addressof(skinned_mesh_component));
}


auto SceneRenderer::Unregister(SkinnedMeshComponent const& skinned_mesh_component) noexcept -> void {
  std::erase(skinned_mesh_components_, std::addressof(skinned_mesh_component));
}


auto SceneRenderer::Register(LightComponent const& light_component) noexcept -> void {
  lights_.emplace_back(std::addressof(light_component));
}


auto SceneRenderer::Unregister(LightComponent const& light_component) noexcept -> void {
  std::erase(lights_, std::addressof(light_component));
}


auto SceneRenderer::Register(Camera const& cam) noexcept -> void {
  cameras_.emplace_back(&cam);
}


auto SceneRenderer::Unregister(Camera const& cam) noexcept -> void {
  std::erase(cameras_, &cam);
}
}
