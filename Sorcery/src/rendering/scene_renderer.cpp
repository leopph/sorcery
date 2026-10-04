#include "scene_renderer.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iterator>
#include <random>

#include "render_instance_registry.hpp"
#include "render_resource_registry.hpp"
#include "ShadowCascadeBoundary.hpp"
#include "../app.hpp"
#include "../random.hpp"
#include "../resource_manager.hpp"
#include "../Window.hpp"
#include "../Resources/Scene.hpp"
#include "../scene_objects/Entity.hpp"
#include "../scene_objects/TransformComponent.hpp"
#include "shaders/shader_interop.h"

#ifndef NDEBUG
#include "shaders/generated/Debug/brdf_integration_ps.h"
#include "shaders/generated/Debug/brdf_integration_vs.h"
#include "shaders/generated/Debug/deferred_lighting_ps.h"
#include "shaders/generated/Debug/deferred_lighting_vs.h"
#include "shaders/generated/Debug/depth_only_as.h"
#include "shaders/generated/Debug/depth_only_ms.h"
#include "shaders/generated/Debug/depth_only_ps.h"
#include "shaders/generated/Debug/depth_resolve_cs.h"
#include "shaders/generated/Debug/envmap_prefilter_ms.h"
#include "shaders/generated/Debug/envmap_prefilter_ps.h"
#include "shaders/generated/Debug/gbuffer_velocity_as.h"
#include "shaders/generated/Debug/gbuffer_velocity_ms.h"
#include "shaders/generated/Debug/gbuffer_velocity_ps.h"
#include "shaders/generated/Debug/gizmos_line_vs.h"
#include "shaders/generated/Debug/gizmos_ps.h"
#include "shaders/generated/Debug/irradiance_ms.h"
#include "shaders/generated/Debug/irradiance_ps.h"
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
#include "shaders/generated/Debug/taa_resolve_ps.h"
#include "shaders/generated/Debug/taa_resolve_vs.h"
#include "shaders/generated/Debug/vtx_skinning_cs.h"
#else
#include "shaders/generated/Release//envmap_prefilter_ms.h"
#include "shaders/generated/Release//envmap_prefilter_ps.h"
#include "shaders/generated/Release/brdf_integration_ps.h"
#include "shaders/generated/Release/brdf_integration_vs.h"
#include "shaders/generated/Release/deferred_lighting_ps.h"
#include "shaders/generated/Release/deferred_lighting_vs.h"
#include "shaders/generated/Release/depth_only_as.h"
#include "shaders/generated/Release/depth_only_ms.h"
#include "shaders/generated/Release/depth_only_ps.h"
#include "shaders/generated/Release/depth_resolve_cs.h"
#include "shaders/generated/Release/gbuffer_velocity_as.h"
#include "shaders/generated/Release/gbuffer_velocity_ms.h"
#include "shaders/generated/Release/gbuffer_velocity_ps.h"
#include "shaders/generated/Release/gizmos_line_vs.h"
#include "shaders/generated/Release/gizmos_ps.h"
#include "shaders/generated/Release/irradiance_ms.h"
#include "shaders/generated/Release/irradiance_ps.h"
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
#include "shaders/generated/Release/taa_resolve_ps.h"
#include "shaders/generated/Release/taa_resolve_vs.h"
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


[[nodiscard]]
auto ToShaderBlendMode(MaterialBlendMode const mode) -> int {
  switch (mode) {
    case MaterialBlendMode::kOpaque:
      return BLEND_MODE_OPAQUE;
    case MaterialBlendMode::kAlphaClip:
      return BLEND_MODE_ALPHA_CLIP;
  }

  assert(false && "Invalid material blend mode.");
  return BLEND_MODE_OPAQUE;
}
}


SceneRenderer::SceneRenderer(Window& window, wand::GraphicsDevice& device, RenderManager& render_manager,
                             RenderResourceRegistry& render_resource_registry,
                             RenderInstanceRegistry& render_instance_registry) :
  window_{&window},
  device_{&device},
  render_manager_{&render_manager},
  resource_registry_{&render_resource_registry},
  instance_registry_{&render_instance_registry} {
  main_rt_ = RenderTarget::New(*device_, RenderTarget::Desc{
    static_cast<UINT>(window_->GetClientAreaSize().width), static_cast<UINT>(window_->GetClientAreaSize().height),
    DXGI_FORMAT_R8G8B8A8_UNORM, std::nullopt, 1, L"Main RT", false
  });

  dir_shadow_map_arr_ = std::make_unique<DirectionalLightShadowMapArray>(device_.Get(), depth_format_, 4096);
  dir_shadow_map_arr_->GetTex()->SetDebugName(L"Directional Shadow Map Array");

  pos_shadow_atlas_ = std::make_unique<PositionalLightShadowAtlas>(device_.Get(), depth_format_, 4096);
  pos_shadow_atlas_->GetTex()->SetDebugName(L"Punctual Shadow Atlas");

  RecreatePipelines();

  CreatePerViewConstantBuffers(1);
  CreatePerInstanceConstantBuffers(100);

  samp_cmp_pcf_ge_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_GREATER_EQUAL, {},
    0, 0
  });

  samp_cmp_pcf_le_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 1, D3D12_COMPARISON_FUNC_LESS_EQUAL, {}, 0, 0
  });

  samp_af16_clamp_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
    D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0, 16, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
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

  samp_point_wrap_ = device_->CreateSampler(D3D12_SAMPLER_DESC{
    D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
    D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0, 1, D3D12_COMPARISON_FUNC_NEVER, {}, 0, std::numeric_limits<float>::max()
  });

  window_size_event_listener_ = window_->OnWindowSize.add_listener([this](Extent2D<unsigned> const size) {
    OnWindowSize(size);
  });

  RecreateSsaoSamples(ssao_params_.sample_count);

  ssao_noise_tex_ = device_->CreateTexture(wand::TextureDesc{
    wand::TextureDimension::k2D, SSAO_NOISE_TEX_DIM, SSAO_NOISE_TEX_DIM, 1, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1,
    false, false, true, false
  }, wand::CpuAccess::kNone, nullptr);
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

  white_tex_ = device_->CreateTexture(wand::TextureDesc{
    wand::TextureDimension::k2D, 1, 1, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, 1, false, false, true, false
  }, wand::CpuAccess::kNone, nullptr);

  std::array<std::uint8_t, 4> constexpr white_tex_data{255, 255, 255, 255};

  render_manager_->UpdateTexture(*white_tex_, 0, std::array{
    D3D12_SUBRESOURCE_DATA{white_tex_data.data(), sizeof(white_tex_data), sizeof(white_tex_data)}
  });

  brdf_integration_map_ = device_->CreateTexture(wand::TextureDesc{
    wand::TextureDimension::k2D, brdf_integration_map_size_, brdf_integration_map_size_, 1, 1,
    brdf_integration_map_format_, 1, false, true, true, false
  }, wand::CpuAccess::kNone, std::array{
    D3D12_CLEAR_VALUE{.Format = brdf_integration_map_format_, .Color = {0.F, 0.F, 0.F, 1.F}}
  }.data());
}


SceneRenderer::~SceneRenderer() {
  window_->OnWindowSize.remove_listener(window_size_event_listener_);
}


