#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "Camera.hpp"
#include "constant_buffer.hpp"
#include "directional_shadow_map_array.hpp"
#include "graphics.hpp"
#include "punctual_shadow_atlas.hpp"
#include "render_manager.hpp"
#include "render_target.hpp"
#include "structured_buffer.hpp"
#include "../Color.hpp"
#include "../fast_vector.hpp"
#include "../Math.hpp"
#include "../Util.hpp"
#include "../Window.hpp"
#include "../scene_objects/LightComponents.hpp"
#include "../scene_objects/SkinnedMeshComponent.hpp"
#include "../scene_objects/StaticMeshComponent.hpp"
#include "shaders/shader_interop.h"
#include "shaders/shadow_filtering_modes.h"


namespace sorcery::rendering {
// Passing these enum values to shaders is valid
enum class ShadowFilteringMode : int {
  kNone        = SHADOW_FILTERING_NONE,
  kHardwarePcf = SHADOW_FILTERING_HARDWARE_PCF,
  kPcf3X3      = SHADOW_FILTERING_PCF_3x3,
  kPcfTent3X3  = SHADOW_FILTERING_PCF_TENT_3x3,
  kPcfTent5X5  = SHADOW_FILTERING_PCF_TENT_5x5
};


struct SsaoParams {
  float radius;
  float bias;
  float power;
  int sample_count;
};


struct ShadowParams {
  std::array<float, MAX_CASCADE_COUNT - 1> normalized_cascade_splits;
  unsigned cascade_count;
  bool visualize_cascades;
  float distance;
  ShadowFilteringMode filtering_mode;
};


struct SsrParams {
  float max_roughness;
  float thickness_vs;
  float stride;
  float max_trace_dist_vs;
  float ray_start_bias_vs;
};


struct SsrRenderData;


class SceneRenderer {
public:
  LEOPPHAPI SceneRenderer(Window& window, graphics::GraphicsDevice& device, RenderManager& render_manager);
  SceneRenderer(SceneRenderer const&) = delete;
  SceneRenderer(SceneRenderer&&) = delete;

  LEOPPHAPI ~SceneRenderer();

  auto operator=(SceneRenderer const&) -> void = delete;
  auto operator=(SceneRenderer&&) -> void = delete;

  LEOPPHAPI auto ExtractCurrentState() -> void;
  LEOPPHAPI auto Render() -> void;

  LEOPPHAPI auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;

  // Global cameras are the ones without a set render target.
  [[nodiscard]] LEOPPHAPI auto IsRenderingGlobalCameras() const noexcept -> bool;
  // Global cameras are the ones without a set render target.
  LEOPPHAPI auto SetRenderGlobalCameras(bool render) noexcept -> void;

  // If a render target override is set, all cameras not targeting a specific render target
  // will render into the override RT.
  [[nodiscard]] LEOPPHAPI auto GetRenderTargetOverride() -> std::shared_ptr<RenderTarget> const&;
  LEOPPHAPI auto SetRenderTargetOverride(std::shared_ptr<RenderTarget> rt_override) -> void;

  [[nodiscard]] LEOPPHAPI auto GetCurrentRenderTarget() const -> RenderTarget const&;

  [[nodiscard]] LEOPPHAPI auto IsUsingPreciseColorFormat() const noexcept -> bool;
  LEOPPHAPI auto SetUsePreciseColorFormat(bool precise) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetShadowDistance() const noexcept -> float;
  LEOPPHAPI auto SetShadowDistance(float distance) noexcept -> void;

  [[nodiscard]] constexpr static auto GetMaxShadowCascadeCount() noexcept -> unsigned;
  [[nodiscard]] LEOPPHAPI auto GetShadowCascadeCount() const noexcept -> unsigned;
  LEOPPHAPI auto SetShadowCascadeCount(unsigned cascade_count) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const>;
  LEOPPHAPI auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsVisualizingShadowCascades() const noexcept -> bool;
  LEOPPHAPI auto VisualizeShadowCascades(bool visualize) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetShadowFilteringMode() const noexcept -> ShadowFilteringMode;
  LEOPPHAPI auto SetShadowFilteringMode(ShadowFilteringMode filtering_mode) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsSsaoEnabled() const noexcept -> bool;
  LEOPPHAPI auto SetSsaoEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSsaoParams() const noexcept -> SsaoParams const&;
  LEOPPHAPI auto SetSsaoParams(SsaoParams const& ssao_params) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsSsrEnabled() const noexcept -> bool;
  LEOPPHAPI auto SetSsrEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSsrParams() const noexcept -> SsrParams const&;
  LEOPPHAPI auto SetSsrParams(SsrParams const& ssr_params) -> void;

