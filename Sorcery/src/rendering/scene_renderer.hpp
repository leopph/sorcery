#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>

#include "Camera.hpp"
#include "config.hpp"
#include "constant_buffer.hpp"
#include "directional_shadow_map_array.hpp"
#include "punctual_shadow_atlas.hpp"
#include "render_frame.hpp"
#include "render_manager.hpp"
#include "render_target.hpp"
#include "structured_buffer.hpp"
#include "../Color.hpp"
#include "../Math.hpp"
#include "../Util.hpp"
#include "../Window.hpp"
#include "../scene_objects/LightComponents.hpp"
#include "../scene_objects/SkinnedMeshComponent.hpp"
#include "../scene_objects/StaticMeshComponent.hpp"
#include "shaders/shader_interop.h"
#include "shaders/shadow_filtering_modes.h"
#include "wand/wand.hpp"


namespace sorcery::rendering {
class RenderInstanceRegistry;
class RenderResourceRegistry;
class RenderMaterial;
class RenderMesh;
class StaticRenderMeshInstance;
class SkinnedRenderMeshInstance;


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


class SceneRenderer {
public:
  SORCERYAPI SceneRenderer(Window& window, wand::GraphicsDevice& device, RenderManager& render_manager,
                           RenderResourceRegistry& render_resource_registry,
                           RenderInstanceRegistry& render_instance_registry);
  SceneRenderer(SceneRenderer const&) = delete;
  SceneRenderer(SceneRenderer&&) = delete;

  SORCERYAPI ~SceneRenderer();

  auto operator=(SceneRenderer const&) -> void = delete;
  auto operator=(SceneRenderer&&) -> void = delete;

  SORCERYAPI auto ExtractFrame(RenderFrame& frame) -> void;
  SORCERYAPI auto PrepareFrame(RenderFrame& frame) -> void;
  SORCERYAPI auto RecordFrame(RenderFrame& frame) -> void;

  SORCERYAPI auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;

  // Global cameras are the ones without a set render target.
  [[nodiscard]] SORCERYAPI auto IsRenderingGlobalCameras() const noexcept -> bool;
  // Global cameras are the ones without a set render target.
  SORCERYAPI auto SetRenderGlobalCameras(bool render) noexcept -> void;

  // If a render target override is set, all cameras not targeting a specific render target
  // will render into the override RT.
  [[nodiscard]] SORCERYAPI auto GetRenderTargetOverride() -> std::shared_ptr<RenderTarget> const&;
  SORCERYAPI auto SetRenderTargetOverride(std::shared_ptr<RenderTarget> rt_override) -> void;

  [[nodiscard]] SORCERYAPI auto GetCurrentRenderTarget() const -> RenderTarget const&;

  [[nodiscard]] SORCERYAPI auto IsUsingPreciseColorFormat() const noexcept -> bool;
  SORCERYAPI auto SetUsePreciseColorFormat(bool precise) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetShadowDistance() const noexcept -> float;
  SORCERYAPI auto SetShadowDistance(float distance) noexcept -> void;

  [[nodiscard]] constexpr static auto GetMaxShadowCascadeCount() noexcept -> unsigned;
  [[nodiscard]] SORCERYAPI auto GetShadowCascadeCount() const noexcept -> unsigned;
  SORCERYAPI auto SetShadowCascadeCount(unsigned cascade_count) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const>;
  SORCERYAPI auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto IsVisualizingShadowCascades() const noexcept -> bool;
  SORCERYAPI auto VisualizeShadowCascades(bool visualize) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetShadowFilteringMode() const noexcept -> ShadowFilteringMode;
  SORCERYAPI auto SetShadowFilteringMode(ShadowFilteringMode filtering_mode) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto IsSsaoEnabled() const noexcept -> bool;
  SORCERYAPI auto SetSsaoEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetSsaoParams() const noexcept -> SsaoParams const&;
  SORCERYAPI auto SetSsaoParams(SsaoParams const& ssao_params) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto IsSsrEnabled() const noexcept -> bool;
  SORCERYAPI auto SetSsrEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetSsrParams() const noexcept -> SsrParams const&;
  SORCERYAPI auto SetSsrParams(SsrParams const& ssr_params) -> void;

  [[nodiscard]] SORCERYAPI auto GetGamma() const noexcept -> float;
  SORCERYAPI auto SetGamma(float gamma) noexcept -> void;