auto SceneRenderer::ExtractFrame(RenderFrame& frame) -> void {
  auto& packet{frame_packets_[frame.GetIndex()]};

  packet.buffers.clear();
  packet.textures.clear();
  packet.light_data.clear();
  packet.geom_batches.clear();
  packet.mtl_slot_groups.clear();
  packet.submesh_data.clear();
  packet.instance_data.clear();
  packet.instance_materials.clear();
  packet.cam_data.clear();
  packet.render_targets.clear();
  packet.anim_pos_keys.clear();
  packet.anim_rot_keys.clear();
  packet.anim_scaling_keys.clear();
  packet.node_anim_data.clear();
  packet.skeleton_node_data.clear();
  packet.bone_data.clear();
  packet.skinning_data.clear();
  packet.ssao_samples.clear();

  // Extract lights

  packet.light_data.reserve(lights_.size());

  for (auto const light : lights_) {
    packet.light_data.emplace_back(light->GetColor(), light->GetIntensity(), light->GetDirection(),
      light->GetEntity()->GetTransform().GetWorldPosition(), light->GetType(), light->GetRange(),
      light->GetInnerAngle(),
      light->GetOuterAngle(), light->IsCastingShadow(), light->GetShadowNearPlane(), light->GetShadowNormalBias(),
      light->GetShadowDepthBias(), light->GetShadowExtension(),
      light->GetEntity()->GetTransform().CalculateLocalToWorldMatrixWithoutScale());
  }

  // Add the default cube mesh to the packet. Used for a couple of passes conditionally.
  // If other default meshes are to be used in the future, they should be added here as well.
  // This is stored as a GeometryBatch in case a component references it.
  packet.cube_geom_local_idx = AddMeshToPacket(*App::Instance().GetResourceManager().GetCubeMesh(), frame, packet);

  // Extract static mesh components
  // Because they're static, we can batch them for better performance.

  // Sorting static mesh components gives the basis for batch extraction
  std::ranges::sort(static_mesh_components_,
    [](MeshComponentBase const* const lhs, MeshComponentBase const* const rhs) {
      auto const lhs_mesh{lhs->GetMesh().Observe()};
      auto const rhs_mesh{rhs->GetMesh().Observe()};

      if (!lhs_mesh || !rhs_mesh) {
        return lhs_mesh && !rhs_mesh;
      }

      return lhs_mesh->GetId() < rhs_mesh->GetId();
    });

  for (auto* const comp : static_mesh_components_) {
    auto const mesh{comp->GetMesh().Observe()};

    if (!mesh) {
      continue;
    }

    auto const [render_inst, is_new] = instance_registry_->CreateOrGetStaticInstance(comp->GetId());
    auto& inst_state = render_inst->GetState();

    if (is_new || inst_state.src_mesh_id != mesh->GetId() || inst_state.src_mesh_rev != mesh->GetRevision()) {
      SyncStaticInstance(*comp, *render_inst);
    }

    // We reuse the same batch for all instances of the same mesh.
    auto const geom_batch_local_idx = FindOrAddMeshInPacket(*mesh, frame, packet);
    // Because we sorted the components by mesh, we can call this function for each component and it will add instances to the same batch.
    AddMeshComponentToPacket(*comp, packet.geom_batches[geom_batch_local_idx], inst_state.prev_frame_transform, frame,
      packet);

    inst_state.prev_frame_transform = comp->GetEntity()->GetTransform().GetLocalToWorldMatrix();
  }

  // Now come the skinned mesh components. Because their geometry is individually animated, we cannot batch them like static meshes.
  // Each component will have its own batch.

  for (auto* const comp : skinned_mesh_components_) {
    auto const mesh{comp->GetMesh().Observe()};

    if (!mesh) {
      continue;
    }

    auto const [render_inst, is_new] = instance_registry_->CreateOrGetSkinnedInstance(comp->GetId());
    auto& inst_state = render_inst->GetState();

    if (is_new || inst_state.src_mesh_id != mesh->GetId() || inst_state.src_mesh_rev != mesh->GetRevision()) {
      SyncSkinnedInstance(*comp, *render_inst);
    }

    auto const geom_batch_local_idx = AddMeshToPacket(*mesh, frame, packet);
    auto& geom_batch = packet.geom_batches[geom_batch_local_idx];
    // Because we created a new batch for this comp, this will add a single instance to that batch.
    AddMeshComponentToPacket(*comp, geom_batch, inst_state.prev_frame_transform, frame, packet);

    inst_state.prev_frame_transform = comp->GetEntity()->GetTransform().GetLocalToWorldMatrix();

    auto const anim{comp->GetCurrentAnimation()};

    if (!anim) {
      continue;
    }

    // TODO add proper skinned mesh culling with GPU culling

    AABB const inf_aabb{
      Vector3{-std::numeric_limits<float>::infinity()},
      Vector3{std::numeric_limits<float>::infinity()}
    };

    // Set mesh AABB to infinity to prevent culling
    geom_batch.bounds = inf_aabb;

    packet.buffers.emplace_back(render_inst->GetSkinnedPositionBuffer(frame.GetIndex()));
    auto const skinned_pos_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

    auto const last_skinning_frame = render_inst->GetLastSkinningFrame();
    auto const prev_skinning_valid = last_skinning_frame && *last_skinning_frame + 1 == frame.GetNumber();
    unsigned prev_skinned_pos_buf_local_idx;

    if (prev_skinning_valid) {
      packet.buffers.emplace_back(render_inst->GetSkinnedPositionBuffer(frame.GetPreviousIndex()));
      prev_skinned_pos_buf_local_idx = static_cast<unsigned>(packet.buffers.size() - 1);
    } else {
      prev_skinned_pos_buf_local_idx = frame_packet_invalid_idx;
    }

    packet.buffers.emplace_back(render_inst->GetSkinnedNormalBuffer(frame.GetIndex()));
    auto const skinned_norm_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

    packet.buffers.emplace_back(render_inst->GetSkinnedTangentBuffer(frame.GetIndex()));
    auto const skinned_tan_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

    packet.buffers.emplace_back(render_inst->GetBoneMatrixBuffer(frame.GetIndex()));
    auto const bone_mtx_buf_local_idx{static_cast<unsigned>(packet.buffers.size() - 1)};

    // Switch the original and skinned buffer indices so that the renderer can treat the skinned mesh as static after
    // the skinning is done

    auto const orig_pos_buf_local_idx{geom_batch.pos_buf_local_idx};
    auto const orig_norm_buf_local_idx{geom_batch.norm_buf_local_idx};
    auto const orig_tan_buf_local_idx{geom_batch.tan_buf_local_idx};

    geom_batch.pos_buf_local_idx = skinned_pos_buf_local_idx;
    geom_batch.norm_buf_local_idx = skinned_norm_buf_local_idx;
    geom_batch.tan_buf_local_idx = skinned_tan_buf_local_idx;

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
      [](SkeletonNode const& node) {
        return SkeletonNodeData{node.transform, node.parent_idx};
      });

    // Extract bone data

    auto const bone_begin_local_idx{static_cast<unsigned>(packet.bone_data.size())};
    std::ranges::transform(mesh->GetBones(), std::back_inserter(packet.bone_data),
      [](Bone const& bone) {
        return BoneData{bone.offset_mtx, bone.skeleton_node_idx};
      });

    geom_batch.skinning_data_local_idx = static_cast<unsigned>(packet.skinning_data.size());

    packet.skinning_data.emplace_back(static_cast<unsigned>(geom_batch_local_idx),
      orig_pos_buf_local_idx, orig_norm_buf_local_idx, orig_tan_buf_local_idx, bone_mtx_buf_local_idx,
      prev_skinned_pos_buf_local_idx, comp->GetCurrentAnimationTime(), node_anim_begin_local_idx,
      static_cast<unsigned>(anim->node_anims.size()), skeleton_begin_local_idx,
      static_cast<unsigned>(mesh->GetSkeleton().size()), bone_begin_local_idx,
      static_cast<unsigned>(mesh->GetBones().size()));

    // Now that we have extracted the skinned mesh data, we can mark the previous skinned vertices as valid for the next frame.
    render_inst->SetLastSkinningFrame(frame.GetNumber());
  }

  // Add global render target
  auto const& global_rt{rt_override_ ? rt_override_ : main_rt_};
  packet.render_targets.emplace_back(global_rt); // The global RT is always at index 0!

  // Extract camera data

  packet.cam_data.reserve(cameras_.size());

  for (auto const cam : cameras_) {
    auto const& cam_rt{cam->GetRenderTarget()};

    if (!cam_rt && !render_global_cameras_) {
      continue;
    }

    auto const& vp{cam->GetViewport()};

    auto accum_tex_empty{false};

    if (auto const accum_rt{detail::GetTaaAccumulationRt(*cam)};
      !accum_rt || accum_rt->GetDesc().color_format != color_buffer_format_ ||
      accum_rt->GetDesc().width != (cam_rt ? cam_rt : global_rt)->GetDesc().width * GetWidth(vp) ||
      accum_rt->GetDesc().height != (cam_rt ? cam_rt : global_rt)->GetDesc().height * GetHeight(vp)) {
      detail::RecreateTaaAccumulationRt(*cam, *device_, Extent2D{
          static_cast<unsigned>((cam_rt ? cam_rt : global_rt)->GetDesc().width * GetWidth(vp)),
          static_cast<unsigned>((cam_rt ? cam_rt : global_rt)->GetDesc().height * GetHeight(vp))
        },
        color_buffer_format_);
      accum_tex_empty = true;
    }

    auto const accum_rt_local_idx{FindOrAddTextureInPacket(detail::GetTaaAccumulationRt(*cam)->GetColorTex(), packet)};

    unsigned rt_local_idx;

    if (cam_rt) {
      rt_local_idx = FindOrAddRenderTargetInPacket(cam_rt, packet);
    } else {
      rt_local_idx = 0; // The global RT is always at index 0!
    }

    packet.cam_data.emplace_back(cam->GetPosition(), cam->GetRightAxis(), cam->GetUpAxis(), cam->GetForwardAxis(),
      cam->GetNearClipPlane(), cam->GetFarClipPlane(), cam->GetType(), cam->GetVerticalPerspectiveFov(),
      cam->GetVerticalOrthographicSize(), cam->GetViewport(), rt_local_idx, accum_rt_local_idx, cam, accum_tex_empty);
  }

  // Add gizmo draw data

  packet.gizmo_data.line_count = line_gizmo_vertex_data_.size();

  if (gizmo_color_buffers_[frame.GetIndex()].GetElementCount() < gizmo_colors_.size()) {
    gizmo_color_buffers_[frame.GetIndex()].Reallocate(gizmo_colors_.size());
  }

  std::ranges::copy(gizmo_colors_, std::ranges::begin(gizmo_color_buffers_[frame.GetIndex()].GetData()));
  gizmo_colors_.clear();

  if (line_gizmo_vertex_data_buffers_[frame.GetIndex()].GetElementCount() < line_gizmo_vertex_data_.size()) {
    line_gizmo_vertex_data_buffers_[frame.GetIndex()].Reallocate(line_gizmo_vertex_data_.size());
  }

  std::ranges::copy(line_gizmo_vertex_data_,
    std::ranges::begin(line_gizmo_vertex_data_buffers_[frame.GetIndex()].GetData()));
  line_gizmo_vertex_data_.clear();

  // Copy settings

  packet.ssao_params = ssao_params_;
  packet.ssr_params = ssr_params_;
  packet.shadow_params = shadow_params_;

  packet.inv_gamma = inv_gamma_;

  packet.ssao_enabled = ssao_enabled_;
  packet.ssr_enabled = ssr_enabled_;

  packet.color_buffer_format = color_buffer_format_;

  packet.background_color = {0, 0, 0, 1};
  packet.skybox_cubemap = nullptr;
  packet.irradiance_map = nullptr;
  packet.prefiltered_env_map = nullptr;
  packet.draw_irradiance_map = false;
  packet.draw_prefiltered_env_map = false;

  if (auto* const active_scene{Scene::GetActiveScene()}) {
    packet.ambient_light = active_scene->GetAmbientLightVector();

    if (active_scene->GetSkyMode() == SkyMode::Color) {
      auto const sky_color{active_scene->GetSkyColor()};
      packet.background_color = {sky_color[0], sky_color[1], sky_color[2], 1.0F};
    }

    if (active_scene->GetSkyMode() == SkyMode::Skybox) {
      if (auto const cubemap{active_scene->GetSkybox().Observe()}) {
        packet.skybox_cubemap = cubemap->GetTex();

        if (auto const irradiance_map{sorcery::detail::GetIrradianceMap(*active_scene)};
          !irradiance_map ||
          irradiance_map->GetDesc().format != irradiance_map_format_ ||
          irradiance_map->GetDesc().width != irradiance_map_size_ ||
          irradiance_map->GetDesc().height != irradiance_map_size_) {
          sorcery::detail::RecreateIrradianceMap(*active_scene, *device_, irradiance_map_format_, irradiance_map_size_);
          packet.draw_irradiance_map = true;
        }

        if (auto const prefiltered_env_map{sorcery::detail::GetPrefilteredEnvMap(*active_scene)};
          !prefiltered_env_map ||
          prefiltered_env_map->GetDesc().format != prefiltered_env_map_format_ ||
          prefiltered_env_map->GetDesc().width != prefiltered_env_map_size_ ||
          prefiltered_env_map->GetDesc().height != prefiltered_env_map_size_) {
          sorcery::detail::RecreatePrefilteredEnvMap(*active_scene, *device_, prefiltered_env_map_format_,
            prefiltered_env_map_size_);
          packet.draw_prefiltered_env_map = true;
        }

        packet.irradiance_map = sorcery::detail::GetIrradianceMap(*active_scene);
        packet.prefiltered_env_map = sorcery::detail::GetPrefilteredEnvMap(*active_scene);
      }
    }
  }

  // Handle SSAO sample count change

  packet.upload_ssao_samples = ssao_samples_changed_;

  if (ssao_samples_changed_) {
    packet.ssao_samples = ssao_samples_;

    auto constexpr ssao_sample_element_size = sizeof(decltype(packet.ssao_samples)::value_type);

    if (auto const ssao_samples_byte_count = packet.ssao_samples.size() * ssao_sample_element_size;
      !ssao_samples_buffer_ || ssao_samples_buffer_->GetDesc().size < ssao_samples_byte_count) {
      ssao_samples_buffer_ = device_->CreateBuffer(wand::BufferDesc{
        .size = ssao_samples_byte_count,
        .stride = ssao_sample_element_size,
        .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination
      }, wand::CpuAccess::kNone);
    }

    ssao_samples_changed_ = false;
  }

  packet.ssao_samples_buf = ssao_samples_buffer_;

  // Copy used PSOs

  packet.shadow_pso = shadow_pso_;
  packet.gbuffer_velocity_pso = gbuffer_velocity_pso_;
  packet.depth_resolve_pso = depth_resolve_pso_;
  packet.line_gizmo_pso = line_gizmo_pso_;
  packet.deferred_lighting_pso = deferred_lighting_pso_;
  packet.post_process_pso = post_process_pso_;
  packet.skybox_pso = skybox_pso_;
  packet.ssao_pso = ssao_pso_;
  packet.ssao_blur_pso = ssao_blur_pso_;
  packet.ssr_compose_pso = ssr_compose_pso_;
  packet.ssr_pso = ssr_pso_;
  packet.taa_resolve_pso = taa_pso_;
  packet.vtx_skinning_pso = vtx_skinning_pso_;
  packet.irradiance_pso = irradiance_pso_;
  packet.envmap_prefilter_pso = envmap_prefilter_pso_;
}