  [[nodiscard]] LEOPPHAPI auto GetGamma() const noexcept -> float;
  LEOPPHAPI auto SetGamma(float gamma) noexcept -> void;

  LEOPPHAPI auto Register(StaticMeshComponent& static_mesh_component) noexcept -> void;
  LEOPPHAPI auto Unregister(StaticMeshComponent const& static_mesh_component) noexcept -> void;

  LEOPPHAPI auto Register(SkinnedMeshComponent& skinned_mesh_component) noexcept -> void;
  LEOPPHAPI auto Unregister(SkinnedMeshComponent const& skinned_mesh_component) noexcept -> void;

  LEOPPHAPI auto Register(LightComponent const& light_component) noexcept -> void;
  LEOPPHAPI auto Unregister(LightComponent const& light_component) noexcept -> void;

  LEOPPHAPI auto Register(Camera& cam) noexcept -> void;
  LEOPPHAPI auto Unregister(Camera const& cam) noexcept -> void;

private:
  struct LightData {
    Vector3 color;
    float intensity;

    Vector3 direction;
    Vector3 position;

    LightComponent::Type type;
    float range;
    float inner_angle;
    float outer_angle;

    bool casts_shadow;
    float shadow_near_plane;
    float shadow_normal_bias;
    float shadow_depth_bias;
    float shadow_extension;

    Matrix4 local_to_world_mtx_no_scale;
  };


  struct MeshData {
    unsigned pos_buf_local_idx;
    unsigned norm_buf_local_idx;
    unsigned tan_buf_local_idx;
    unsigned uv_buf_local_idx;
    unsigned meshlet_buf_local_idx;
    unsigned vtx_idx_buf_local_idx;
    unsigned prim_idx_buf_local_idx;
    unsigned cull_data_buf_local_idx;
    AABB bounds;
    unsigned vtx_count;
    bool idx32;
  };


  struct SubmeshData {
    unsigned mesh_local_idx;
    UINT first_meshlet;
    UINT meshlet_count;
    UINT base_vertex;
    UINT mtl_buf_local_idx;
    AABB bounds;
  };


  struct InstanceData {
    unsigned submesh_local_idx;
    Matrix4 local_to_world_mtx;
    Matrix4 prev_local_to_world_mtx;
    float max_abs_scaling;
  };


  struct CameraData {
    Vector3 position;
    Vector3 right;
    Vector3 up;
    Vector3 forward;

    float near_plane;
    float far_plane;

    Camera::Type type;
    float fov_vert_deg;
    float size_vert;

    NormalizedViewport viewport;

    unsigned rt_local_idx;
    unsigned accum_tex_local_idx;

    void const* id; // Used to identify cameras between frames, e.g. for TAA accumulation

    bool accum_tex_empty;

    float jitter_x;
    float jitter_y;
  };


  struct NodeAnimationData {
    unsigned pos_key_begin_local_idx;
    unsigned pos_key_count;

    unsigned rot_key_begin_local_idx;
    unsigned rot_key_count;

    unsigned scaling_key_begin_local_idx;
    unsigned scaling_key_count;

    unsigned node_idx; // Original index, needs to be offset with the absolute position in the array
  };


  struct SkeletonNodeData {
    Matrix4 transform;
    std::optional<unsigned> parent_idx; // Original index, needs to be offset with the absolute position in the array
  };


  struct BoneData {
    Matrix4 offset_mtx;
    unsigned skeleton_node_idx; // Original index, needs to be offset with the absolute position in the array
  };


  struct SkinnedMeshData {
    unsigned mesh_data_local_idx;
    // The referenced mesh data contains an index to skinned vertex buffer
    unsigned original_vertex_buf_local_idx;
    // The referenced mesh data contains an index to skinned normal buffer
    unsigned original_normal_buf_local_idx;
    // The referenced mesh data contains an index to skinned tangent buffer
    unsigned original_tangent_buf_local_idx;
    unsigned bone_weight_buf_local_idx;
    unsigned bone_index_buf_local_idx;
    unsigned bone_matrix_buf_local_idx;
    unsigned prev_frame_vertex_buf_local_idx;