  SORCERYAPI auto Register(StaticMeshComponent& static_mesh_component) noexcept -> void;
  SORCERYAPI auto Unregister(StaticMeshComponent const& static_mesh_component) noexcept -> void;

  SORCERYAPI auto Register(SkinnedMeshComponent& skinned_mesh_component) noexcept -> void;
  SORCERYAPI auto Unregister(SkinnedMeshComponent const& skinned_mesh_component) noexcept -> void;

  SORCERYAPI auto Register(LightComponent const& light_component) noexcept -> void;
  SORCERYAPI auto Unregister(LightComponent const& light_component) noexcept -> void;

  SORCERYAPI auto Register(Camera& cam) noexcept -> void;
  SORCERYAPI auto Unregister(Camera const& cam) noexcept -> void;

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


  struct GeometryBatch {
    unsigned pos_buf_local_idx;
    unsigned norm_buf_local_idx;
    unsigned tan_buf_local_idx;
    unsigned uv_buf_local_idx;

    unsigned bone_weight_buf_local_idx;
    unsigned bone_idx_buf_local_idx;

    unsigned meshlet_buf_local_idx;
    unsigned vtx_idx_buf_local_idx;
    unsigned prim_idx_buf_local_idx;
    unsigned cull_data_buf_local_idx;

    unsigned first_mtl_group;
    unsigned mtl_group_count;

    unsigned first_instance;
    unsigned instance_count;

    unsigned skinning_data_local_idx;

    AABB bounds;
    unsigned vtx_count;
    ObjectId src_mesh_id;
    bool idx32;
  };


  struct MaterialSlotGroup {
    unsigned mtl_slot;
    unsigned first_submesh;
    unsigned submesh_count;
  };


  struct SubmeshData {
    UINT first_meshlet;
    UINT meshlet_count;
    UINT base_vertex;
    AABB bounds;
  };


  struct InstanceData {
    Matrix4 local_to_world_mtx;
    Matrix4 prev_local_to_world_mtx;
    float max_abs_scaling;

    unsigned first_mtl;
    unsigned mtl_count;
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

    Vector2 jitter_ndc;
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
    unsigned geom_batch_local_idx;
    // The referenced mesh data contains an index to skinned vertex buffer
    unsigned original_vertex_buf_local_idx;
    // The referenced mesh data contains an index to skinned normal buffer
    unsigned original_normal_buf_local_idx;
    // The referenced mesh data contains an index to skinned tangent buffer
    unsigned original_tangent_buf_local_idx;
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


  struct GizmoDrawData {
    std::uint64_t line_count;
  };


  struct ExtractedFrameData {
    std::vector<wand::SharedDeviceChildHandle<wand::Buffer>> buffers;
    std::vector<wand::SharedDeviceChildHandle<wand::Texture>> textures;

    std::vector<LightData> light_data;
    std::vector<GeometryBatch> geom_batches;
    std::vector<MaterialSlotGroup> mtl_slot_groups;
    std::vector<SubmeshData> submesh_data;
    std::vector<InstanceData> instance_data;

    std::vector<unsigned> instance_materials;

    std::vector<CameraData> cam_data;
    std::vector<std::shared_ptr<RenderTarget>> render_targets;

    std::vector<AnimPositionKey> anim_pos_keys;
    std::vector<AnimRotationKey> anim_rot_keys;
    std::vector<AnimScalingKey> anim_scaling_keys;
    std::vector<NodeAnimationData> node_anim_data;
    std::vector<SkeletonNodeData> skeleton_node_data;
    std::vector<BoneData> bone_data;
    std::vector<SkinnedMeshData> skinning_data;

    GizmoDrawData gizmo_data;

    unsigned cube_geom_local_idx;

    SsaoParams ssao_params;
    SsrParams ssr_params;
    ShadowParams shadow_params;
    float inv_gamma;
    bool ssao_enabled;
    bool ssr_enabled;
    DXGI_FORMAT color_buffer_format;
    std::array<float, 4> background_color;

    wand::SharedDeviceChildHandle<wand::Texture> skybox_cubemap;
    wand::SharedDeviceChildHandle<wand::Texture> irradiance_map;
    wand::SharedDeviceChildHandle<wand::Texture> prefiltered_env_map;
    bool draw_irradiance_map;
    bool draw_prefiltered_env_map;

    Vector3 ambient_light;

    wand::SharedDeviceChildHandle<wand::Buffer> ssao_samples_buf;
    std::vector<Vector4> ssao_samples;
    bool upload_ssao_samples;