auto SceneRenderer::PrepareFrame(RenderFrame& frame) -> void {
  auto& frame_packet{frame_packets_[frame.GetIndex()]};

  // Compute bone matrices for skinning and stage them for upload.

  for (auto& [geom_batch_local_idx, original_vertex_buf_local_idx, original_normal_buf_local_idx,
         original_tangent_buf_local_idx, bone_matrix_buf_local_idx, prev_frame_vertex_buf_local_idx, cur_animation_time,
         node_anim_begin_local_idx, node_anim_count, skeleton_begin_local_idx, skeleton_size, bone_begin_local_idx,
         bone_count] : frame_packet.skinning_data) {
    // Skip skinning when we are sitting at 0 time.
    // This happens for example in the editor scene view.
    if (cur_animation_time == 0) {
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

    frame.UploadBuffer(frame_packet.buffers[bone_matrix_buf_local_idx], 0, as_bytes(std::span{bone_matrices}));
  }

  // Upload SSAO samples if they have changed

  if (frame_packet.upload_ssao_samples) {
    frame.UploadBuffer(frame_packet.ssao_samples_buf, 0, as_bytes(std::span{frame_packet.ssao_samples}));
  }

  prepared_data_.cam_data.clear();
  prepared_data_.visible_light_indices.clear();
  prepared_data_.views.clear();

  // Prepare camera data

  auto const& prev_frame_packet{frame_packets_[frame.GetPreviousIndex()]};

  for (auto i = 0uz; i < frame_packet.cam_data.size(); ++i) {
    auto& cam_data = frame_packet.cam_data[i];
    auto const prev_cam_data = [&] {
      auto const prev_cam_it = std::ranges::find(prev_frame_packet.cam_data, cam_data.id, &CameraData::id);
      return prev_cam_it != std::ranges::end(prev_frame_packet.cam_data)
               ? &*prev_cam_it
               : nullptr;
    }();

    auto& target_rt{*frame_packet.render_targets[cam_data.rt_local_idx]};
    auto const& target_rt_desc{target_rt.GetDesc()};

    auto const target_rt_width{target_rt_desc.width};
    auto const target_rt_height{target_rt_desc.height};

    CD3DX12_VIEWPORT const cam_viewport{
      cam_data.viewport.left * static_cast<FLOAT>(target_rt_width),
      cam_data.viewport.top * static_cast<float>(target_rt_height),
      std::max(
        cam_data.viewport.right * static_cast<float>(target_rt_width) - cam_data.viewport.left * static_cast<
          FLOAT>(
          target_rt_width), 1.0f),
      std::max(
        cam_data.viewport.bottom * static_cast<float>(target_rt_height) - cam_data.viewport.top * static_cast<
          float>(
          target_rt_height), 1.0f),
    };

    CD3DX12_RECT const cam_scissor{
      static_cast<LONG>(cam_data.viewport.left * static_cast<FLOAT>(target_rt_width)),
      static_cast<LONG>(cam_data.viewport.top * static_cast<FLOAT>(target_rt_height)),
      std::max(static_cast<LONG>(cam_data.viewport.right * static_cast<FLOAT>(target_rt_width)), 1l),
      std::max(static_cast<LONG>(cam_data.viewport.bottom * static_cast<FLOAT>(target_rt_height)), 1l),
    };

    auto const cam_view_mtx = Camera::CalculateViewMatrix(cam_data.position, cam_data.right, cam_data.up,
      cam_data.forward);

    auto const prev_cam_view_mtx = prev_cam_data
                                     ? Camera::CalculateViewMatrix(prev_cam_data->position,
                                       prev_cam_data->right, prev_cam_data->up, prev_cam_data->forward)
                                     : cam_view_mtx;

    auto const viewport_aspect{cam_viewport.Width / cam_viewport.Height};

    auto const transient_rt_width{static_cast<UINT>(cam_viewport.Width)};
    auto const transient_rt_height{static_cast<UINT>(cam_viewport.Height)};

    // Jitter is defined to be in NDC [-1, 1]
    // We store this in the cam data so that we can use it in the next frame
    cam_data.jitter_ndc = [this, &frame, transient_rt_width, transient_rt_height] {
      auto const jitter_idx{frame.GetNumber() % taa_subpixel_sample_count_};

      if constexpr (true) {
        auto const [r2_x, r2_y]{R2Sequence2d(jitter_idx)};
        return Vector2{
          // R2 is in [0,1]. We are creating the jitter offset in NDC space [-1, 1].
          // In NDC space, one pixel is 2/W and 2/H in size. The valid jitter range is +- half a pixel.
          // So this offset needs to be in [-1/W, 1/W] and [-1/H, 1/H].
          (r2_x - 0.5f) * 2.0f / static_cast<float>(transient_rt_width),
          (r2_y - 0.5f) * 2.0f / static_cast<float>(transient_rt_height)
        };
      } else {
        auto const halton_x{HaltonSequence(jitter_idx + 1, 2)};
        auto const halton_y{HaltonSequence(jitter_idx + 1, 3)};
        return Vector2{
          (2 * halton_x - 1) / static_cast<float>(transient_rt_width),
          (2 * halton_y - 1) / static_cast<float>(transient_rt_height)
        };
      }
    }();

    auto const cam_proj_mtx = TransformProjectionMatrixForRendering(
      Camera::CalculateProjectionMatrix(cam_data.type, cam_data.fov_vert_deg, cam_data.size_vert,
        viewport_aspect,
        cam_data.near_plane, cam_data.far_plane) * Matrix4::Translate(Vector3{cam_data.jitter_ndc, 0}));

    auto const prev_cam_proj_mtx = prev_cam_data
                                     ? TransformProjectionMatrixForRendering(
                                       Camera::CalculateProjectionMatrix(prev_cam_data->type,
                                         prev_cam_data->fov_vert_deg, prev_cam_data->size_vert, viewport_aspect,
                                         prev_cam_data->near_plane, prev_cam_data->far_plane) * Matrix4::Translate(
                                         Vector3{prev_cam_data->jitter_ndc, 0}))
                                     : cam_proj_mtx;

    auto const cam_view_proj_mtx{cam_view_mtx * cam_proj_mtx};
    auto const prev_cam_view_proj_mtx{prev_cam_view_mtx * prev_cam_proj_mtx};

    Frustum const cam_frust_ws{cam_view_proj_mtx};

    auto const first_visible_light = static_cast<std::uint32_t>(prepared_data_.visible_light_indices.size());
    auto const visible_light_count = static_cast<std::uint32_t>(CullLights(cam_frust_ws, frame_packet.light_data,
      prepared_data_.visible_light_indices));
    auto const visible_light_indices = std::span{prepared_data_.visible_light_indices}.subspan(first_visible_light,
      visible_light_count);

    auto const primary_view_idx = static_cast<std::uint32_t>(prepared_data_.views.size());
    prepared_data_.views.emplace_back(cam_view_mtx, cam_proj_mtx, cam_view_proj_mtx, cam_frust_ws, cam_viewport,
      cam_scissor, cam_data.near_plane, cam_data.far_plane);

    auto const cascades = CalculateCameraShadowCascadeBoundaries(cam_data, frame_packet.shadow_params);
    auto const dir_shadows = PrepareDirectionalShadows(frame_packet, visible_light_indices, cam_data, cascades,
      viewport_aspect, frame_packet.shadow_params.cascade_count, dir_shadow_map_arr_->GetSize(), prepared_data_.views);

    prepared_data_.cam_data.emplace_back(cascades, dir_shadows, static_cast<std::uint32_t>(i), first_visible_light,
      visible_light_count, primary_view_idx, prev_cam_view_proj_mtx, cam_data.jitter_ndc,
      prev_cam_data ? prev_cam_data->jitter_ndc : cam_data.jitter_ndc);
  }
}


auto SceneRenderer::RecordFrame(RenderFrame& frame) -> void {
  if (!gpu_init_work_recorded_) {
    RecordGpuInitWork(frame);
    gpu_init_work_recorded_ = true;
  }

  next_per_instance_cb_idx_ = 0;
  next_per_view_cb_idx_ = 0;

  auto const frame_idx{frame.GetIndex()};

  auto& frame_packet{frame_packets_[frame_idx]};

  // Clears all render targets, dispatches skinning and draws irradiance and prefiltered env maps if needed.
  auto& prepass_cmd{frame.AcquireCommandList()};
  prepass_cmd.Begin(nullptr);

  std::ranges::for_each(frame_packet.render_targets, [&prepass_cmd](std::shared_ptr<RenderTarget> const& rt) {
    prepass_cmd.ClearRenderTarget(*rt->GetColorTex(), rt->GetDesc().color_clear_value, {});
  });

  if (!frame_packet.skinning_data.empty()) {
    prepass_cmd.SetPipelineState(*frame_packet.vtx_skinning_pso);
  }

  for (auto& [geom_batch_local_idx, original_vertex_buf_local_idx, original_normal_buf_local_idx,
         original_tangent_buf_local_idx, bone_matrix_buf_local_idx, prev_frame_vertex_buf_local_idx, cur_animation_time,
         node_anim_begin_local_idx, node_anim_count, skeleton_begin_local_idx, skeleton_size, bone_begin_local_idx,
         bone_count] : frame_packet.skinning_data) {
    auto const& geom_batch = frame_packet.geom_batches[geom_batch_local_idx];

    // Skip skinning when we are sitting at 0 time.
    // This happens for example in the editor scene view.
    if (cur_animation_time == 0) {
      prepass_cmd.CopyBuffer(
        *frame_packet.buffers[geom_batch.pos_buf_local_idx], *frame_packet.buffers[original_vertex_buf_local_idx]);
      prepass_cmd.CopyBuffer(
        *frame_packet.buffers[geom_batch.norm_buf_local_idx], *frame_packet.buffers[original_normal_buf_local_idx]);
      prepass_cmd.CopyBuffer(
        *frame_packet.buffers[geom_batch.tan_buf_local_idx], *frame_packet.buffers[original_tangent_buf_local_idx]);
      continue;
    }

    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, vtx_buf_idx),
      *frame_packet.buffers[original_vertex_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, norm_buf_idx),
      *frame_packet.buffers[original_normal_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, tan_buf_idx),
      *frame_packet.buffers[original_tangent_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, bone_weight_buf_idx),
      *frame_packet.buffers[geom_batch.bone_weight_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, bone_idx_buf_idx),
      *frame_packet.buffers[geom_batch.bone_idx_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, bone_buf_idx),
      *frame_packet.buffers[bone_matrix_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, skinned_vtx_buf_idx),
      *frame_packet.buffers[geom_batch.pos_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, skinned_norm_buf_idx),
      *frame_packet.buffers[geom_batch.norm_buf_local_idx]);
    prepass_cmd.SetUnorderedAccess(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, skinned_tan_buf_idx),
      *frame_packet.buffers[geom_batch.tan_buf_local_idx]);
    prepass_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(VertexSkinningDrawParams, vtx_count), geom_batch.vtx_count);
    prepass_cmd.Dispatch(
      static_cast<UINT>(std::ceil(static_cast<float>(geom_batch.vtx_count) / static_cast<float>(SKINNING_CS_THREADS))),
      1, 1);
  }

  if (frame_packet.draw_irradiance_map) {
    auto const& irradiance_map_desc{frame_packet.irradiance_map->GetDesc()};

    CD3DX12_VIEWPORT const irradiance_viewport{
      0.0F, 0.0F, static_cast<FLOAT>(irradiance_map_desc.width), static_cast<FLOAT>(irradiance_map_desc.height)
    };

    D3D12_RECT const irradiance_scissor{
      0, 0, static_cast<LONG>(irradiance_map_desc.width), static_cast<LONG>(irradiance_map_desc.height)
    };

    prepass_cmd.SetPipelineState(*frame_packet.irradiance_pso);
    prepass_cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    prepass_cmd.SetRenderTargets(std::array{frame_packet.irradiance_map.get()}, nullptr);
    prepass_cmd.SetViewports(std::span{static_cast<D3D12_VIEWPORT const*>(&irradiance_viewport), 1});
    prepass_cmd.SetScissorRects(std::span{&irradiance_scissor, 1});
    prepass_cmd.ClearRenderTarget(*frame_packet.irradiance_map, std::array{0.F, 0.F, 0.F, 1.F}, {});

    auto const& cube_geom = frame_packet.geom_batches[frame_packet.cube_geom_local_idx];

    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(IrradianceDrawParams, meshlet_buf_idx),
      *frame_packet.buffers[cube_geom.meshlet_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(IrradianceDrawParams, vertex_idx_buf_idx),
      *frame_packet.buffers[cube_geom.vtx_idx_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(IrradianceDrawParams, prim_idx_buf_idx),
      *frame_packet.buffers[cube_geom.prim_idx_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(IrradianceDrawParams, pos_buf_idx),
      *frame_packet.buffers[cube_geom.pos_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(IrradianceDrawParams, environment_map_idx),
      *frame_packet.skybox_cubemap);
    prepass_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(IrradianceDrawParams, bi_clamp_samp_idx),
      samp_bi_clamp_.Get());

    auto const view_matrices{MakeCubeFaceViewMatrices(Vector3::Zero())};
    auto const proj_mtx{TransformProjectionMatrixForRendering(Matrix4::PerspectiveFov(ToRadians(90), 1, .1F, 10.F))};

    for (auto i{0U}; i < 6U; i++) {
      prepass_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(IrradianceDrawParams, rt_idx), i);
      auto const view_proj_mtx{view_matrices[i] * proj_mtx};
      prepass_cmd.SetPipelineParameters(PIPELINE_PARAM_INDEX(IrradianceDrawParams, view_proj_mtx),
        std::span{std::bit_cast<UINT const*>(view_proj_mtx.GetData()), 16});

      DrawSubmesh(1, 0, 0, {}, {}, {}, prepass_cmd);
    }
  }

  if (frame_packet.draw_prefiltered_env_map) {
    prepass_cmd.SetPipelineState(*frame_packet.envmap_prefilter_pso);
    prepass_cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    auto const& cube_geom = frame_packet.geom_batches[frame_packet.cube_geom_local_idx];
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, meshlet_buf_idx),
      *frame_packet.buffers[cube_geom.meshlet_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, vertex_idx_buf_idx),
      *frame_packet.buffers[cube_geom.vtx_idx_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, prim_idx_buf_idx),
      *frame_packet.buffers[cube_geom.prim_idx_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, pos_buf_idx),
      *frame_packet.buffers[cube_geom.pos_buf_local_idx]);
    prepass_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, env_map_idx),
      *frame_packet.skybox_cubemap);
    prepass_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, tri_clamp_samp_idx),
      samp_tri_clamp_.Get());

    auto const& envmap_desc{frame_packet.prefiltered_env_map->GetDesc()};
    auto const mip_count{wand::GetActualMipLevels(envmap_desc)};

    for (UINT16 mip{0}; mip < mip_count; mip++) {
      auto const mip_scale{std::pow(0.5, mip)};
      auto const mip_width{static_cast<UINT>(envmap_desc.width * mip_scale)};
      auto const mip_height{static_cast<UINT>(envmap_desc.height * mip_scale)};

      CD3DX12_VIEWPORT const envmap_viewport{
        0.0F, 0.0F, static_cast<FLOAT>(mip_width), static_cast<FLOAT>(mip_height)
      };

      D3D12_RECT const envmap_scissor{
        0, 0, static_cast<LONG>(mip_width), static_cast<LONG>(mip_height)
      };

      prepass_cmd.SetViewports(std::span{static_cast<D3D12_VIEWPORT const*>(&envmap_viewport), 1});
      prepass_cmd.SetScissorRects(std::span{&envmap_scissor, 1});
      prepass_cmd.SetRenderTargets(std::array{frame_packet.prefiltered_env_map.get()}, nullptr, mip);
      prepass_cmd.ClearRenderTarget(*frame_packet.prefiltered_env_map, std::array{0.F, 0.F, 0.F, 1.F}, {}, mip);

      auto const roughness{static_cast<float>(mip) / static_cast<float>(mip_count - 1)};
      prepass_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, roughness),
        *std::bit_cast<UINT const*>(&roughness));

      auto const view_matrices{MakeCubeFaceViewMatrices(Vector3::Zero())};
      auto const proj_mtx{TransformProjectionMatrixForRendering(Matrix4::PerspectiveFov(ToRadians(90), 1, .1F, 10.F))};

      for (auto i{0U}; i < 6U; i++) {
        prepass_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, rt_idx), i);
        auto const view_proj_mtx{view_matrices[i] * proj_mtx};
        prepass_cmd.SetPipelineParameters(PIPELINE_PARAM_INDEX(EnvmapPrefilterDrawParams, view_proj_mtx),
          std::span{std::bit_cast<UINT const*>(view_proj_mtx.GetData()), 16});

        DrawSubmesh(1, 0, 0, {}, {}, {}, prepass_cmd);
      }
    }
  }

  prepass_cmd.End();
  frame.EnqueueCommandList(prepass_cmd);

  auto& per_frame_cb{per_frame_cbs_[frame_idx]};
  SetPerFrameConstants(per_frame_cb, frame_packet.ambient_light, frame_packet.shadow_params);

  for (auto& prepared_cam : prepared_data_.cam_data) {
    auto const& extracted_cam = frame_packet.cam_data[prepared_cam.extracted_data_idx];
    auto const& view = prepared_data_.views[prepared_cam.primary_view_idx];

    // Compute render target dimensions

    auto& target_rt{*frame_packet.render_targets[extracted_cam.rt_local_idx]};
    auto const& target_rt_desc{target_rt.GetDesc()};

    auto const target_rt_width{target_rt_desc.width};
    auto const target_rt_height{target_rt_desc.height};

    auto const transient_rt_width{static_cast<UINT>(view.viewport.Width)};
    auto const transient_rt_height{static_cast<UINT>(view.viewport.Height)};

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

    RenderTarget::Desc const velocity_rt_desc{
      transient_rt_width, transient_rt_height, velocity_format_, std::nullopt, 1,
      L"Camera Velocity RenderTarget", true, {0.0f, 0.0f, 0.0f, 0.0f}
    };

    RenderTarget::Desc const color_hdr_rt_desc{
      transient_rt_width, transient_rt_height, frame_packet.color_buffer_format, std::nullopt,
      1, L"Camera HDR RenderTarget", true, frame_packet.background_color
    };

    auto const depth_rt{render_manager_->AcquireTemporaryRenderTarget(depth_rt_desc)};
    auto const depth_sample_rt{render_manager_->AcquireTemporaryRenderTarget(depth_sample_rt_desc)};
    auto const gbuffer0_rt{render_manager_->AcquireTemporaryRenderTarget(gbuffer0_rt_desc)};
    auto const gbuffer1_rt{render_manager_->AcquireTemporaryRenderTarget(gbuffer1_rt_desc)};
    auto const gbuffer2_rt{render_manager_->AcquireTemporaryRenderTarget(gbuffer2_rt_desc)};
    auto const velocity_rt{render_manager_->AcquireTemporaryRenderTarget(velocity_rt_desc)};
    auto const color_hdr_rt{render_manager_->AcquireTemporaryRenderTarget(color_hdr_rt_desc)};


    // Command list for the camera
    auto& cam_cmd{frame.AcquireCommandList()};
    cam_cmd.Begin(nullptr);
    cam_cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    auto const cam_visible_light_indices = std::span{prepared_data_.visible_light_indices}.subspan(
      prepared_cam.first_visible_light, prepared_cam.visible_light_count);

    // Shadow pass
    if (prepared_cam.dir_shadows) {
      RecordDirectionalShadows(frame_packet, frame, *prepared_cam.dir_shadows, cam_cmd);
    }

    UpdatePositionalShadowAtlas(*pos_shadow_atlas_, frame_packet.light_data, cam_visible_light_indices,
      extracted_cam,
      view.view_proj_mtx, frame_packet.shadow_params.distance);
    DrawPositionalShadowMaps(*pos_shadow_atlas_, frame_packet, frame_idx, cam_cmd);

    auto& cam_per_view_cb{AcquirePerViewConstantBuffer(frame_idx)};
    SetPerViewConstants(cam_per_view_cb, view.view_mtx, view.proj_mtx, view.view_proj_mtx,
      prepared_cam.prev_view_proj_mtx, prepared_cam.cascade_boundaries, view.frustum_ws, extracted_cam.position,
      view.near_plane, view.far_plane, static_cast<int>(transient_rt_width), static_cast<int>(transient_rt_height));

    cam_cmd.SetViewports(std::span{static_cast<D3D12_VIEWPORT const*>(&transient_viewport), 1});
    cam_cmd.SetScissorRects(std::span{static_cast<D3D12_RECT const*>(&transient_scissor), 1});

    // GBuffer and velocity pass

    cam_cmd.SetPipelineState(*frame_packet.gbuffer_velocity_pso);

    std::array<wand::Texture const*, 4> const gbuffer_velocity_textures{
      gbuffer0_rt->GetColorTex().get(), gbuffer1_rt->GetColorTex().get(), gbuffer2_rt->GetColorTex().get(),
      velocity_rt->GetColorTex().get()
    };
    cam_cmd.SetRenderTargets(std::span{gbuffer_velocity_textures},
      depth_rt->GetDepthStencilTex().get());

    cam_cmd.ClearRenderTarget(*gbuffer0_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cam_cmd.ClearRenderTarget(*gbuffer1_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cam_cmd.ClearRenderTarget(*gbuffer2_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cam_cmd.ClearRenderTarget(*velocity_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 0.0f}, {});
    cam_cmd.ClearDepthStencil(*depth_rt->GetDepthStencilTex(), D3D12_CLEAR_FLAG_DEPTH, DEPTH_CLEAR_VALUE, 0, {});

    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, mtl_samp_idx), samp_af16_wrap_.Get());
    cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GBufferDrawParams, per_view_cb_idx), *cam_per_view_cb.GetBuffer());
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, jitter_x),
      *std::bit_cast<UINT const*>(&prepared_cam.jitter_ndc[0]));
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, jitter_y),
      *std::bit_cast<UINT const*>(&prepared_cam.jitter_ndc[1]));
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, prev_jitter_x),
      *std::bit_cast<UINT const*>(&prepared_cam.prev_jitter_ndc[0]));
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, prev_jitter_y),
      *std::bit_cast<UINT const*>(&prepared_cam.prev_jitter_ndc[1]));

    for (auto const& geom_batch : frame_packet.geom_batches) {
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, pos_buf_idx),
        *frame_packet.buffers[geom_batch.pos_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, norm_buf_idx),
        *frame_packet.buffers[geom_batch.norm_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, tan_buf_idx),
        *frame_packet.buffers[geom_batch.tan_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, uv_buf_idx),
        *frame_packet.buffers[geom_batch.uv_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, vertex_idx_buf_idx),
        *frame_packet.buffers[geom_batch.vtx_idx_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, prim_idx_buf_idx),
        *frame_packet.buffers[geom_batch.prim_idx_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, meshlet_buf_idx),
        *frame_packet.buffers[geom_batch.meshlet_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, cull_data_buf_idx),
        *frame_packet.buffers[geom_batch.cull_data_buf_local_idx]);
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, idx32), geom_batch.idx32);

      if (geom_batch.skinning_data_local_idx != frame_packet_invalid_idx) {
        if (auto const& skinning_data = frame_packet.skinning_data[geom_batch.skinning_data_local_idx];
          skinning_data.prev_frame_vertex_buf_local_idx != frame_packet_invalid_idx) {
          cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GBufferDrawParams, prev_frame_pos_buf_idx),
            *frame_packet.buffers[skinning_data.prev_frame_vertex_buf_local_idx]);
        } else {
          cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, prev_frame_pos_buf_idx),
            INVALID_RES_IDX);
        }
      } else {
        cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(GBufferDrawParams, prev_frame_pos_buf_idx),
          INVALID_RES_IDX);
      }

      auto const instances = std::span{frame_packet.instance_data}.subspan(geom_batch.first_instance,
        geom_batch.instance_count);

      auto const mtl_groups = std::span{frame_packet.mtl_slot_groups}.subspan(geom_batch.first_mtl_group,
        geom_batch.mtl_group_count);

      for (auto const& instance : instances) {
        auto& per_inst_cb{AcquirePerInstanceConstantBuffer(frame_idx)};
        SetPerInstanceConstants(per_inst_cb, instance.local_to_world_mtx, view.view_mtx, view.proj_mtx,
          instance.prev_local_to_world_mtx, instance.max_abs_scaling);
        cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GBufferDrawParams, per_inst_cb_idx),
          *per_inst_cb.GetBuffer());

        for (auto const& mtl_group : mtl_groups) {
          auto const inst_mtl_local_idx = instance.first_mtl + mtl_group.mtl_slot;
          auto const& mtl_buf_local_idx{frame_packet.instance_materials[inst_mtl_local_idx]};

          if (mtl_buf_local_idx == frame_packet_invalid_idx) {
            continue;
          }

          cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GBufferDrawParams, mtl_idx),
            *frame_packet.buffers[mtl_buf_local_idx]);

          auto const submeshes = std::span{frame_packet.submesh_data}.subspan(mtl_group.first_submesh,
            mtl_group.submesh_count);

          for (auto const& submesh : submeshes) {
            DrawSubmesh(submesh, PIPELINE_PARAM_INDEX(GBufferDrawParams, meshlet_count),
              PIPELINE_PARAM_INDEX(GBufferDrawParams, meshlet_offset),
              PIPELINE_PARAM_INDEX(GBufferDrawParams, base_vertex),
              cam_cmd);
          }
        }
      }
    }

    // Duplicate depth texture so that we can sample it and use it for depth test at the same time.
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
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SsaoDrawParams, samp_buf_idx), *frame_packet.ssao_samples_buf);
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
        std::array{static_cast<wand::Texture const*>(ssao_rt->GetColorTex().get())}.data(), 1
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
        std::array{static_cast<wand::Texture const*>(ssao_blur_rt->GetColorTex().get())}.data(), 1
      }, nullptr);
      cam_cmd.ClearRenderTarget(*ssao_blur_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});
      cam_cmd.DrawInstanced(3, 1, 0, 0);

      ssao_tex = ssao_blur_rt->GetColorTex().get();
    }

    // Deferred lighting pass

    auto const light_count = cam_visible_light_indices.size();
    std::vector<ShaderLight> shader_lights(light_count);

    for (auto i = 0uz; i < light_count; i++) {
      shader_lights[i].color = frame_packet.light_data[cam_visible_light_indices[i]].color;
      shader_lights[i].intensity = frame_packet.light_data[cam_visible_light_indices[i]].intensity;
      shader_lights[i].type = static_cast<int>(frame_packet.light_data[cam_visible_light_indices[i]].type);
      shader_lights[i].direction = frame_packet.light_data[cam_visible_light_indices[i]].direction;
      shader_lights[i].isCastingShadow = FALSE;
      shader_lights[i].range = frame_packet.light_data[cam_visible_light_indices[i]].range;
      shader_lights[i].halfInnerAngleCos = std::cos(
        ToRadians(frame_packet.light_data[cam_visible_light_indices[i]].inner_angle / 2.0f));
      shader_lights[i].halfOuterAngleCos = std::cos(
        ToRadians(frame_packet.light_data[cam_visible_light_indices[i]].outer_angle / 2.0f));
      shader_lights[i].position = frame_packet.light_data[cam_visible_light_indices[i]].position;
      shader_lights[i].depthBias = frame_packet.light_data[cam_visible_light_indices[i]].shadow_depth_bias;
      shader_lights[i].normalBias = frame_packet.light_data[cam_visible_light_indices[i]].shadow_normal_bias;

      for (auto& sample : shader_lights[i].sampleShadowMap) {
        sample = FALSE;
      }
    }

    if (prepared_cam.dir_shadows) {
      auto const visible_light_idx = prepared_cam.dir_shadows->visible_light_idx;

      auto& shadowing_shader_light = shader_lights[visible_light_idx];
      shadowing_shader_light.isCastingShadow = TRUE;

      auto const shadow_views = std::span{prepared_data_.views}.subspan(prepared_cam.dir_shadows->first_view,
        prepared_cam.dir_shadows->view_count);

      for (auto cascade_idx = 0uz; cascade_idx < shadow_views.size(); ++cascade_idx) {
        shadowing_shader_light.sampleShadowMap[cascade_idx] = TRUE;
        shadowing_shader_light.shadowViewProjMatrices[cascade_idx] = shadow_views[cascade_idx].view_proj_mtx;
      }
    }

    pos_shadow_atlas_->SetLookUpInfo(shader_lights);

    auto& light_buffer{light_buffers_[frame_idx]};

    if (light_buffer.GetElementCount() < light_count) {
      // Ignored, because light buffers are per-frame.
      std::ignore = light_buffer.Reallocate(light_count);
    }

    std::ranges::copy(shader_lights, std::ranges::begin(light_buffer.GetData()));

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
      *pos_shadow_atlas_->GetTex());
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, shadow_samp_idx),
#ifdef REVERSE_Z
      samp_cmp_pcf_ge_.Get()
