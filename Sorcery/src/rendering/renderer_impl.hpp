#pragma once

#include "graphics.hpp"
#include "directional_shadow_map_array.hpp"
#include "punctual_shadow_atlas.hpp"
#include "Renderer.hpp"
#include "structured_buffer.hpp"
#include "constant_buffer.hpp"
#include "shaders/shader_interop.h"
#include "../Core.hpp"
#include "../Color.hpp"
#include "../Math.hpp"
#include "Visibility.hpp"

#include <array>
#include <memory>
#include <mutex>
#include <vector>


namespace sorcery {
class Renderer::Impl {
  struct TempRenderTargetRecord {
    std::unique_ptr<RenderTarget> rt;
    int age_in_frames;
  };


  constexpr static int max_tmp_rt_age_{10};
  inline static Guid const default_material_guid_{1, 0};
  inline static Guid const cube_mesh_guid_{2, 0};
  inline static Guid const plane_mesh_guid_{3, 0};
  inline static Guid const sphere_mesh_guid_{4, 0};
  static UINT constexpr max_frames_in_flight_{2};
  static DXGI_FORMAT constexpr imprecise_color_buffer_format_{DXGI_FORMAT_R11G11B10_FLOAT};
  static DXGI_FORMAT constexpr precise_color_buffer_format_{DXGI_FORMAT_R16G16B16A16_FLOAT};
  static DXGI_FORMAT constexpr depth_format_{DXGI_FORMAT_D32_FLOAT};
  static DXGI_FORMAT constexpr render_target_format_{DXGI_FORMAT_R8G8B8A8_UNORM};
  static DXGI_FORMAT constexpr ssao_buffer_format_{DXGI_FORMAT_R8_UNORM};
  static DXGI_FORMAT constexpr normal_buffer_format_{DXGI_FORMAT_R8G8B8A8_SNORM};

  std::unique_ptr<graphics::GraphicsDevice> device_;
  graphics::UniqueHandle<graphics::SwapChain> swap_chain_;

  std::array<graphics::UniqueHandle<graphics::CommandList>, max_frames_in_flight_> command_lists_;
  std::array<ConstantBuffer<ShaderPerFrameConstants>, max_frames_in_flight_> per_frame_cbs_;
  std::array<std::vector<ConstantBuffer<ShaderPerViewConstants>>, max_frames_in_flight_> per_view_cbs_;
  std::array<std::vector<ConstantBuffer<ShaderPerDrawConstants>>, max_frames_in_flight_> per_draw_cbs_;

  graphics::UniqueHandle<graphics::Texture> white_tex_;
  graphics::UniqueHandle<graphics::Texture> ssao_noise_tex_;

  graphics::UniqueHandle<graphics::PipelineState> depth_only_pso_;
  graphics::UniqueHandle<graphics::PipelineState> depth_normal_pso_;
  graphics::UniqueHandle<graphics::PipelineState> depth_resolve_pso_;
  graphics::UniqueHandle<graphics::PipelineState> line_gizmo_pso_;
  graphics::UniqueHandle<graphics::PipelineState> object_pso_;
  graphics::UniqueHandle<graphics::PipelineState> post_process_pso_;
  graphics::UniqueHandle<graphics::PipelineState> skybox_pso_;
  graphics::UniqueHandle<graphics::PipelineState> ssao_pso_;
  graphics::UniqueHandle<graphics::PipelineState> ssao_blur_pso_;

  graphics::UniqueHandle<graphics::Sampler> samp_cmp_pcf_ge_;
  graphics::UniqueHandle<graphics::Sampler> samp_cmp_pcf_le_;
  graphics::UniqueHandle<graphics::Sampler> samp_cmp_point_ge_;
  graphics::UniqueHandle<graphics::Sampler> samp_cmp_point_le_;
  graphics::UniqueHandle<graphics::Sampler> samp_af16_clamp_;
  graphics::UniqueHandle<graphics::Sampler> samp_af8_clamp_;
  graphics::UniqueHandle<graphics::Sampler> samp_af4_clamp_;
  graphics::UniqueHandle<graphics::Sampler> samp_af2_clamp_;
  graphics::UniqueHandle<graphics::Sampler> samp_tri_clamp_;
  graphics::UniqueHandle<graphics::Sampler> samp_bi_clamp_;
  graphics::UniqueHandle<graphics::Sampler> samp_point_clamp_;
  graphics::UniqueHandle<graphics::Sampler> samp_af16_wrap_;
  graphics::UniqueHandle<graphics::Sampler> samp_af8_wrap_;
  graphics::UniqueHandle<graphics::Sampler> samp_af4_wrap_;
  graphics::UniqueHandle<graphics::Sampler> samp_af2_wrap_;
  graphics::UniqueHandle<graphics::Sampler> samp_tri_wrap_;
  graphics::UniqueHandle<graphics::Sampler> samp_bi_wrap_;
  graphics::UniqueHandle<graphics::Sampler> samp_point_wrap_;