    wand::SharedDeviceChildHandle<wand::PipelineState> shadow_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> gbuffer_velocity_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> depth_resolve_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> line_gizmo_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> deferred_lighting_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> post_process_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> skybox_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> ssao_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> ssao_blur_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> ssr_compose_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> ssr_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> taa_resolve_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> vtx_skinning_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> irradiance_pso;
    wand::SharedDeviceChildHandle<wand::PipelineState> envmap_prefilter_pso;
  };


  struct PreparedView {
    Matrix4 view_mtx;
    Matrix4 proj_mtx;
    Matrix4 view_proj_mtx;

    Frustum frustum_ws;

    D3D12_VIEWPORT viewport;
    D3D12_RECT scissor;

    float near_plane;
    float far_plane;
  };


  struct PreparedDirectionalShadows {
    // Index within this camera's visible-light slice.
    std::uint32_t visible_light_idx;

    std::uint32_t first_view;
    std::uint32_t view_count;
  };


  struct PreparedCameraData {
    ShadowCascadeBoundaries cascade_boundaries;

    std::optional<PreparedDirectionalShadows> dir_shadows;

    std::uint32_t extracted_data_idx;

    std::uint32_t first_visible_light;
    std::uint32_t visible_light_count;

    std::uint32_t primary_view_idx;

    Matrix4 prev_view_proj_mtx;

    Vector2 jitter_ndc;
    Vector2 prev_jitter_ndc;
  };


  struct PreparedFrameData {
    std::vector<PreparedCameraData> cam_data;
    std::vector<std::uint32_t> visible_light_indices;
    std::vector<PreparedView> views;
  };


  [[nodiscard]] static
  auto FindOrAddBufferInPacket(
    wand::SharedDeviceChildHandle<wand::Buffer> const& buf,
    ExtractedFrameData& packet
  ) -> std::uint32_t;

  [[nodiscard]] static
  auto FindOrAddTextureInPacket(
    wand::SharedDeviceChildHandle<wand::Texture> const& tex,
    ExtractedFrameData& packet
  ) -> std::uint32_t;

  [[nodiscard]]
  auto AddMeshToPacket(
    Mesh& mesh,
    RenderFrame& frame,
    ExtractedFrameData& packet
  ) const -> std::uint32_t;

  [[nodiscard]]
  auto FindOrAddMeshInPacket(
    Mesh& mesh,
    RenderFrame& frame,
    ExtractedFrameData& packet
  ) -> std::uint32_t;

  auto AddMeshComponentToPacket(
    MeshComponentBase const& comp,
    GeometryBatch& geom_batch,
    Matrix4 const& prev_local_to_world_mtx,
    RenderFrame& frame,
    ExtractedFrameData& packet
  ) const -> void;

  [[nodiscard]] static
  auto FindOrAddRenderTargetInPacket(
    std::shared_ptr<RenderTarget> const& rt,
    ExtractedFrameData& packet
  ) -> std::uint32_t;

  static
  auto SyncMaterial(
    Material const& mtl,
    RenderMaterial& render_mtl,
    RenderFrame& frame
  ) -> void;

  auto SyncMesh(
    Mesh& mesh,
    RenderMesh& render_mesh,
    RenderFrame& frame
  ) const -> void;

  static
  auto SyncStaticInstance(
    StaticMeshComponent const& comp,
    StaticRenderMeshInstance& inst
  ) -> void;

  auto SyncSkinnedInstance(
    SkinnedMeshComponent const& comp,
    SkinnedRenderMeshInstance& inst
  ) const -> void;

  [[nodiscard]] static
  auto CalculateCameraShadowCascadeBoundaries(
    CameraData const& cam_data,
    ShadowParams const& shadow_params
  ) -> ShadowCascadeBoundaries;

  // Places the indices of the surviving lights in the passed container.
  // Returns the number of surviving lights.
  [[nodiscard]] static
  auto CullLights(
    Frustum const& frustum_ws,
    std::span<LightData const> lights,
    std::vector<unsigned>& visible_light_indices
  ) -> uint64_t;

  [[nodiscard]] static
  auto PrepareDirectionalShadows(
    ExtractedFrameData const& frame_packet,
    std::span<unsigned const> cam_visible_light_indices,
    CameraData const& cam_data,
    ShadowCascadeBoundaries const& shadow_cascade_boundaries,
    float rt_aspect,
    std::uint32_t cascade_count,
    std::uint32_t shadow_map_size,
    std::vector<PreparedView>& views
  ) -> std::optional<PreparedDirectionalShadows>;