#else
      samp_cmp_pcf_le_.Get()
#endif
    );
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, point_clamp_samp_idx),
      samp_point_clamp_.Get());
    if (light_buffer.GetElementCount() > 0) {
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, light_buf_idx),
        *light_buffer.GetBuffer());
    } else {
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, light_buf_idx), INVALID_RES_IDX);
    }
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, light_count),
      static_cast<UINT>(light_count));
    cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, per_view_cb_idx),
      *cam_per_view_cb.GetBuffer());
    cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, per_frame_cb_idx),
      *per_frame_cb.GetBuffer());
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, bi_clamp_samp_idx),
      samp_bi_clamp_.Get());
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, tri_clamp_samp_idx),
      samp_tri_clamp_.Get());
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, brdf_integration_map_idx),
      *brdf_integration_map_);

    if (frame_packet.irradiance_map) {
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, irradiance_map_idx),
        *frame_packet.irradiance_map);
    } else {
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, irradiance_map_idx),
        INVALID_RES_IDX);
    }

    if (frame_packet.prefiltered_env_map) {
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, prefiltered_env_map_idx),
        *frame_packet.prefiltered_env_map);
    } else {
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DeferredLightingDrawParams, prefiltered_env_map_idx),
        INVALID_RES_IDX);
    }

    cam_cmd.SetRenderTargets(std::span{
      std::array{static_cast<wand::Texture const*>(color_hdr_rt->GetColorTex().get())}.data(), 1
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
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, thickness_vs),
        *std::bit_cast<UINT const*>(&frame_packet.ssr_params.thickness_vs));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, stride),
        *std::bit_cast<UINT const*>(&frame_packet.ssr_params.stride));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, max_trace_dist_vs),
        *std::bit_cast<UINT const*>(&frame_packet.ssr_params.max_trace_dist_vs));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SsrDrawParams, ray_start_bias_vs),
        *std::bit_cast<UINT const*>(&frame_packet.ssr_params.ray_start_bias_vs));

      RenderTarget::Desc const ssr_rt_desc{
        transient_rt_width, transient_rt_height, color_buffer_format_, std::nullopt,
        1, L"SSR RT", false, std::array{0.0f, 0.0f, 0.0f, 1.0f}
      };

      auto const ssr_rt{render_manager_->AcquireTemporaryRenderTarget(ssr_rt_desc)};

      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<wand::Texture const*>(ssr_rt->GetColorTex().get())}.data(), 1
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
        std::array{static_cast<wand::Texture const*>(ssr_compose_rt->GetColorTex().get())}.data(), 1
      }, nullptr);
      cam_cmd.ClearRenderTarget(*ssr_compose_rt->GetColorTex(), ssr_compose_rt_desc.color_clear_value, {});

      cam_cmd.DrawInstanced(3, 1, 0, 0);

      cam_cmd.CopyTexture(*color_hdr_rt->GetColorTex(), *ssr_compose_rt->GetColorTex());
    }

    // Skybox pass
    if (frame_packet.skybox_cubemap) {
      cam_cmd.SetPipelineState(*frame_packet.skybox_pso);
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<wand::Texture const*>(color_hdr_rt->GetColorTex().get())}.data(), 1
      }, depth_rt->GetDepthStencilTex().get());

      auto const& cube_geom = frame_packet.geom_batches[frame_packet.cube_geom_local_idx];

      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, pos_buf_idx),
        *frame_packet.buffers[cube_geom.pos_buf_local_idx]);
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(SkyboxDrawParams, per_view_cb_idx),
        *cam_per_view_cb.GetBuffer());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, cubemap_idx),
        *frame_packet.skybox_cubemap);
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(SkyboxDrawParams, samp_idx), samp_af16_clamp_.Get());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, vertex_idx_buf_idx),
        *frame_packet.buffers[cube_geom.vtx_idx_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, prim_idx_buf_idx),
        *frame_packet.buffers[cube_geom.prim_idx_buf_local_idx]);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(SkyboxDrawParams, meshlet_buf_idx),
        *frame_packet.buffers[cube_geom.meshlet_buf_local_idx]);

      DrawSubmesh(1, 0, 0, {}, {}, {}, cam_cmd);
    }

    // TAA resolve
    if (extracted_cam.accum_tex_empty) {
      // We don't have an accumulation texture yet.
      // We can just copy the color HDR render target to the accumulation texture.
      cam_cmd.CopyTexture(*frame_packet.textures[extracted_cam.accum_tex_local_idx],
        *color_hdr_rt->GetColorTex());
    } else {
      auto& accum_tex{*frame_packet.textures[extracted_cam.accum_tex_local_idx]};
      auto const& accum_tex_desc{accum_tex.GetDesc()};

      auto const taa_rt{
        render_manager_->AcquireTemporaryRenderTarget(RenderTarget::Desc{
          accum_tex_desc.width, accum_tex_desc.height, color_buffer_format_, std::nullopt, 1,
          L"TAA Resolve RT", false, std::array{0.0f, 0.0f, 0.0f, 1.0f}
        })
      };

      cam_cmd.SetPipelineState(*frame_packet.taa_resolve_pso);
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<wand::Texture const*>(taa_rt->GetColorTex().get())}.data(), 1
      }, nullptr);

      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(TaaResolveDrawParams, color_tex_idx),
        *color_hdr_rt->GetColorTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(TaaResolveDrawParams, accum_tex_idx),
        accum_tex);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(TaaResolveDrawParams, depth_tex_idx),
        *depth_sample_rt->GetDepthStencilTex());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(TaaResolveDrawParams, velocity_tex_idx),
        *velocity_rt->GetColorTex());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(TaaResolveDrawParams, linear_samp_idx),
        samp_bi_clamp_.Get());
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(TaaResolveDrawParams, jitter_x),
        *std::bit_cast<UINT const*>(&prepared_cam.jitter_ndc[0]));
      cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(TaaResolveDrawParams, jitter_y),
        *std::bit_cast<UINT const*>(&prepared_cam.jitter_ndc[1]));

      cam_cmd.ClearRenderTarget(*taa_rt->GetColorTex(), std::array{0.0f, 0.0f, 0.0f, 1.0f}, {});
      cam_cmd.DrawInstanced(3, 1, 0, 0);
      cam_cmd.CopyTexture(accum_tex, *taa_rt->GetColorTex());
    }

    // Post-processing pass

    auto const* const post_process_input_tex{frame_packet.textures[extracted_cam.accum_tex_local_idx].get()};

    cam_cmd.SetViewports(std::span{&view.viewport, 1});
    cam_cmd.SetScissorRects(std::span{&view.scissor, 1});

    cam_cmd.SetPipelineState(*frame_packet.post_process_pso);
    cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(PostProcessDrawParams, in_tex_idx), *post_process_input_tex);
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(PostProcessDrawParams, inv_gamma),
      *std::bit_cast<UINT*>(&frame_packet.inv_gamma));
    cam_cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(PostProcessDrawParams, bi_clamp_samp_idx), samp_bi_clamp_.Get());
    cam_cmd.SetRenderTargets(std::span{
      std::array{static_cast<wand::Texture const*>(target_rt.GetColorTex().get())}.data(), 1
    }, nullptr);
    cam_cmd.DrawInstanced(3, 1, 0, 0);

    // Gizmo pass

    if (frame_packet.gizmo_data.line_count > 0) {
      cam_cmd.SetPipelineState(*frame_packet.line_gizmo_pso);
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GizmoDrawParams, vertex_buf_idx),
        *line_gizmo_vertex_data_buffers_[frame_idx].GetBuffer());
      cam_cmd.SetShaderResource(PIPELINE_PARAM_INDEX(GizmoDrawParams, color_buf_idx),
        *gizmo_color_buffers_[frame_idx].GetBuffer());
      cam_cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(GizmoDrawParams, per_view_cb_idx),
        *cam_per_view_cb.GetBuffer());
      cam_cmd.SetRenderTargets(std::span{
        std::array{static_cast<wand::Texture const*>(target_rt.GetColorTex().get())}.data(), 1
      }, nullptr);
      cam_cmd.SetScissorRects(std::span{&view.scissor, 1});
      cam_cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
      cam_cmd.DrawInstanced(2, static_cast<UINT>(frame_packet.gizmo_data.line_count), 0, 0);
    }

    cam_cmd.End();
    frame.EnqueueCommandList(cam_cmd);
  }
}