  UINT frame_idx_{0};

  ObserverPtr<Material> default_material_{nullptr};
  ObserverPtr<Mesh> cube_mesh_{nullptr};
  ObserverPtr<Mesh> plane_mesh_{nullptr};
  ObserverPtr<Mesh> sphere_mesh_{nullptr};

  std::unique_ptr<DirectionalShadowMapArray> dir_shadow_map_arr_;
  std::unique_ptr<PunctualShadowAtlas> punctual_shadow_atlas_;
  std::unique_ptr<StructuredBuffer<ShaderLight>> light_buffer_;
  std::unique_ptr<StructuredBuffer<Vector4>> gizmo_color_buffer_;
  std::unique_ptr<StructuredBuffer<ShaderLineGizmoVertexData>> line_gizmo_vertex_data_buffer_;
  std::unique_ptr<RenderTarget> main_rt_;
  std::unique_ptr<StructuredBuffer<Vector4>> ssao_samples_buffer_;

  std::vector<StaticMeshComponent const*> static_mesh_components_;
  std::vector<LightComponent const*> lights_;
  std::vector<Camera const*> game_render_cameras_;

  std::vector<Vector4> gizmo_colors_;
  std::vector<ShaderLineGizmoVertexData> line_gizmo_vertex_data_;

  // Normalized to [0, 1]
  std::array<float, MAX_CASCADE_COUNT - 1> cascade_splits_{0.1f, 0.3f, 0.6f};
  int cascade_count_{4};
  bool shadow_cascades_{false};
  float shadow_distance_{100};
  ShadowFilteringMode shadow_filtering_mode_{ShadowFilteringMode::PCFTent5x5};

  MultisamplingMode msaa_mode_{MultisamplingMode::X8};

  int sync_interval_{0};

  float inv_gamma_{1.f / 2.2f};

  bool depth_normal_pre_pass_enabled_{true};
  bool ssao_enabled_{true};
  SsaoParams ssao_params_{.radius = 0.1f, .bias = 0.025f, .power = 6.0f, .sampleCount = 12};

  DXGI_FORMAT color_buffer_format_{imprecise_color_buffer_format_};

  std::mutex static_mesh_mutex_;
  std::mutex light_mutex_;
  std::mutex game_camera_mutex_;
  std::mutex tmp_render_targets_mutex_;

  std::vector<TempRenderTargetRecord> tmp_render_targets_;

  UINT next_per_draw_cb_idx_{0};
  UINT next_per_view_cb_idx_{0};

  [[nodiscard]] auto CalculateCameraShadowCascadeBoundaries(Camera const& cam) const -> ShadowCascadeBoundaries;

  auto CullStaticMeshComponents(Frustum const& frustum_ws,
                                std::pmr::vector<StaticMeshSubmeshIndex>& visible_indices) const -> void;
  auto CullLights(Frustum const& frustum_ws, std::pmr::vector<int> visible_indices) const -> void;

  auto SetPerFrameConstants(ConstantBuffer<ShaderPerFrameConstants>& cb, int rt_width,
                            int rt_height) const noexcept -> void;
  static auto SetPerViewConstants(ConstantBuffer<ShaderPerViewConstants>& cb, Matrix4 const& view_mtx,
                                  Matrix4 const& proj_mtx, ShadowCascadeBoundaries const& cascade_bounds,
                                  Vector3 const& view_pos) -> void;
  static auto SetPerDrawConstants(ConstantBuffer<ShaderPerDrawConstants>& cb, Matrix4 const& model_mtx) -> void;

  auto DrawDirectionalShadowMaps(std::span<int const> light_indices, Camera const& cam, float rt_aspect,
                                 ShadowCascadeBoundaries const& shadow_cascade_boundaries,
                                 std::array<Matrix4, MAX_CASCADE_COUNT>& shadow_view_proj_matrices,
                                 graphics::CommandList& cmd) -> void;
  auto DrawPunctualShadowMaps(PunctualShadowAtlas const& atlas, graphics::CommandList& cmd) -> void;
  auto DrawSkybox(graphics::CommandList& cmd) noexcept -> void;
  auto PostProcess(graphics::Texture const& src, graphics::Texture const& dst,
                   graphics::CommandList& cmd) const noexcept -> void;

  auto ClearGizmoDrawQueue() noexcept -> void;
  auto ReleaseTempRenderTargets() noexcept -> void;

  auto RecreateSsaoSamples(int sample_count) const noexcept -> void;

  [[nodiscard]] auto RecreatePipelines() -> bool;
  auto CreatePerViewConstantBuffers(UINT count) -> void;
  auto CreatePerDrawConstantBuffers(UINT count) -> void;

