#pragma once

#include "Camera.hpp"
#include "constant_buffer.hpp"
#include "directional_shadow_map_array.hpp"
#include "graphics.hpp"
#include "punctual_shadow_atlas.hpp"
#include "render_manager.hpp"
#include "render_target.hpp"
#include "shaders/shader_interop.h"
#include "shaders/shadow_filtering_modes.h"
#include "structured_buffer.hpp"
#include "../Color.hpp"
#include "../Math.hpp"
#include "../scene_objects/StaticMeshComponent.hpp"
#include "../scene_objects/LightComponents.hpp"
#include "../Util.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>


namespace sorcery::rendering {
// Passing these enum values to shaders is valid
enum class ShadowFilteringMode : int {
  kNone        = SHADOW_FILTERING_NONE,
  kHardwarePcf = SHADOW_FILTERING_HARDWARE_PCF,
  kPcf3X3      = SHADOW_FILTERING_PCF_3x3,
  kPcfTent3X3  = SHADOW_FILTERING_PCF_TENT_3x3,
  kPcfTent5X5  = SHADOW_FILTERING_PCF_TENT_5x5
};


// Cast to int to get the sample count
enum class MultisamplingMode : int {
  kOff = 1,
  kX2  = 2,
  kX4  = 4,
  kX8  = 8
};


struct SsaoParams {
  float radius;
  float bias;
  float power;
  int sample_count;
};


struct ShadowParams {
  std::array<float, MAX_CASCADE_COUNT - 1> normalized_cascade_splits;
  int cascade_count;
  bool visualize_cascades;
  float distance;
  ShadowFilteringMode filtering_mode;
};


class SceneRenderer {
public:
  auto StartUp() -> void;
  auto ShutDown() -> void;

  auto Render() -> void;

  auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;
  auto DrawGizmos(RenderTarget const* rt = nullptr) -> void;

  /*auto ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void;
  auto BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void; TODO*/

  auto Present() noexcept -> void;

  [[nodiscard]] auto GetSyncInterval() const noexcept -> UINT;
  auto SetSyncInterval(UINT interval) noexcept -> void;

  [[nodiscard]] auto GetMultisamplingMode() const noexcept -> MultisamplingMode;
  auto SetMultisamplingMode(MultisamplingMode mode) noexcept -> void;

  [[nodiscard]] auto IsDepthNormalPrePassEnabled() const noexcept -> bool;
  auto SetDepthNormalPrePassEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] auto IsUsingPreciseColorFormat() const noexcept -> bool;
  auto SetUsePreciseColorFormat(bool precise) noexcept -> void;

  [[nodiscard]] auto GetShadowDistance() const noexcept -> float;
  auto SetShadowDistance(float distance) noexcept -> void;

  [[nodiscard]] constexpr static auto GetMaxShadowCascadeCount() noexcept -> int;
  [[nodiscard]] auto GetShadowCascadeCount() const noexcept -> int;
  auto SetShadowCascadeCount(int cascade_count) noexcept -> void;

  [[nodiscard]] auto GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const>;
  auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;

  [[nodiscard]] auto IsVisualizingShadowCascades() const noexcept -> bool;
  auto VisualizeShadowCascades(bool visualize) noexcept -> void;

  [[nodiscard]] auto GetShadowFilteringMode() const noexcept -> ShadowFilteringMode;
  auto SetShadowFilteringMode(ShadowFilteringMode filtering_mode) noexcept -> void;