auto SceneRenderer::DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void {
  gizmo_colors_.emplace_back(color);
  line_gizmo_vertex_data_.emplace_back(from, static_cast<std::uint32_t>(gizmo_colors_.size() - 1), to, 0.0f);
}


auto SceneRenderer::IsRenderingGlobalCameras() const noexcept -> bool {
  return render_global_cameras_;
}


auto SceneRenderer::SetRenderGlobalCameras(bool const render) noexcept -> void {
  render_global_cameras_ = render;
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


auto SceneRenderer::Register(StaticMeshComponent& static_mesh_component) noexcept -> void {
  static_mesh_components_.emplace_back(std::addressof(static_mesh_component));
}


auto SceneRenderer::Unregister(StaticMeshComponent const& static_mesh_component) noexcept -> void {
  std::erase(static_mesh_components_, std::addressof(static_mesh_component));
}


auto SceneRenderer::Register(SkinnedMeshComponent& skinned_mesh_component) noexcept -> void {
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


auto SceneRenderer::Register(Camera& cam) noexcept -> void {
  cameras_.emplace_back(&cam);
}


auto SceneRenderer::Unregister(Camera const& cam) noexcept -> void {
  std::erase(cameras_, &cam);
}


auto SceneRenderer::FindOrAddBufferInPacket(wand::SharedDeviceChildHandle<wand::Buffer> const& buf,
                                            ExtractedFrameData& packet) -> std::uint32_t {
  std::uint32_t idx;

  if (auto const it{std::ranges::find(packet.buffers, buf)}; it != std::ranges::end(packet.buffers)) {
    idx = static_cast<uint32_t>(it - packet.buffers.begin());
  } else {
    idx = static_cast<uint32_t>(packet.buffers.size());
    packet.buffers.emplace_back(buf);
  }

  return idx;
}


auto SceneRenderer::FindOrAddTextureInPacket(wand::SharedDeviceChildHandle<wand::Texture> const& tex,
                                             ExtractedFrameData& packet) -> std::uint32_t {
  std::uint32_t idx;

  if (auto const it{std::ranges::find(packet.textures, tex)}; it != std::ranges::end(packet.textures)) {
    idx = static_cast<uint32_t>(it - packet.textures.begin());
  } else {
    idx = static_cast<uint32_t>(packet.textures.size());
    packet.textures.emplace_back(tex);
  }

  return idx;
}


auto SceneRenderer::AddMeshToPacket(Mesh& mesh, RenderFrame& frame, ExtractedFrameData& packet) const -> std::uint32_t {
  auto const [render_mesh, is_new] = resource_registry_->CreateOrGetMesh(mesh.GetId());

  if (is_new || mesh.GetRevision() != render_mesh->GetRevision()) {
    SyncMesh(mesh, *render_mesh, frame);
  }

  auto const ret = static_cast<unsigned>(packet.geom_batches.size());

  auto const pos_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetPositionBuffer(), packet)};
  auto const norm_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetNormalBuffer(), packet)};
  auto const tan_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetTangentBuffer(), packet)};
  auto const uv_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetUvBuffer(), packet)};
  auto const meshlet_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetMeshletBuffer(), packet)};
  auto const vtx_idx_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetVertexIndexBuffer(), packet)};
  auto const prim_idx_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetPrimitiveIndexBuffer(), packet)};
  auto const cull_data_buf_local_idx{FindOrAddBufferInPacket(render_mesh->GetCullDataBuffer(), packet)};

  auto const& bone_weight_buf = render_mesh->GetBoneWeightBuffer();
  auto const bone_weight_buf_local_idx = bone_weight_buf
                                           ? FindOrAddBufferInPacket(bone_weight_buf, packet)
                                           : frame_packet_invalid_idx;

  auto const& bone_idx_buf = render_mesh->GetBoneIndexBuffer();
  auto const bone_idx_buf_local_idx = bone_idx_buf
                                        ? FindOrAddBufferInPacket(bone_idx_buf, packet)
                                        : frame_packet_invalid_idx;

  auto const mtl_slots = mesh.GetMaterialSlots();
  auto const submeshes = mesh.GetSubmeshes();

  auto const first_mtl_group = static_cast<unsigned>(packet.mtl_slot_groups.size());
  auto mtl_group_count = 0u;

  for (auto i = 0u; i < static_cast<unsigned>(mtl_slots.size()); ++i) {
    auto first_submesh = static_cast<unsigned>(packet.submesh_data.size());
    auto submesh_count = 0u;

    for (auto const& submesh : submeshes) {
      if (submesh.GetMaterialIndex() == i) {
        packet.submesh_data.emplace_back(submesh.GetFirstMeshlet(), submesh.GetMeshletCount(),
          submesh.GetBaseVertex(), submesh.GetBounds());

        ++submesh_count;
      }
    }

    if (submesh_count != 0) {
      packet.mtl_slot_groups.emplace_back(i, first_submesh, submesh_count);
      ++mtl_group_count;
    }
  }

  // Deliberately setting instance_count to 0, as it will be set later when instances are added.
  // Skinning data is also set to invalid. It will be patched when extracting skinned meshes.
  packet.geom_batches.emplace_back(pos_buf_local_idx, norm_buf_local_idx, tan_buf_local_idx, uv_buf_local_idx,
    bone_weight_buf_local_idx, bone_idx_buf_local_idx, meshlet_buf_local_idx, vtx_idx_buf_local_idx,
    prim_idx_buf_local_idx, cull_data_buf_local_idx, first_mtl_group, mtl_group_count,
    static_cast<unsigned>(packet.instance_data.size()), 0u, frame_packet_invalid_idx,
    mesh.GetBounds(), static_cast<unsigned>(mesh.GetVertexCount()), mesh.GetId(), mesh.Has32BitVertexIndices());

  return ret;
}


auto SceneRenderer::FindOrAddMeshInPacket(Mesh& mesh, RenderFrame& frame, ExtractedFrameData& packet) -> std::uint32_t {
  if (auto const it{std::ranges::find(packet.geom_batches, mesh.GetId(), &GeometryBatch::src_mesh_id)};
    it != std::ranges::end(packet.geom_batches)) {
    return static_cast<std::uint32_t>(it - std::ranges::begin(packet.geom_batches));
  }

  return AddMeshToPacket(mesh, frame, packet);
}


auto SceneRenderer::AddMeshComponentToPacket(MeshComponentBase const& comp, GeometryBatch& geom_batch,
                                             Matrix4 const& prev_local_to_world_mtx, RenderFrame& frame,
                                             ExtractedFrameData& packet) const -> void {
  auto const& materials = comp.GetMaterials();
  auto const first_mtl = static_cast<unsigned>(packet.instance_materials.size());
  auto const mtl_count = static_cast<unsigned>(materials.size());

  for (auto const& mtl_ref : materials) {
    auto const mtl = mtl_ref.Observe();

    if (!mtl) {
      packet.instance_materials.emplace_back(frame_packet_invalid_idx);
      continue;
    }

    auto const [render_mtl, is_new] = resource_registry_->CreateOrGetMaterial(mtl->GetId());

    if (is_new || mtl->GetRevision() != render_mtl->GetRevision()) {
      SyncMaterial(*mtl, *render_mtl, frame);
    }

    packet.instance_materials.emplace_back(FindOrAddBufferInPacket(render_mtl->GetBuffer(), packet));

    for (auto const tex : {
           mtl->GetAlbedoMap().Observe(),
           mtl->GetMetallicMap().Observe(),
           mtl->GetRoughnessMap().Observe(),
           mtl->GetAoMap().Observe(),
           mtl->GetNormalMap().Observe(),
           mtl->GetOpacityMask().Observe()
         }) {
      if (tex) {
        // Return ignored because index is irrelevant, we call this only to make sure the packet contains the textures
        // the material references.
        std::ignore = FindOrAddTextureInPacket(tex->GetTex(), packet);
      }
    }
  }

  auto const& transform{comp.GetEntity()->GetTransform()};
  auto const local_to_world_mtx{transform.GetLocalToWorldMatrix()};
  auto const scaling{transform.GetWorldScale()};
  auto const max_abs_scale{std::max({std::abs(scaling[0]), std::abs(scaling[1]), std::abs(scaling[2])})};

  // We place instances linearly and keep incrementing the instance count of the associated batch.
  // The caller can create an instance list for a batch by repeatedly calling this function for all instances in-order.
  packet.instance_data.emplace_back(local_to_world_mtx, prev_local_to_world_mtx, max_abs_scale, first_mtl, mtl_count);
  ++geom_batch.instance_count;
}


auto SceneRenderer::FindOrAddRenderTargetInPacket(std::shared_ptr<RenderTarget> const& rt,
                                                  ExtractedFrameData& packet) -> std::uint32_t {
  std::uint32_t idx;

  if (auto const it{std::ranges::find(packet.render_targets, rt)}; it != std::ranges::end(packet.render_targets)) {
    idx = static_cast<std::uint32_t>(it - packet.render_targets.begin());
  } else {
    idx = static_cast<std::uint32_t>(packet.render_targets.size());
    packet.render_targets.emplace_back(rt);
  }

  return idx;
}