    float cur_animation_time;

    unsigned node_anim_begin_local_idx;
    unsigned node_anim_count;
    unsigned skeleton_begin_local_idx;
    unsigned skeleton_size;
    unsigned bone_begin_local_idx;
    unsigned bone_count;
  };


  struct FramePacket {
    FastVector<graphics::SharedDeviceChildHandle<graphics::Buffer>> buffers;
    FastVector<graphics::SharedDeviceChildHandle<graphics::Texture>> textures;
    FastVector<LightData> light_data;
    FastVector<MeshData> mesh_data;
    FastVector<SubmeshData> submesh_data;
    FastVector<InstanceData> instance_data;
    FastVector<CameraData> cam_data;
    FastVector<std::shared_ptr<RenderTarget>> render_targets;

    FastVector<AnimPositionKey> anim_pos_keys;
    FastVector<AnimRotationKey> anim_rot_keys;
    FastVector<AnimScalingKey> anim_scaling_keys;
    FastVector<NodeAnimationData> node_anim_data;
    FastVector<SkeletonNodeData> skeleton_node_data;
    FastVector<BoneData> bone_data;
    FastVector<SkinnedMeshData> skinned_mesh_data;

    FastVector<Vector4> gizmo_colors;
    FastVector<ShaderLineGizmoVertexData> line_gizmo_vertex_data;

    SsaoParams ssao_params;
    SsrParams ssr_params;
    ShadowParams shadow_params;
    float inv_gamma;
    bool ssao_enabled;
    bool ssr_enabled;
    DXGI_FORMAT color_buffer_format;
    std::array<float, 4> background_color;

    graphics::SharedDeviceChildHandle<graphics::Texture> skybox_cubemap;
    graphics::SharedDeviceChildHandle<graphics::Texture> irradiance_map;
    graphics::SharedDeviceChildHandle<graphics::Texture> prefiltered_env_map;
    bool draw_irradiance_map;
    bool draw_prefiltered_env_map;

    Vector3 ambient_light;

    graphics::SharedDeviceChildHandle<graphics::PipelineState> shadow_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> gbuffer_velocity_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> depth_resolve_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> line_gizmo_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> deferred_lighting_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> post_process_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> skybox_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> ssao_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> ssao_blur_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> ssr_compose_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> ssr_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> taa_resolve_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> vtx_skinning_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> irradiance_pso;
    graphics::SharedDeviceChildHandle<graphics::PipelineState> envmap_prefilter_pso;
  };


  [[nodiscard]] static auto CalculateCameraShadowCascadeBoundaries(CameraData const& cam_data,
                                                                   ShadowParams const& shadow_params) ->
    ShadowCascadeBoundaries;


  static auto CullLights(Frustum const& frustum_ws, std::span<LightData const> lights,
                         FastVector<unsigned>& visible_light_indices) -> void;


  static auto SetPerFrameConstants(ConstantBuffer<ShaderPerFrameConstants>& cb, int rt_width, int rt_height,
                                   Vector3 const& ambient_light, ShadowParams const& shadow_params) -> void;
  static auto SetPerViewConstants(ConstantBuffer<ShaderPerViewConstants>& cb, Matrix4 const& view_mtx,
                                  Matrix4 const& proj_mtx, Matrix4 const& prev_view_proj_mtx,
                                  ShadowCascadeBoundaries const& cascade_bounds, Frustum const& frustum_ws,
                                  Vector3 const& view_pos, float near_clip_plane, float far_clip_plane) -> void;
  static auto SetPerDrawConstants(ConstantBuffer<ShaderPerDrawConstants>& cb, Matrix4 const& model_mtx,
                                  Matrix4 const& view_mtx, Matrix4 const& proj_mtx,
                                  Matrix4 const& prev_model_mtx, float max_abs_scaling) -> void;


  auto UpdatePunctualShadowAtlas(PunctualShadowAtlas& atlas, std::span<LightData const> lights,
                                 std::span<unsigned const> visible_light_indices, CameraData const& cam_data,
                                 Matrix4 const& cam_view_proj_mtx, float shadow_distance) -> void;