  static auto OnWindowSize(Impl* self, Extent2D<std::uint32_t> size) -> void;

  static auto GenerateSphere(float radius, int latitudes, int longitudes, std::vector<Vector3>& vertices,
                             std::vector<Vector3>& normals, std::vector<Vector2>& uvs,
                             std::vector<std::uint32_t>& indices) -> void;

public:
  static auto GetProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4;

  // LIFETIME FUNCTIONS

  auto StartUp() -> void;
  auto ShutDown() -> void;

  // FRAME RENDERING FUNCTIONS

  auto DrawCamera(Camera const& cam, RenderTarget const* rt = nullptr) -> void;
  auto DrawAllCameras(RenderTarget const* rt = nullptr) -> void;

  auto DrawLineAtNextRender(Vector3 const& from, Vector3 const& to, Color const& color) -> void;
  auto DrawGizmos(RenderTarget const* rt = nullptr) -> void;

  /*auto ClearAndBindMainRt(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void;
  auto BlitMainRtToSwapChain(ObserverPtr<ID3D11DeviceContext> ctx) const noexcept -> void; TODO*/

  auto Present() noexcept -> void;

  // FUNCTIONS FOR CUSTOM EXTERNAL REQUESTS

  [[nodiscard]] auto GetDevice() const noexcept -> ObserverPtr<graphics::GraphicsDevice>;

  auto GetTemporaryRenderTarget(RenderTarget::Desc const& desc) -> RenderTarget&;

  // DEFAULT RENDERING RESOURCES

  [[nodiscard]] auto GetDefaultMaterial() const noexcept -> ObserverPtr<Material>;
  [[nodiscard]] auto GetCubeMesh() const noexcept -> ObserverPtr<Mesh>;
  [[nodiscard]] auto GetPlaneMesh() const noexcept -> ObserverPtr<Mesh>;
  [[nodiscard]] auto GetSphereMesh() const noexcept -> ObserverPtr<Mesh>;

  // RENDER STATE CONFIGURATION

  [[nodiscard]] auto GetSyncInterval() const noexcept -> int;
  auto SetSyncInterval(int interval) noexcept -> void;

  [[nodiscard]] auto GetMultisamplingMode() const noexcept -> MultisamplingMode;
  auto SetMultisamplingMode(MultisamplingMode mode) noexcept -> void;

  [[nodiscard]] auto IsDepthNormalPrePassEnabled() const noexcept -> bool;
  auto SetDepthNormalPrePassEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] auto IsUsingPreciseColorFormat() const noexcept -> bool;
  auto SetUsePreciseColorFormat(bool value) noexcept -> void;

  // SHADOWS

  [[nodiscard]] auto GetShadowDistance() const noexcept -> float;
  auto SetShadowDistance(float shadowDistance) noexcept -> void;

  [[nodiscard]] constexpr static auto GetMaxShadowCascadeCount() noexcept -> int;
  [[nodiscard]] auto GetShadowCascadeCount() const noexcept -> int;
  auto SetShadowCascadeCount(int cascadeCount) noexcept -> void;

  [[nodiscard]] auto GetNormalizedShadowCascadeSplits() const noexcept -> std::span<float const>;
  auto SetNormalizedShadowCascadeSplit(int idx, float split) noexcept -> void;

  [[nodiscard]] auto IsVisualizingShadowCascades() const noexcept -> bool;
  auto VisualizeShadowCascades(bool visualize) noexcept -> void;

  [[nodiscard]] auto GetShadowFilteringMode() const noexcept -> ShadowFilteringMode;
  auto SetShadowFilteringMode(ShadowFilteringMode filteringMode) noexcept -> void;

  // LIGHTING

  [[nodiscard]] auto IsSsaoEnabled() const noexcept -> bool;
  auto SetSsaoEnabled(bool enabled) noexcept -> void;

  [[nodiscard]] auto GetSsaoParams() const noexcept -> SsaoParams const&;
  auto SetSsaoParams(SsaoParams const& ssaoParams) noexcept -> void;

  // POST-PROCESSING

  [[nodiscard]] auto GetGamma() const noexcept -> float;
  auto SetGamma(float gamma) noexcept -> void;

  // REGISTRATION FUNCTIONS

  auto Register(StaticMeshComponent const& staticMeshComponent) noexcept -> void;
  auto Unregister(StaticMeshComponent const& staticMeshComponent) noexcept -> void;

  auto Register(LightComponent const& lightComponent) noexcept -> void;
  auto Unregister(LightComponent const& lightComponent) noexcept -> void;

  auto Register(Camera const& cam) noexcept -> void;
  auto Unregister(Camera const& cam) noexcept -> void;
};


constexpr auto Renderer::Impl::GetMaxShadowCascadeCount() noexcept -> int {
  return MAX_CASCADE_COUNT;
}
}