auto SceneRenderer::SyncMaterial(Material const& mtl, RenderMaterial& render_mtl, RenderFrame& frame) -> void {
  auto const albedo_map = mtl.GetAlbedoMap();
  auto const metallic_map = mtl.GetMetallicMap();
  auto const roughness_map = mtl.GetRoughnessMap();
  auto const ao_map = mtl.GetAoMap();
  auto const normal_map = mtl.GetNormalMap();
  auto const opacity_map = mtl.GetOpacityMask();

  ShaderMaterial const shader_mtl{
    .albedo = mtl.GetAlbedoVector(),
    .metallic = mtl.GetMetallic(),
    .roughness = mtl.GetRoughness(),
    .ao = mtl.GetAo(),
    .alphaThreshold = mtl.GetAlphaThreshold(),
    .albedo_map_idx = albedo_map ? albedo_map->GetTex()->GetShaderResource() : INVALID_RES_IDX,
    .metallic_map_idx = metallic_map ? metallic_map->GetTex()->GetShaderResource() : INVALID_RES_IDX,
    .roughness_map_idx = roughness_map ? roughness_map->GetTex()->GetShaderResource() : INVALID_RES_IDX,
    .ao_map_idx = ao_map ? ao_map->GetTex()->GetShaderResource() : INVALID_RES_IDX,
    .normal_map_idx = normal_map ? normal_map->GetTex()->GetShaderResource() : INVALID_RES_IDX,
    .opacity_map_idx = opacity_map ? opacity_map->GetTex()->GetShaderResource() : INVALID_RES_IDX,
    .blendMode = ToShaderBlendMode(mtl.GetBlendMode()),
    .pad = {}
  };

  frame.UploadBuffer(render_mtl.GetBuffer(), 0,
    std::span{reinterpret_cast<std::byte const*>(&shader_mtl), sizeof(shader_mtl)});

  render_mtl.SetRevision(mtl.GetRevision());
}


auto SceneRenderer::SyncMesh(Mesh& mesh, RenderMesh& render_mesh, RenderFrame& frame) const -> void {
  auto const mesh_data = mesh.GetData();
  assert(mesh_data);

  render_mesh.Init(*device_, mesh_data->positions.size(), mesh_data->meshlets.size(), mesh_data->vertex_indices.size(),
    mesh_data->triangle_indices.size(), mesh_data->cull_data.size(), !mesh_data->bone_weights.empty());

  auto const to_vec4{
    [](std::span<Vector3 const> const vectors, float const component4,
       std::vector<Vector4>& out) -> std::vector<Vector4>& {
      out.reserve(vectors.size());
      out.clear();

      std::ranges::transform(vectors, std::back_inserter(out), [component4](Vector3 const vec3) {
        return Vector4{vec3, component4};
      });

      return out;
    }
  };

  std::vector<Vector4> vec4_buf;

  frame.UploadBuffer(render_mesh.GetPositionBuffer(), 0,
    as_bytes(std::span{(to_vec4(mesh_data->positions, 1, vec4_buf))}));
  frame.UploadBuffer(render_mesh.GetNormalBuffer(), 0,
    as_bytes(std::span{(to_vec4(mesh_data->normals, 0, vec4_buf))}));
  frame.UploadBuffer(render_mesh.GetTangentBuffer(), 0,
    as_bytes(std::span{(to_vec4(mesh_data->tangents, 0, vec4_buf))}));
  frame.UploadBuffer(render_mesh.GetUvBuffer(), 0, as_bytes(std::span{mesh_data->uvs}));

  if (!mesh_data->bone_weights.empty()) {
    frame.UploadBuffer(render_mesh.GetBoneWeightBuffer(), 0, as_bytes(std::span{mesh_data->bone_weights}));
  }

  if (!mesh_data->bone_indices.empty()) {
    frame.UploadBuffer(render_mesh.GetBoneIndexBuffer(), 0, as_bytes(std::span{mesh_data->bone_indices}));
  }

  frame.UploadBuffer(render_mesh.GetMeshletBuffer(), 0, as_bytes(std::span{mesh_data->meshlets}));
  frame.UploadBuffer(render_mesh.GetVertexIndexBuffer(), 0, as_bytes(std::span{mesh_data->vertex_indices}));
  frame.UploadBuffer(render_mesh.GetPrimitiveIndexBuffer(), 0, as_bytes(std::span{mesh_data->triangle_indices}));
  frame.UploadBuffer(render_mesh.GetCullDataBuffer(), 0, as_bytes(std::span{mesh_data->cull_data}));

  render_mesh.SetRevision(mesh.GetRevision());

  if (mesh.GetCpuDataPolicy() == CpuResidencyPolicy::kReleaseAfterUpload) {
    sorcery::detail::ClearMeshCpuData(mesh);
  }
}


auto SceneRenderer::SyncStaticInstance(StaticMeshComponent const& comp, StaticRenderMeshInstance& inst) -> void {
  auto const mesh = comp.GetMesh();
  assert(mesh);

  auto& state = inst.GetState();
  state.src_mesh_id = mesh->GetId();
  state.src_mesh_rev = mesh->GetRevision();
  state.prev_frame_transform = comp.GetEntity()->GetTransform().GetLocalToWorldMatrix();
}


auto SceneRenderer::SyncSkinnedInstance(SkinnedMeshComponent const& comp,
                                        SkinnedRenderMeshInstance& inst) const -> void {
  auto const mesh = comp.GetMesh();
  assert(mesh);

  if (mesh->GetVertexCount() > inst.GetVertexCapacity() || mesh->GetBones().size() > inst.GetBoneCapacity()) {
    inst.Init(*device_, mesh->GetVertexCount(), mesh->GetBones().size());
  }

  inst.InvalidateSkinningHistory();

  auto& state = inst.GetState();
  state.src_mesh_id = mesh->GetId();
  state.src_mesh_rev = mesh->GetRevision();
  state.prev_frame_transform = comp.GetEntity()->GetTransform().GetLocalToWorldMatrix();
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
                               std::vector<unsigned>& visible_light_indices) -> uint64_t {
  std::uint64_t light_count = 0;

  for (unsigned light_idx = 0; light_idx < static_cast<unsigned>(lights.size()); light_idx++) {
    switch (auto const light{lights[light_idx]}; light.type) {
      case LightComponent::Type::Directional: {
        visible_light_indices.emplace_back(light_idx);
        ++light_count;
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
          ++light_count;
        }

        break;
      }

      case LightComponent::Type::Point: {
        if (BoundingSphere const bounds_ws{Vector3{light.position}, light.range}; frustum_ws.Intersects(bounds_ws)) {
          visible_light_indices.emplace_back(light_idx);
          ++light_count;
        }
        break;
      }
    }
  }

  return light_count;
}


auto SceneRenderer::PrepareDirectionalShadows(
  ExtractedFrameData const& frame_packet,
  std::span<unsigned const> const cam_visible_light_indices,
  CameraData const& cam_data,
  ShadowCascadeBoundaries const& shadow_cascade_boundaries,
  float const rt_aspect,
  std::uint32_t const cascade_count,
  std::uint32_t const shadow_map_size,
  std::vector<PreparedView>& views
) -> std::optional<PreparedDirectionalShadows> {
  std::optional<std::uint32_t> shadow_casting_light_idx;
  auto const first_view = static_cast<std::uint32_t>(views.size());

  for (auto i = 0u; i < static_cast<std::uint32_t>(cam_visible_light_indices.size()); ++i) {
    auto const light_idx = cam_visible_light_indices[i];

    if (auto const light = frame_packet.light_data[light_idx];
      light.type == LightComponent::Type::Directional && light.casts_shadow) {
      enum FrustumVertex : std::uint8_t {
        kFrustumVertexNearTopRight    = 0,
        kFrustumVertexNearTopLeft     = 1,
        kFrustumVertexNearBottomLeft  = 2,
        kFrustumVertexNearBottomRight = 3,
        kFrustumVertexFarTopRight     = 4,
        kFrustumVertexFarTopLeft      = 5,
        kFrustumVertexFarBottomLeft   = 6,
        kFrustumVertexFarBottomRight  = 7,
      };

      auto const cam_near = cam_data.near_plane;
      auto const cam_far = cam_data.far_plane;

      // Order of vertices is CCW from top right, near first
      auto const frustum_verts_ws = [&cam_data, rt_aspect, cam_near, cam_far] {
        std::array<Vector3, 8> ret;

        auto const near_world_forward = cam_data.position + cam_data.forward * cam_near;
        auto const far_world_forward = cam_data.position + cam_data.forward * cam_far;

        switch (cam_data.type) {
          case Camera::Type::Perspective: {
            auto const tan_half_fov = std::tan(ToRadians(cam_data.fov_vert_deg / 2.0f));
            auto const near_extent_y = cam_near * tan_half_fov;
            auto const near_extent_x = near_extent_y * rt_aspect;
            auto const far_extent_y = cam_far * tan_half_fov;
            auto const far_extent_x = far_extent_y * rt_aspect;

            ret[kFrustumVertexNearTopRight] = near_world_forward + cam_data.right * near_extent_x + cam_data.up *
                                              near_extent_y;
            ret[kFrustumVertexNearTopLeft] = near_world_forward - cam_data.right * near_extent_x + cam_data.up *
                                             near_extent_y;
            ret[kFrustumVertexNearBottomLeft] = near_world_forward - cam_data.right * near_extent_x - cam_data.up *
                                                near_extent_y;
            ret[kFrustumVertexNearBottomRight] =
              near_world_forward + cam_data.right * near_extent_x - cam_data.up * near_extent_y;
            ret[kFrustumVertexFarTopRight] = far_world_forward + cam_data.right * far_extent_x + cam_data.up *
                                             far_extent_y;
            ret[kFrustumVertexFarTopLeft] = far_world_forward - cam_data.right * far_extent_x + cam_data.up *
                                            far_extent_y;
            ret[kFrustumVertexFarBottomLeft] = far_world_forward - cam_data.right * far_extent_x - cam_data.up *
                                               far_extent_y;
            ret[kFrustumVertexFarBottomRight] = far_world_forward + cam_data.right * far_extent_x - cam_data.up *
                                                far_extent_y;
            break;
          }
          case Camera::Type::Orthographic: {
            auto const extent_x = cam_data.size_vert / 2.0f;
            auto const extent_y = extent_x / rt_aspect;

            ret[kFrustumVertexNearTopRight] = near_world_forward + cam_data.right * extent_x + cam_data.up * extent_y;
            ret[kFrustumVertexNearTopLeft] = near_world_forward - cam_data.right * extent_x + cam_data.up * extent_y;
            ret[kFrustumVertexNearBottomLeft] = near_world_forward - cam_data.right * extent_x - cam_data.up * extent_y;
            ret[kFrustumVertexNearBottomRight] =
              near_world_forward + cam_data.right * extent_x - cam_data.up * extent_y;
            ret[kFrustumVertexFarTopRight] = far_world_forward + cam_data.right * extent_x + cam_data.up * extent_y;
            ret[kFrustumVertexFarTopLeft] = far_world_forward - cam_data.right * extent_x + cam_data.up * extent_y;
            ret[kFrustumVertexFarBottomLeft] = far_world_forward - cam_data.right * extent_x - cam_data.up * extent_y;
            ret[kFrustumVertexFarBottomRight] = far_world_forward + cam_data.right * extent_x - cam_data.up * extent_y;
            break;
          }
        }

        return ret;
      }();

      auto const frustum_depth = cam_far - cam_near;

      for (auto cascade_idx = 0u; cascade_idx < cascade_count; cascade_idx++) {
        // cascade vertices in world space
        auto const cascade_verts_ws =
          [&frustum_verts_ws, &shadow_cascade_boundaries, cascade_idx, cam_near, frustum_depth] {
            auto const [cascade_near, cascade_far] = shadow_cascade_boundaries[cascade_idx];

            auto const cascade_near_norm = (cascade_near - cam_near) / frustum_depth;
            auto const cascade_far_norm = (cascade_far - cam_near) / frustum_depth;

            std::array<Vector3, 8> ret;

            for (auto j = 0; j < 4; j++) {
              auto const& from = frustum_verts_ws[j];
              auto const& to = frustum_verts_ws[j + 4];

              ret[j] = Lerp(from, to, cascade_near_norm);
              ret[j + 4] = Lerp(from, to, cascade_far_norm);
            }

            return ret;
          }();

        auto cascade_center_ws = Vector3::Zero();

        for (auto const& cascade_vert_ws : cascade_verts_ws) {
          cascade_center_ws += cascade_vert_ws;
        }

        cascade_center_ws /= 8.0f;

        auto sphere_radius = 0.0f;

        for (auto const& cascade_vert_ws : cascade_verts_ws) {
          sphere_radius = std::max(sphere_radius, Distance(cascade_center_ws, cascade_vert_ws));
        }

        auto const world_units_per_texel = sphere_radius * 2.0f / static_cast<float>(shadow_map_size);

        auto const up = [&light] {
          auto const dot{Dot(light.direction, Vector3::Up())};
          return Approximately(dot, 1.0F)
                   ? Vector3::Backward()
                   : Approximately(dot, -1.0F)
                       ? Vector3::Forward()
                       : Vector3::Up();
        }();

        auto shadow_view_mtx = Matrix4::LookTo(Vector3::Zero(), light.direction, up);
        cascade_center_ws = Vector3{Vector4{cascade_center_ws, 1} * shadow_view_mtx};
        cascade_center_ws /= world_units_per_texel;
        cascade_center_ws[0] = std::floor(cascade_center_ws[0]);
        cascade_center_ws[1] = std::floor(cascade_center_ws[1]);
        cascade_center_ws *= world_units_per_texel;
        // shadowViewMtx is only rotation, transpose is its inverse
        cascade_center_ws = Vector3{Vector4{cascade_center_ws, 1} * shadow_view_mtx.Transpose()};

        auto const shadow_near_clip = -sphere_radius - light.shadow_extension;
        auto const shadow_far_clip = sphere_radius;

        shadow_view_mtx = Matrix4::LookTo(cascade_center_ws, light.direction, up);
        auto const shadow_proj_mtx = TransformProjectionMatrixForRendering(Matrix4::OrthographicOffCenter(
          -sphere_radius, sphere_radius, sphere_radius, -sphere_radius, shadow_near_clip, shadow_far_clip));

        auto const shadow_view_proj_mtx = shadow_view_mtx * shadow_proj_mtx;

        Frustum const shadow_frustum_ws{shadow_view_proj_mtx};

        D3D12_VIEWPORT const shadow_viewport{
          .TopLeftX = 0,
          .TopLeftY = 0,
          .Width = static_cast<float>(shadow_map_size),
          .Height = static_cast<float>(shadow_map_size),
          .MinDepth = 0,
          .MaxDepth = 1
        };

        D3D12_RECT const shadow_scissor{
          .left = 0,
          .top = 0,
          .right = static_cast<LONG>(shadow_map_size),
          .bottom = static_cast<LONG>(shadow_map_size)
        };

        views.emplace_back(shadow_view_mtx, shadow_proj_mtx, shadow_view_proj_mtx, shadow_frustum_ws, shadow_viewport,
          shadow_scissor, shadow_near_clip, shadow_far_clip);
      }

      shadow_casting_light_idx = i;
      break;
    }
  }

  if (!shadow_casting_light_idx) {
    return std::nullopt;
  }

  return PreparedDirectionalShadows{
    .visible_light_idx = *shadow_casting_light_idx,
    .first_view = first_view,
    .view_count = cascade_count
  };
}