  [[nodiscard]] auto IsSsaoEnabled() const noexcept -> bool;
  auto SetSsaoEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] auto GetSsaoParams() const noexcept -> SsaoParams const&;
  auto SetSsaoParams(SsaoParams const& ssao_params) noexcept -> void;

  [[nodiscard]] auto GetGamma() const noexcept -> float;
  auto SetGamma(float gamma) noexcept -> void;

  auto Register(StaticMeshComponent const& static_mesh_component) noexcept -> void;
  auto Unregister(StaticMeshComponent const& static_mesh_component) noexcept -> void;

  auto Register(LightComponent const& light_component) noexcept -> void;
  auto Unregister(LightComponent const& light_component) noexcept -> void;

  auto Register(Camera const& cam) noexcept -> void;
  auto Unregister(Camera const& cam) noexcept -> void;

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
    unsigned idx_buf_local_idx;
    AABB bounds;
  };


  struct SubmeshData {
    unsigned mesh_local_idx;
    INT base_vertex;
    UINT first_index;
    UINT index_count;
    UINT mtl_buf_local_idx;
    AABB bounds;
  };


  struct InstanceData {
    unsigned submesh_local_idx;
    Matrix4 local_to_world_mtx;
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

    std::shared_ptr<RenderTarget> render_target;
  };


  struct FramePacket {
    std::vector<graphics::SharedDeviceChildHandle<graphics::Buffer>> buffers;
    std::vector<graphics::SharedDeviceChildHandle<graphics::Texture>> textures;
    std::vector<LightData> light_data;
    std::vector<MeshData> mesh_data;
    std::vector<SubmeshData> submesh_data;
    std::vector<InstanceData> instance_data;
    std::vector<CameraData> cam_data;
  };


  auto ExtractCurrentState(FramePacket& packet) const -> void;

  [[nodiscard]] auto
  CalculateCameraShadowCascadeBoundaries(CameraData const& cam_data) const -> ShadowCascadeBoundaries;

  // Culling

  static auto CullLights(Frustum const& frustum_ws, std::span<LightData const> lights,
                         std::pmr::vector<unsigned> visible_light_indices) -> void;
  static auto CullStaticSubmeshInstances(Frustum const& frustum_ws, std::span<MeshData const> meshes,
                                         std::span<SubmeshData const> submeshes,
                                         std::span<InstanceData const> instances,
                                         std::pmr::vector<unsigned>& visible_static_submesh_instance_indices) -> void;

  // Constant buffers

  auto SetPerFrameConstants(ConstantBuffer<ShaderPerFrameConstants>& cb, int rt_width,
                            int rt_height) const noexcept -> void;
  static auto SetPerViewConstants(ConstantBuffer<ShaderPerViewConstants>& cb, Matrix4 const& view_mtx,
                                  Matrix4 const& proj_mtx, ShadowCascadeBoundaries const& cascade_bounds,
                                  Vector3 const& view_pos) -> void;
  static auto SetPerDrawConstants(ConstantBuffer<ShaderPerDrawConstants>& cb, Matrix4 const& model_mtx) -> void;

  // Shadow map preparation
  auto UpdatePunctualShadowAtlas(PunctualShadowAtlas& atlas, std::span<LightData const> const lights,
                                 std::span<unsigned const> visible_light_indices, CameraData const& cam_data,
                                 Matrix4 const& cam_view_proj_mtx, float shadow_distance) -> void;

  // Shadow map rendering

  auto DrawDirectionalShadowMaps(FramePacket const& frame_packet, std::span<unsigned const> visible_light_indices,
                                 CameraData const& cam_data, float rt_aspect,
                                 ShadowCascadeBoundaries const& shadow_cascade_boundaries,
                                 std::array<Matrix4, MAX_CASCADE_COUNT>& shadow_view_proj_matrices,
                                 graphics::CommandList& cmd) -> void;
  auto DrawPunctualShadowMaps(PunctualShadowAtlas const& atlas, FramePacket const& frame_packet,
                              graphics::CommandList& cmd) -> void;

  auto DrawSkybox(graphics::CommandList& cmd) noexcept -> void;
  auto PostProcess(graphics::Texture const& src, graphics::Texture const& dst,
                   graphics::CommandList& cmd) const noexcept -> void;

  auto ClearGizmoDrawQueue() noexcept -> void;

  auto RecreateSsaoSamples(int sample_count) noexcept -> void;

  [[nodiscard]] auto RecreatePipelines() -> bool;
  auto CreatePerViewConstantBuffers(UINT count) -> void;
  auto CreatePerDrawConstantBuffers(UINT count) -> void;

  auto AcquirePerViewConstantBuffer() -> ConstantBuffer<ShaderPerViewConstants>&;
  auto AcquirePerDrawConstantBuffer() -> ConstantBuffer<ShaderPerDrawConstants>&;

  auto EndFrame() -> void;

  static auto OnWindowSize(SceneRenderer* self, Extent2D<std::uint32_t> size) -> void;

  static UINT constexpr max_frames_in_flight_{2};
  static DXGI_FORMAT constexpr imprecise_color_buffer_format_{DXGI_FORMAT_R11G11B10_FLOAT};
  static DXGI_FORMAT constexpr precise_color_buffer_format_{DXGI_FORMAT_R16G16B16A16_FLOAT};
  static DXGI_FORMAT constexpr depth_format_{DXGI_FORMAT_D32_FLOAT};
  static DXGI_FORMAT constexpr render_target_format_{DXGI_FORMAT_R8G8B8A8_UNORM};
  static DXGI_FORMAT constexpr ssao_buffer_format_{DXGI_FORMAT_R8_UNORM};
  static DXGI_FORMAT constexpr normal_buffer_format_{DXGI_FORMAT_R8G8B8A8_SNORM};

  std::unique_ptr<graphics::GraphicsDevice> device_;
  graphics::SharedDeviceChildHandle<graphics::SwapChain> swap_chain_;

  std::array<ConstantBuffer<ShaderPerFrameConstants>, max_frames_in_flight_> per_frame_cbs_;
  std::vector<std::array<ConstantBuffer<ShaderPerViewConstants>, max_frames_in_flight_>> per_view_cbs_;
  std::vector<std::array<ConstantBuffer<ShaderPerDrawConstants>, max_frames_in_flight_>> per_draw_cbs_;
  std::array<StructuredBuffer<ShaderLight>, max_frames_in_flight_> light_buffers_;

  graphics::SharedDeviceChildHandle<graphics::Texture> white_tex_;
  graphics::SharedDeviceChildHandle<graphics::Texture> ssao_noise_tex_;

  graphics::SharedDeviceChildHandle<graphics::PipelineState> depth_only_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> depth_normal_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> depth_resolve_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> line_gizmo_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> object_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> post_process_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> skybox_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> ssao_pso_;
  graphics::SharedDeviceChildHandle<graphics::PipelineState> ssao_blur_pso_;

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

  std::array<FramePacket, max_frames_in_flight_> frame_packets_;

  UINT next_per_draw_cb_idx_{0};
  UINT next_per_view_cb_idx_{0};

  std::unique_ptr<DirectionalShadowMapArray> dir_shadow_map_arr_;
  std::unique_ptr<PunctualShadowAtlas> punctual_shadow_atlas_;
  StructuredBuffer<Vector4> gizmo_color_buffer_;
  StructuredBuffer<ShaderLineGizmoVertexData> line_gizmo_vertex_data_buffer_;
  std::unique_ptr<RenderTarget> main_rt_;
  StructuredBuffer<Vector4> ssao_samples_buffer_;


  std::vector<Vector4> gizmo_colors_;
  std::vector<ShaderLineGizmoVertexData> line_gizmo_vertex_data_;

  MultisamplingMode msaa_mode_{MultisamplingMode::kX8};
  SsaoParams ssao_params_{.radius = 0.1f, .bias = 0.025f, .power = 6.0f, .sample_count = 12};
  ShadowParams shadow_params_{{0.1f, 0.3f, 0.6f}, 4, false, 100, ShadowFilteringMode::kPcfTent5X5};

  float inv_gamma_{1.f / 2.2f};

  bool depth_normal_pre_pass_enabled_{true};
  bool ssao_enabled_{true};

  UINT sync_interval_{0};

  DXGI_FORMAT color_buffer_format_{imprecise_color_buffer_format_};

  std::mutex static_mesh_mutex_;
  std::vector<StaticMeshComponent const*> static_mesh_components_;

  std::mutex light_mutex_;
  std::vector<LightComponent const*> lights_;

  std::mutex game_camera_mutex_;
  std::vector<Camera const*> game_render_cameras_;
};


constexpr auto SceneRenderer::GetMaxShadowCascadeCount() noexcept -> int {
  return MAX_CASCADE_COUNT;
}
}