  auto RecordDirectionalShadows(
    ExtractedFrameData const& frame_packet,
    RenderFrame const& frame,
    PreparedDirectionalShadows const& shadows,
    wand::CommandList& cmd
  ) -> void;

  static
  auto SetPerFrameConstants(
    MappedConstantBuffer<ShaderPerFrameConstants>& cb,
    Vector3 const& ambient_light,
    ShadowParams const& shadow_params
  ) -> void;

  static
  auto SetPerViewConstants(
    MappedConstantBuffer<ShaderPerViewConstants>& cb,
    Matrix4 const& view_mtx,
    Matrix4 const& proj_mtx,
    Matrix4 const& prev_view_proj_mtx,
    ShadowCascadeBoundaries const& cascade_bounds,
    Frustum const& frustum_ws,
    Vector3 const& view_pos,
    float near_clip_plane,
    float far_clip_plane,
    int rt_width,
    int rt_height
  ) -> void;

  static
  auto SetPerInstanceConstants(
    MappedConstantBuffer<ShaderPerInstanceConstants>& cb,
    Matrix4 const& model_mtx,
    Matrix4 const& view_mtx,
    Matrix4 const& proj_mtx,
    Matrix4 const& prev_model_mtx,
    float max_abs_scaling
  ) -> void;

  auto UpdatePunctualShadowAtlas(
    PunctualShadowAtlas& atlas,
    std::span<LightData const> lights,
    std::span<unsigned const> visible_light_indices,
    CameraData const& cam_data,
    Matrix4 const& cam_view_proj_mtx,
    float shadow_distance
  ) -> void;

  auto DrawPunctualShadowMaps(
    PunctualShadowAtlas const& atlas,
    ExtractedFrameData const& frame_packet,
    std::uint32_t frame_idx,
    wand::CommandList& cmd
  ) -> void;

  auto RecreateSsaoSamples(int sample_count) noexcept -> void;

  auto RecreatePipelines() -> void;

  auto CreatePerViewConstantBuffers(UINT count) -> void;
  auto CreatePerInstanceConstantBuffers(UINT count) -> void;

  auto AcquirePerViewConstantBuffer(std::uint32_t frame_idx) -> MappedConstantBuffer<ShaderPerViewConstants>&;
  auto AcquirePerInstanceConstantBuffer(std::uint32_t frame_idx) -> MappedConstantBuffer<ShaderPerInstanceConstants>&;

  auto OnWindowSize(Extent2D<std::uint32_t> size) -> void;

  auto RecordGpuInitWork(RenderFrame& frame) const -> void;

  auto RecordDepthOnlyPass(
    ExtractedFrameData const& frame_packet,
    RenderFrame const& frame,
    PreparedView const& view, std::uint32_t rt_idx, wand::CommandList& cmd
  ) -> void;

  static
  auto DrawSubmesh(
    SubmeshData const& submesh,
    std::optional<UINT> meshlet_count_param_idx,
    std::optional<UINT> meshlet_offset_param_idx,
    std::optional<UINT> base_vertex_param_idx,
    wand::CommandList const& cmd
  ) -> void;

  static
  auto DrawSubmesh(
    UINT submesh_meshlet_count,
    UINT submesh_meshlet_offset,
    UINT submesh_base_vertex,
    std::optional<UINT> meshlet_count_param_idx,
    std::optional<UINT> meshlet_offset_param_idx,
    std::optional<UINT> base_vertex_param_idx,
    wand::CommandList const& cmd
  ) -> void;

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
  static constexpr unsigned frame_packet_invalid_idx{~0u};

  ObserverPtr<Window> window_;
  ObserverPtr<wand::GraphicsDevice> device_;
  ObserverPtr<RenderManager> render_manager_;
  ObserverPtr<RenderResourceRegistry> resource_registry_;
  ObserverPtr<RenderInstanceRegistry> instance_registry_;