auto SceneRenderer::RecordDirectionalShadows(
  ExtractedFrameData const& frame_packet,
  RenderFrame const& frame,
  PreparedDirectionalShadows const& shadows,
  wand::CommandList& cmd
) -> void {
  cmd.SetPipelineState(*frame_packet.shadow_pso);
  cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, samp_idx), samp_af16_wrap_.Get());
  cmd.SetRenderTargets({}, dir_shadow_map_arr_->GetTex().get());
  cmd.ClearDepthStencil(*dir_shadow_map_arr_->GetTex(), D3D12_CLEAR_FLAG_DEPTH, DEPTH_CLEAR_VALUE, 0, {});

  auto const views = std::span{prepared_data_.views}.subspan(shadows.first_view, shadows.view_count);

  for (auto i = 0u; i < static_cast<std::uint32_t>(views.size()); ++i) {
    RecordDepthOnlyPass(frame_packet, frame, views[i], i, cmd);
  }
}


auto SceneRenderer::SetPerFrameConstants(MappedConstantBuffer<ShaderPerFrameConstants>& cb,
                                         Vector3 const& ambient_light, ShadowParams const& shadow_params) -> void {
  cb.GetData() = ShaderPerFrameConstants{
    .ambientLightColor = ambient_light, .shadowCascadeCount = shadow_params.cascade_count,
    .visualizeShadowCascades = shadow_params.visualize_cascades,
    .shadowFilteringMode = static_cast<int>(shadow_params.filtering_mode)
  };
}


auto SceneRenderer::SetPerViewConstants(
  MappedConstantBuffer<ShaderPerViewConstants>& cb,
  Matrix4 const& view_mtx,
  Matrix4 const& proj_mtx,
  Matrix4 const& view_proj_mtx,
  Matrix4 const& prev_view_proj_mtx,
  ShadowCascadeBoundaries const& cascade_bounds,
  Frustum const& frustum_ws,
  Vector3 const& view_pos,
  float const near_clip_plane,
  float const far_clip_plane,
  int const rt_width,
  int const rt_height
) -> void {
  ShaderPerViewConstants data;
  data.viewMtx = view_mtx;
  data.invViewMtx = view_mtx.Inverse();
  data.projMtx = proj_mtx;
  data.invProjMtx = proj_mtx.Inverse();
  data.viewProjMtx = view_proj_mtx;
  data.invViewProjMtx = data.viewProjMtx.Inverse();
  data.prev_view_proj_mtx = prev_view_proj_mtx;

  for (auto i = 0; i < 6; i++) {
    data.frustum_planes_ws[i] = frustum_ws.GetPlanes()[i];
  }

  data.viewPos = view_pos;
  data.near_clip_plane = near_clip_plane;
  data.far_clip_plane = far_clip_plane;
  data.screenSize = Vector2{rt_width, rt_height};

  for (auto i = 0; i < MAX_CASCADE_COUNT; i++) {
    data.shadowCascadeSplitDistances[i] = cascade_bounds[i].farClip;
  }

  cb.GetData() = data;
}


auto SceneRenderer::SetPerInstanceConstants(MappedConstantBuffer<ShaderPerInstanceConstants>& cb,
                                            Matrix4 const& model_mtx,
                                            Matrix4 const& view_mtx, Matrix4 const& proj_mtx,
                                            Matrix4 const& prev_model_mtx, float const max_abs_scaling) -> void {
  cb.GetData() = ShaderPerInstanceConstants{
    .modelMtx = model_mtx, .invTranspModelMtx = model_mtx.Inverse().Transpose(), .model_view_mtx = model_mtx * view_mtx,
    .model_view_proj_mtx = model_mtx * view_mtx * proj_mtx, .prev_model_mtx = prev_model_mtx,
    .max_abs_scaling = max_abs_scaling
  };
}


auto SceneRenderer::UpdatePositionalShadowAtlas(
  PositionalLightShadowAtlas& atlas,
  std::span<LightData const> const lights,
  std::span<unsigned const> visible_light_indices,
  CameraData const& cam_data,
  Matrix4 const& cam_view_proj_mtx,
  float const shadow_distance
) -> void {
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
            faceBoundsRotations[j].Rotate(Vector3{lightRange, lightRange, lightRange}) + lightPos,
            faceBoundsRotations[j].Rotate(Vector3{-lightRange, lightRange, lightRange}) + lightPos,
            faceBoundsRotations[j].Rotate(Vector3{-lightRange, -lightRange, lightRange}) + lightPos,
            faceBoundsRotations[j].Rotate(Vector3{lightRange, -lightRange, lightRange}) + lightPos, lightPos,
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


auto SceneRenderer::DrawPositionalShadowMaps(
  PositionalLightShadowAtlas const& atlas,
  ExtractedFrameData const& frame_packet,
  std::uint32_t const frame_idx,
  wand::CommandList& cmd
) -> void {
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

        Frustum const shadow_frustum_ws{subcell->shadowViewProjMtx};

        auto& per_view_cb{AcquirePerViewConstantBuffer(frame_idx)};
        SetPerViewConstants(per_view_cb, Matrix4::Identity(), subcell->shadowViewProjMtx, subcell->shadowViewProjMtx,
          {}, ShadowCascadeBoundaries{}, shadow_frustum_ws, Vector3{}, 0, 0, 0,
          0); // TODO pass proper near and far clip planes and rt size
        cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_view_cb_idx), *per_view_cb.GetBuffer());

        for (auto const& geom_batch : frame_packet.geom_batches) {
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, pos_buf_idx),
            *frame_packet.buffers[geom_batch.pos_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, uv_buf_idx),
            *frame_packet.buffers[geom_batch.uv_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, vertex_idx_buf_idx),
            *frame_packet.buffers[geom_batch.vtx_idx_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, prim_idx_buf_idx),
            *frame_packet.buffers[geom_batch.prim_idx_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_buf_idx),
            *frame_packet.buffers[geom_batch.meshlet_buf_local_idx]);
          cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, cull_data_buf_idx),
            *frame_packet.buffers[geom_batch.cull_data_buf_local_idx]);
          cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, idx32), geom_batch.idx32);

          auto const instances = std::span{frame_packet.instance_data}.subspan(geom_batch.first_instance,
            geom_batch.instance_count);

          auto const mtl_groups = std::span{frame_packet.mtl_slot_groups}.subspan(geom_batch.first_mtl_group,
            geom_batch.mtl_group_count);

          for (auto const& instance : instances) {
            auto& per_inst_cb{AcquirePerInstanceConstantBuffer(frame_idx)};
            SetPerInstanceConstants(per_inst_cb, instance.local_to_world_mtx, Matrix4::Identity(),
              subcell->shadowViewProjMtx, {}, instance.max_abs_scaling);
            cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_inst_cb_idx),
              *per_inst_cb.GetBuffer());

            for (auto const& mtl_group : mtl_groups) {
              auto const inst_mtl_local_idx = instance.first_mtl + mtl_group.mtl_slot;
              auto const& mtl_buf_local_idx{frame_packet.instance_materials[inst_mtl_local_idx]};

              if (mtl_buf_local_idx == frame_packet_invalid_idx) {
                continue;
              }

              cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, mtl_idx),
                *frame_packet.buffers[mtl_buf_local_idx]);

              auto const submeshes = std::span{frame_packet.submesh_data}.subspan(mtl_group.first_submesh,
                mtl_group.submesh_count);

              for (auto const& submesh : submeshes) {
                DrawSubmesh(submesh, PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_count),
                  PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_offset),
                  PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, base_vertex),
                  cmd);
              }
            }
          }
        }
      }
    }
  }
}