  auto DrawDirectionalShadowMaps(FramePacket const& frame_packet, std::span<unsigned const> visible_light_indices,
                                 CameraData const& cam_data, float rt_aspect, int cascade_count,
                                 ShadowCascadeBoundaries const& shadow_cascade_boundaries,
                                 std::array<Matrix4, MAX_CASCADE_COUNT>& shadow_view_proj_matrices,
                                 graphics::CommandList& cmd) -> void;
  auto DrawPunctualShadowMaps(PunctualShadowAtlas const& atlas, FramePacket const& frame_packet,
                              graphics::CommandList& cmd) -> void;

  auto ClearGizmoDrawQueue() noexcept -> void;

  auto RecreateSsaoSamples(int sample_count) noexcept -> void;

  auto RecreatePipelines() -> void;

  auto CreatePerViewConstantBuffers(UINT count) -> void;
  auto CreatePerDrawConstantBuffers(UINT count) -> void;

  auto AcquirePerViewConstantBuffer() -> ConstantBuffer<ShaderPerViewConstants>&;
  auto AcquirePerDrawConstantBuffer() -> ConstantBuffer<ShaderPerDrawConstants>&;

  auto RecreateSssrContext(Extent2D<std::uint32_t> size) const -> void;

  auto OnWindowSize(Extent2D<std::uint32_t> size) -> void;

  static auto DrawSubmesh(SubmeshData const& submesh, std::optional<UINT> meshlet_count_param_idx,
                          std::optional<UINT> meshlet_offset_param_idx,
                          std::optional<UINT> base_vertex_param_idx,
                          graphics::CommandList const& cmd) -> void;
  static auto DrawSubmesh(UINT submesh_meshlet_count, UINT submesh_meshlet_offset,
                          UINT submesh_base_vertex, std::optional<UINT> meshlet_count_param_idx,
                          std::optional<UINT> meshlet_offset_param_idx,
                          std::optional<UINT> base_vertex_param_idx,
                          graphics::CommandList const& cmd) -> void;

  static DXGI_FORMAT constexpr imprecise_color_buffer_format_{DXGI_FORMAT_R11G11B10_FLOAT};
  static DXGI_FORMAT constexpr precise_color_buffer_format_{DXGI_FORMAT_R16G16B16A16_FLOAT};
  static DXGI_FORMAT constexpr depth_format_{DXGI_FORMAT_D32_FLOAT};
  static DXGI_FORMAT constexpr render_target_format_{DXGI_FORMAT_R8G8B8A8_UNORM};
  static DXGI_FORMAT constexpr ssao_buffer_format_{DXGI_FORMAT_R8_UNORM};
  static DXGI_FORMAT constexpr normal_buffer_format_{DXGI_FORMAT_R8G8B8A8_SNORM};
  static DXGI_FORMAT constexpr gbuffer0_format_{DXGI_FORMAT_R8G8B8A8_UNORM};
  static DXGI_FORMAT constexpr gbuffer1_format_{DXGI_FORMAT_R16G16_FLOAT};
  static DXGI_FORMAT constexpr gbuffer2_format_{DXGI_FORMAT_R8G8_UNORM};
  static DXGI_FORMAT constexpr velocity_format_{DXGI_FORMAT_R16G16_FLOAT};
  static DXGI_FORMAT constexpr irradiance_map_format_{DXGI_FORMAT_R16G16B16A16_FLOAT};
  static DXGI_FORMAT constexpr prefiltered_env_map_format_{DXGI_FORMAT_R16G16B16A16_FLOAT};
  static DXGI_FORMAT constexpr brdf_integration_map_format_{DXGI_FORMAT_R16G16_FLOAT};
  static constexpr unsigned taa_subpixel_sample_count_{8};
  static constexpr UINT irradiance_map_size_{64};
  static constexpr UINT prefiltered_env_map_size_{1024};
  static constexpr UINT brdf_integration_map_size_{128};

  ObserverPtr<RenderManager> render_manager_;
  ObserverPtr<Window> window_;

  ObserverPtr<graphics::GraphicsDevice> device_;

  std::array<ConstantBuffer<ShaderPerFrameConstants>, RenderManager::GetMaxFramesInFlight()> per_frame_cbs_;
  FastVector<std::array<ConstantBuffer<ShaderPerViewConstants>, RenderManager::GetMaxFramesInFlight()>> per_view_cbs_;
  FastVector<std::array<ConstantBuffer<ShaderPerDrawConstants>, RenderManager::GetMaxFramesInFlight()>> per_draw_cbs_;
  StructuredBuffer<ShaderLight> light_buffer_;