  std::array<MappedStructuredBuffer<ShaderLight>, kFramesInFlight> light_buffers_{
    MappedStructuredBuffer<ShaderLight>{*device_, 0, true, false},
    MappedStructuredBuffer<ShaderLight>{*device_, 0, true, false},
  };
  std::array<MappedConstantBuffer<ShaderPerFrameConstants>, kFramesInFlight> per_frame_cbs_{
    MappedConstantBuffer<ShaderPerFrameConstants>{*device_},
    MappedConstantBuffer<ShaderPerFrameConstants>{*device_}
  };
  std::array<MappedStructuredBuffer<Vector4>, kFramesInFlight> gizmo_color_buffers_{
    MappedStructuredBuffer<Vector4>{*device_},
    MappedStructuredBuffer<Vector4>{*device_},
  };
  std::array<MappedStructuredBuffer<ShaderLineGizmoVertexData>, kFramesInFlight> line_gizmo_vertex_data_buffers_{
    MappedStructuredBuffer<ShaderLineGizmoVertexData>{*device_},
    MappedStructuredBuffer<ShaderLineGizmoVertexData>{*device_}
  };
  std::vector<std::array<MappedConstantBuffer<ShaderPerViewConstants>, kFramesInFlight>> per_view_cbs_;
  std::vector<std::array<MappedConstantBuffer<ShaderPerInstanceConstants>, kFramesInFlight>> per_inst_cbs_;

  wand::SharedDeviceChildHandle<wand::Buffer> ssao_samples_buffer_;

  wand::SharedDeviceChildHandle<wand::Texture> white_tex_;
  wand::SharedDeviceChildHandle<wand::Texture> ssao_noise_tex_;
  wand::SharedDeviceChildHandle<wand::Texture> brdf_integration_map_;

  wand::SharedDeviceChildHandle<wand::PipelineState> shadow_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> depth_resolve_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> line_gizmo_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> gbuffer_velocity_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> deferred_lighting_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> post_process_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> skybox_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> ssao_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> ssao_blur_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> ssr_compose_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> ssr_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> taa_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> vtx_skinning_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> irradiance_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> envmap_prefilter_pso_;
  wand::SharedDeviceChildHandle<wand::PipelineState> brdf_integration_pso_;

  wand::UniqueSamplerHandle samp_cmp_pcf_ge_;
  wand::UniqueSamplerHandle samp_cmp_pcf_le_;
  wand::UniqueSamplerHandle samp_cmp_point_ge_;
  wand::UniqueSamplerHandle samp_cmp_point_le_;
  wand::UniqueSamplerHandle samp_af16_clamp_;
  wand::UniqueSamplerHandle samp_af8_clamp_;
  wand::UniqueSamplerHandle samp_af4_clamp_;
  wand::UniqueSamplerHandle samp_af2_clamp_;
  wand::UniqueSamplerHandle samp_tri_clamp_;
  wand::UniqueSamplerHandle samp_bi_clamp_;
  wand::UniqueSamplerHandle samp_point_clamp_;
  wand::UniqueSamplerHandle samp_af16_wrap_;
  wand::UniqueSamplerHandle samp_af8_wrap_;
  wand::UniqueSamplerHandle samp_af4_wrap_;
  wand::UniqueSamplerHandle samp_af2_wrap_;
  wand::UniqueSamplerHandle samp_tri_wrap_;
  wand::UniqueSamplerHandle samp_bi_wrap_;
  wand::UniqueSamplerHandle samp_point_wrap_;

  std::array<ExtractedFrameData, kFramesInFlight> frame_packets_;
  PreparedFrameData prepared_data_;

  UINT next_per_instance_cb_idx_{0};
  UINT next_per_view_cb_idx_{0};

  std::unique_ptr<DirectionalShadowMapArray> dir_shadow_map_arr_;
  std::unique_ptr<PunctualShadowAtlas> punctual_shadow_atlas_;

  std::vector<Vector4> gizmo_colors_;
  std::vector<ShaderLineGizmoVertexData> line_gizmo_vertex_data_;

  std::vector<Vector4> ssao_samples_;
  bool ssao_samples_changed_{false};

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

  std::vector<StaticMeshComponent*> static_mesh_components_;
  std::vector<SkinnedMeshComponent*> skinned_mesh_components_;
  std::vector<LightComponent const*> lights_;
  std::vector<Camera*> cameras_;

  std::shared_ptr<RenderTarget> main_rt_;
  std::shared_ptr<RenderTarget> rt_override_;

  EventListenerHandle<Extent2D<unsigned>> window_size_event_listener_{};

  bool gpu_init_work_recorded_{false};
};


constexpr auto SceneRenderer::GetMaxShadowCascadeCount() noexcept -> unsigned {
  return MAX_CASCADE_COUNT;
}
}