auto SceneRenderer::RecreateSsaoSamples(int const sample_count) noexcept -> void {
  ssao_samples_.resize(sample_count);
  ssao_samples_changed_ = true;

  std::uniform_real_distribution dist{0.0f, 1.0f};
  std::default_random_engine gen; // NOLINT(cert-msc51-cpp)

  for (auto i{0}; i < sample_count; i++) {
    Vector3 sample{dist(gen) * 2 - 1, dist(gen) * 2 - 1, dist(gen)};
    Normalize(sample);
    sample *= dist(gen);

    auto scale{static_cast<float>(i) / static_cast<float>(sample_count)};
    scale = std::lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;

    ssao_samples_[i] = Vector4{sample, 0};
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
    D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, FALSE, depth_bias_multiplier * 1, 0.F, depth_bias_multiplier * 2.5F,
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
  CD3DX12_RT_FORMAT_ARRAY const gbuffer_velocity_format{
    D3D12_RT_FORMAT_ARRAY{
      .RTFormats = {gbuffer0_format_, gbuffer1_format_, gbuffer2_format_, velocity_format_}, .NumRenderTargets = 4
    }
  };

  wand::PipelineDesc const shadow_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{&g_depth_only_ps_bytes, ARRAYSIZE(g_depth_only_ps_bytes)},
    .as = CD3DX12_SHADER_BYTECODE{&g_depth_only_as_bytes, ARRAYSIZE(g_depth_only_as_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{&g_depth_only_ms_bytes, ARRAYSIZE(g_depth_only_ms_bytes)},
    .depth_stencil_state = depth_stencil_write, .ds_format = depth_format_,
    .rasterizer_state = shadow_rasterizer_desc
  };

  shadow_pso_ = device_->CreatePipelineState(shadow_pso_desc, sizeof(DepthOnlyDrawParams) / 4);

  wand::PipelineDesc const depth_resolve_pso_desc{
    .cs = CD3DX12_SHADER_BYTECODE{&g_depth_resolve_cs_bytes, ARRAYSIZE(g_depth_resolve_cs_bytes)}
  };

  depth_resolve_pso_ = device_->CreatePipelineState(depth_resolve_pso_desc, sizeof(DepthResolveDrawParams) / 4);

  wand::PipelineDesc const line_gizmo_pso_desc{
    .primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
    .vs = CD3DX12_SHADER_BYTECODE{&g_gizmos_line_vs_bytes, ARRAYSIZE(g_gizmos_line_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_gizmos_ps_bytes, ARRAYSIZE(g_gizmos_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = render_target_format
  };

  line_gizmo_pso_ = device_->CreatePipelineState(line_gizmo_pso_desc, sizeof(GizmoDrawParams) / 4);


  wand::PipelineDesc const gbuffer_velocity_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{&g_gbuffer_velocity_ps_bytes, ARRAYSIZE(g_gbuffer_velocity_ps_bytes)},
    .as = CD3DX12_SHADER_BYTECODE{&g_gbuffer_velocity_as_bytes, ARRAYSIZE(g_gbuffer_velocity_as_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{&g_gbuffer_velocity_ms_bytes, ARRAYSIZE(g_gbuffer_velocity_ms_bytes)},
    .depth_stencil_state = depth_stencil_write, .ds_format = depth_format_,
    .rt_formats = gbuffer_velocity_format
  };
  gbuffer_velocity_pso_ = device_->CreatePipelineState(gbuffer_velocity_pso_desc, sizeof(GBufferDrawParams) / 4);

  wand::PipelineDesc const deferred_lighting_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_deferred_lighting_vs_bytes, ARRAYSIZE(g_deferred_lighting_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_deferred_lighting_ps_bytes, ARRAYSIZE(g_deferred_lighting_ps_bytes)},
    .depth_stencil_state = depth_stencil_read_not_equal, .ds_format = depth_format_, .rt_formats = color_format
  };

  deferred_lighting_pso_ = device_->CreatePipelineState(deferred_lighting_pso_desc,
    sizeof(DeferredLightingDrawParams) / 4);

  wand::PipelineDesc const post_process_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_post_process_vs_bytes, ARRAYSIZE(g_post_process_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_post_process_ps_bytes, ARRAYSIZE(g_post_process_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = render_target_format,
  };

  post_process_pso_ = device_->CreatePipelineState(post_process_pso_desc, sizeof(PostProcessDrawParams) / 4);

  wand::PipelineDesc const skybox_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{&g_skybox_ps_bytes, ARRAYSIZE(g_skybox_ps_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{&g_skybox_ms_bytes, ARRAYSIZE(g_skybox_ms_bytes)},
    .depth_stencil_state = depth_stencil_write, .ds_format = depth_format_, .rasterizer_state = skybox_rasterizer_desc,
    .rt_formats = color_format
  };
  skybox_pso_ = device_->CreatePipelineState(skybox_pso_desc, sizeof(SkyboxDrawParams) / 4);

  wand::PipelineDesc const ssao_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssao_main_ps_bytes, ARRAYSIZE(g_ssao_main_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = ssao_format,
  };

  ssao_pso_ = device_->CreatePipelineState(ssao_pso_desc, sizeof(SsaoDrawParams) / 4);

  wand::PipelineDesc const ssao_blur_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssao_vs_bytes, ARRAYSIZE(g_ssao_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssao_blur_ps_bytes, ARRAYSIZE(g_ssao_blur_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = ssao_format
  };

  ssao_blur_pso_ = device_->CreatePipelineState(ssao_blur_pso_desc, sizeof(SsaoBlurDrawParams) / 4);

  wand::PipelineDesc const ssr_compose_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssr_vs_bytes, ARRAYSIZE(g_ssr_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssr_compose_ps_bytes, ARRAYSIZE(g_ssr_compose_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = color_format
  };

  ssr_compose_pso_ = device_->CreatePipelineState(ssr_compose_pso_desc, sizeof(SsrComposeDrawParams) / 4);

  wand::PipelineDesc const ssr_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_ssr_vs_bytes, ARRAYSIZE(g_ssr_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_ssr_ps_bytes, ARRAYSIZE(g_ssr_ps_bytes)},
    .depth_stencil_state = depth_stencil_read_not_equal, .ds_format = depth_format_,
    .rt_formats = color_format
  };

  ssr_pso_ = device_->CreatePipelineState(ssr_pso_desc, sizeof(SsrDrawParams) / 4);

  wand::PipelineDesc const taa_resolve_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_taa_resolve_vs_bytes, ARRAYSIZE(g_taa_resolve_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_taa_resolve_ps_bytes, ARRAYSIZE(g_taa_resolve_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = color_format
  };

  taa_pso_ = device_->CreatePipelineState(taa_resolve_pso_desc, sizeof(TaaResolveDrawParams) / 4);

  wand::PipelineDesc const vtx_skinning_pso_desc{
    .cs = CD3DX12_SHADER_BYTECODE{&g_vtx_skinning_cs_bytes, ARRAYSIZE(g_vtx_skinning_cs_bytes)}
  };

  vtx_skinning_pso_ = device_->CreatePipelineState(vtx_skinning_pso_desc, sizeof(VertexSkinningDrawParams) / 4);

  wand::PipelineDesc const irradiance_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{&g_irradiance_ps_bytes, ARRAYSIZE(g_irradiance_ps_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{&g_irradiance_ms_bytes, ARRAYSIZE(g_irradiance_ms_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rasterizer_state = skybox_rasterizer_desc,
    .rt_formats = CD3DX12_RT_FORMAT_ARRAY{
      D3D12_RT_FORMAT_ARRAY{.RTFormats = {irradiance_map_format_}, .NumRenderTargets = 1}
    },
  };

  irradiance_pso_ = device_->CreatePipelineState(irradiance_pso_desc, sizeof(IrradianceDrawParams) / 4);

  wand::PipelineDesc const envmap_prefilter_pso_desc{
    .ps = CD3DX12_SHADER_BYTECODE{&g_envmap_prefilter_ps_bytes, ARRAYSIZE(g_envmap_prefilter_ps_bytes)},
    .ms = CD3DX12_SHADER_BYTECODE{&g_envmap_prefilter_ms_bytes, ARRAYSIZE(g_envmap_prefilter_ms_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rasterizer_state = skybox_rasterizer_desc,
    .rt_formats = CD3DX12_RT_FORMAT_ARRAY{
      D3D12_RT_FORMAT_ARRAY{.RTFormats = {prefiltered_env_map_format_}, .NumRenderTargets = 1}
    },
  };

  envmap_prefilter_pso_ = device_->
    CreatePipelineState(envmap_prefilter_pso_desc, sizeof(EnvmapPrefilterDrawParams) / 4);

  wand::PipelineDesc const brdf_integration_pso_desc{
    .vs = CD3DX12_SHADER_BYTECODE{&g_brdf_integration_vs_bytes, ARRAYSIZE(g_brdf_integration_vs_bytes)},
    .ps = CD3DX12_SHADER_BYTECODE{&g_brdf_integration_ps_bytes, ARRAYSIZE(g_brdf_integration_ps_bytes)},
    .depth_stencil_state = depth_stencil_disabled, .rt_formats = CD3DX12_RT_FORMAT_ARRAY{
      D3D12_RT_FORMAT_ARRAY{.RTFormats = {brdf_integration_map_format_}, .NumRenderTargets = 1}
    },
  };

  brdf_integration_pso_ = device_->CreatePipelineState(brdf_integration_pso_desc, 0);
}


auto SceneRenderer::CreatePerViewConstantBuffers(UINT const count) -> void {
  per_view_cbs_.reserve(per_view_cbs_.size() + count);

  for (UINT i{0}; i < count; i++) {
    per_view_cbs_.emplace_back(std::array{
        MappedConstantBuffer<ShaderPerViewConstants>{*device_},
        MappedConstantBuffer<ShaderPerViewConstants>{*device_}
      }
    );
  }
}


auto SceneRenderer::CreatePerInstanceConstantBuffers(UINT const count) -> void {
  per_inst_cbs_.reserve(per_inst_cbs_.size() + count);

  for (UINT i{0}; i < count; i++) {
    per_inst_cbs_.emplace_back(
      std::array{
        MappedConstantBuffer<ShaderPerInstanceConstants>{*device_},
        MappedConstantBuffer<ShaderPerInstanceConstants>{*device_}
      }
    );
  }
}


auto SceneRenderer::AcquirePerViewConstantBuffer(
  std::uint32_t const frame_idx) -> MappedConstantBuffer<ShaderPerViewConstants>& {
  if (next_per_view_cb_idx_ >= per_view_cbs_.size()) {
    CreatePerViewConstantBuffers(1);
  }

  return per_view_cbs_[next_per_view_cb_idx_++][frame_idx];
}


auto SceneRenderer::AcquirePerInstanceConstantBuffer(
  std::uint32_t const frame_idx) -> MappedConstantBuffer<ShaderPerInstanceConstants>& {
  if (next_per_instance_cb_idx_ >= per_inst_cbs_.size()) {
    CreatePerInstanceConstantBuffers(1);
  }

  return per_inst_cbs_[next_per_instance_cb_idx_++][frame_idx];
}


auto SceneRenderer::OnWindowSize(Extent2D<std::uint32_t> const size) -> void {
  if (size.width != 0 && size.height != 0) {
    RenderTarget::Desc desc{main_rt_->GetDesc()};
    desc.width = size.width;
    desc.height = size.height;
    main_rt_ = RenderTarget::New(*device_, desc);
  }
}


auto SceneRenderer::RecordGpuInitWork(RenderFrame& frame) const -> void {
  D3D12_VIEWPORT const brdf_integration_viewport{
    0.F, 0.F, static_cast<FLOAT>(brdf_integration_map_size_), static_cast<FLOAT>(brdf_integration_map_size_), 0.F, 1.F
  };

  D3D12_RECT const brdf_integration_scissor_rect{
    0, 0, static_cast<LONG>(brdf_integration_map_size_), static_cast<LONG>(brdf_integration_map_size_)
  };

  auto& cmd{frame.AcquireCommandList()};
  cmd.Begin(brdf_integration_pso_.get());
  cmd.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  cmd.SetRenderTargets(std::array{static_cast<wand::Texture const*>(brdf_integration_map_.get())}, nullptr);
  cmd.SetViewports(std::span{&brdf_integration_viewport, 1});
  cmd.SetScissorRects(std::span{&brdf_integration_scissor_rect, 1});
  cmd.ClearRenderTarget(*brdf_integration_map_, std::array{0.F, 0.F, 0.F, 1.F}, {});
  cmd.DrawInstanced(3, 1, 0, 0);
  cmd.End();

  frame.EnqueueCommandList(cmd);
}


auto SceneRenderer::RecordDepthOnlyPass(ExtractedFrameData const& frame_packet, RenderFrame const& frame,
                                        PreparedView const& view, std::uint32_t const rt_idx,
                                        wand::CommandList& cmd) -> void {
  cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, rt_idx), rt_idx);

  cmd.SetViewports(std::span{&view.viewport, 1});
  cmd.SetScissorRects(std::span{&view.scissor, 1});

  auto& per_view_cb{AcquirePerViewConstantBuffer(frame.GetIndex())};
  SetPerViewConstants(per_view_cb, view.view_mtx, view.proj_mtx, view.view_proj_mtx, {}, ShadowCascadeBoundaries{},
    view.frustum_ws, Vector3{}, view.near_plane, view.far_plane, static_cast<int>(view.viewport.Width),
    static_cast<int>(view.viewport.Height));
  cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_view_cb_idx), *per_view_cb.GetBuffer());

  for (auto const& geom_batch : frame_packet.geom_batches) {
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, pos_buf_idx),
      *frame_packet.buffers[geom_batch.pos_buf_local_idx]);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, uv_buf_idx),
      *frame_packet.buffers[geom_batch.uv_buf_local_idx]);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, vertex_idx_buf_idx),
      *frame_packet.buffers[geom_batch.vtx_idx_buf_local_idx]);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, prim_idx_buf_idx),
      *frame_packet.buffers[geom_batch.prim_idx_buf_local_idx]);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_buf_idx),
      *frame_packet.buffers[geom_batch.meshlet_buf_local_idx]);
    cmd.SetShaderResource(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, cull_data_buf_idx),
      *frame_packet.buffers[geom_batch.cull_data_buf_local_idx]);
    cmd.SetPipelineParameter(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, idx32), geom_batch.idx32);

    auto const instances = std::span{frame_packet.instance_data}.subspan(geom_batch.first_instance,
      geom_batch.instance_count);

    auto const mtl_groups = std::span{frame_packet.mtl_slot_groups}.subspan(geom_batch.first_mtl_group,
      geom_batch.mtl_group_count);

    for (auto const& instance : instances) {
      auto& per_inst_cb{AcquirePerInstanceConstantBuffer(frame.GetIndex())};
      SetPerInstanceConstants(per_inst_cb, instance.local_to_world_mtx, view.view_mtx, view.proj_mtx, {},
        instance.max_abs_scaling);
      cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, per_inst_cb_idx),
        *per_inst_cb.GetBuffer());

      for (auto const& mtl_group : mtl_groups) {
        auto const inst_mtl_local_idx = instance.first_mtl + mtl_group.mtl_slot;
        auto const& mtl_buf_local_idx{frame_packet.instance_materials[inst_mtl_local_idx]};

        if (mtl_buf_local_idx == frame_packet_invalid_idx) {
          continue;
        }

        cmd.SetConstantBuffer(PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, mtl_idx),
          *frame_packet.buffers[mtl_buf_local_idx]);

        auto const submeshes = std::span{frame_packet.submesh_data}.subspan(mtl_group.first_submesh,
          mtl_group.submesh_count);

        for (auto const& submesh : submeshes) {
          DrawSubmesh(submesh, PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_count),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, meshlet_offset),
            PIPELINE_PARAM_INDEX(DepthOnlyDrawParams, base_vertex),
            cmd);
        }
      }
    }
  }
}


auto SceneRenderer::DrawSubmesh(SubmeshData const& submesh, std::optional<UINT> const meshlet_count_param_idx,
                                std::optional<UINT> const meshlet_offset_param_idx,
                                std::optional<UINT> const base_vertex_param_idx,
                                wand::CommandList const& cmd) -> void {
  DrawSubmesh(submesh.meshlet_count, submesh.first_meshlet, submesh.base_vertex, meshlet_count_param_idx,
    meshlet_offset_param_idx, base_vertex_param_idx, cmd);
}


auto SceneRenderer::DrawSubmesh(UINT const submesh_meshlet_count, UINT const submesh_meshlet_offset,
                                UINT const submesh_base_vertex,
                                std::optional<UINT> const meshlet_count_param_idx,
                                std::optional<UINT> const meshlet_offset_param_idx,
                                std::optional<UINT> const base_vertex_param_idx,
                                wand::CommandList const& cmd) -> void {
  UINT constexpr max_dispatch_thread_group_count{65535};
  UINT constexpr max_meshlet_count_per_dispatch{AS_THREAD_GROUP_SIZE * max_dispatch_thread_group_count};

  auto const submesh_meshlet_end{submesh_meshlet_offset + submesh_meshlet_count};

  for (auto meshlet_offset{submesh_meshlet_offset};
       meshlet_offset < submesh_meshlet_end;
       meshlet_offset += max_meshlet_count_per_dispatch) {
    auto const this_dispatch_meshlet_count{
      std::min(submesh_meshlet_end - meshlet_offset, max_meshlet_count_per_dispatch)
    };

    auto const this_dispatch_thread_group_count{DivRoundUp<UINT>(this_dispatch_meshlet_count, AS_THREAD_GROUP_SIZE)};

    if (meshlet_count_param_idx) {
      cmd.SetPipelineParameter(*meshlet_count_param_idx, this_dispatch_meshlet_count);
    }

    if (meshlet_offset_param_idx) {
      cmd.SetPipelineParameter(*meshlet_offset_param_idx, meshlet_offset);
    }

    if (base_vertex_param_idx) {
      cmd.SetPipelineParameter(*base_vertex_param_idx, submesh_base_vertex);
    }

    cmd.DispatchMesh(this_dispatch_thread_group_count, 1, 1);
  }
}
}