  graphics::SharedDeviceChildHandle<graphics::Texture> white_tex_;
  graphics::SharedDeviceChildHandle<graphics::Texture> ssao_noise_tex_;

  graphics::SharedDeviceChildHandle<graphics::PipelineState> shadow_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> depth_resolve_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> line_gizmo_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> gbuffer_velocity_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> deferred_lighting_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> post_process_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> skybox_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> ssao_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> ssao_blur_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> ssr_compose_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> ssr_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> taa_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> vtx_skinning_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> irradiance_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> envmap_prefilter_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> brdf_integration_pso_;

  graphics::UniqueSamplerHandle samp_cmp_pcf_ge_;
  graphics::UniqueSamplerHandle samp_cmp_pcf_le_;
  graphics::UniqueSamplerHandle samp_cmp_point_ge_;
  graphics::UniqueSamplerHandle samp_cmp_point_le_;
  graphics::UniqueSamplerHandle samp_af16_clamp_;
  graphics::UniqueSamplerHandle samp_af8_clamp_;
  graphics::UniqueSamplerHandle samp_af4_clamp_;
  graphics::UniqueSamplerHandle samp_af2_clamp_;
  graphics::UniqueSamplerHandle samp_tri_clamp_;
  graphics::UniqueSamplerHandle samp_bi_clamp_;
  graphics::UniqueSamplerHandle samp_point_clamp_;
  graphics::UniqueSamplerHandle samp_af16_wrap_;
  graphics::UniqueSamplerHandle samp_af8_wrap_;
  graphics::UniqueSamplerHandle samp_af4_wrap_;
  graphics::UniqueSamplerHandle samp_af2_wrap_;
  graphics::UniqueSamplerHandle samp_tri_wrap_;
  graphics::UniqueSamplerHandle samp_bi_wrap_;
  graphics::UniqueSamplerHandle samp_point_wrap_;

  std::array<FramePacket, RenderManager::GetMaxFramesInFlight()> frame_packets_;

  UINT next_per_draw_cb_idx_{0};
  UINT next_per_view_cb_idx_{0};

  std::unique_ptr<DirectionalShadowMapArray> dir_shadow_map_arr_;
  std::unique_ptr<PunctualShadowAtlas> punctual_shadow_atlas_;

  FastVector<Vector4> gizmo_colors_;
  StructuredBuffer<Vector4> gizmo_color_buffer_;

  FastVector<ShaderLineGizmoVertexData> line_gizmo_vertex_data_;
  StructuredBuffer<ShaderLineGizmoVertexData> line_gizmo_vertex_data_buffer_;

  StructuredBuffer<Vector4> ssao_samples_buffer_;

  graphics::SharedDeviceChildHandle<graphics::Texture> brdf_integration_map_;

  SsaoParams ssao_params_{.radius = 0.1f, .bias = 0.025f, .power = 6.0f, .sample_count = 12};
  SsrParams ssr_params_{
    .max_roughness = 0.3f, .thickness_vs = 0.35f, .stride = 1, .max_trace_dist_vs = 1000, .ray_start_bias_vs = 0.1f
  };
  ShadowParams shadow_params_{{0.1f, 0.3f, 0.6f}, 4, false, 100, ShadowFilteringMode::kPcfTent5X5};

  float inv_gamma_{1.f / 2.2f};

  bool ssao_enabled_{true};
  bool ssr_enabled_{false};
  bool render_global_cameras_{true};

  DXGI_FORMAT color_buffer_format_{imprecise_color_buffer_format_};

  FastVector<StaticMeshComponent*> static_mesh_components_;
  FastVector<SkinnedMeshComponent*> skinned_mesh_components_;
  FastVector<LightComponent const*> lights_;
  FastVector<Camera*> cameras_;

  std::shared_ptr<RenderTarget> main_rt_;
  std::shared_ptr<RenderTarget> rt_override_;

  EventListenerHandle<Extent2D<unsigned>> window_size_event_listener_{};

  std::unique_ptr<SsrRenderData> ssr_data_;
};


constexpr auto SceneRenderer::GetMaxShadowCascadeCount() noexcept -> unsigned {
  return MAX_CASCADE_COUNT;
}
}
