#ifndef SHADER_INTEROP_H
#define SHADER_INTEROP_H

#ifdef __cplusplus

#include <cstdint>

#include "../../Math.hpp"


namespace sorcery {
using float2 = Vector2;
using float3 = Vector3;
using float4 = Vector4;
using float4x4 = Matrix4;

using uint = unsigned;
using uint2 = Vector<std::uint32_t, 2>;
using uint3 = Vector<std::uint32_t, 3>;

using BOOL = int;

#define row_major
#else
typedef bool BOOL;
#endif

#define INVALID_RES_IDX ((uint)-1)
#define INVALID_IDX ((uint)-1)

#define MESHLET_MAX_VERTS 128
#define MESHLET_MAX_PRIMS 128

#define AS_THREAD_GROUP_SIZE 32

#define MAX_CASCADE_COUNT (uint) 4

#define BLEND_MODE_OPAQUE 0
#define BLEND_MODE_ALPHA_CLIP 1

#define SSAO_NOISE_TEX_DIM 4

#define DEPTH_RESOLVE_CS_THREADS_X 16
#define DEPTH_RESOLVE_CS_THREADS_Y 16
#define DEPTH_RESOLVE_CS_THREADS_Z 1

#define SKINNING_CS_THREADS 64

#define REVERSE_Z
#ifdef REVERSE_Z
#define DEPTH_CLEAR_VALUE 0.0f
#else
#define DEPTH_CLEAR_VALUE 1.0f
#endif

#define LIGHT_DIRECTIONAL 0
#define LIGHT_SPOT 1
#define LIGHT_POINT 2


struct ShaderLight {
  float3 color;
  float intensity;
  float3 direction;
  int type;
  float3 position;
  float range;
  float half_inner_angle_cos;
  float half_outer_angle_cos;
};


struct ShaderPositionalLightShadow {
  row_major float4x4 view_proj_matrices[6];
  float2 atlas_offsets[6];
  float2 atlas_scales[6];
  uint allocated_mask;
  float depth_bias;
  float normal_bias;
};


struct ShaderVisibleLight {
  uint light_idx;
  uint positional_shadow_idx;
};


struct ShaderDirectionalLightShadow {
  float4 split_distances;

  row_major float4x4 view_proj_matrices[MAX_CASCADE_COUNT];

  float depth_bias;
  float normal_bias;
  uint light_idx;
};


struct ShaderCameraLightingData {
  uint first_visible_light;
  uint visible_light_count;
  uint2 pad;
  ShaderDirectionalLightShadow dir_shadow;
};


struct ShaderMaterial {
  float3 albedo;
  float metallic;

  float roughness;
  float ao;
  float alphaThreshold;
  uint albedo_map_idx;

  uint metallic_map_idx;
  uint roughness_map_idx;
  uint ao_map_idx;
  uint normal_map_idx;

  uint opacity_map_idx;
  int blendMode;
  float2 pad;
};


struct ShaderLineGizmoVertexData {
  float3 from;
  uint colorIdx;
  float3 to;
  float pad;
};


struct ShaderPerFrameConstants {
  float3 ambient_light_color;
  uint dir_shadow_cascade_count;
  BOOL visualize_dir_shadow_cascades;
  int shadow_filtering_mode;
};


struct ShaderPerViewConstants {
  row_major float4x4 viewMtx;
  row_major float4x4 invViewMtx;

  row_major float4x4 projMtx;
  row_major float4x4 invProjMtx;

  row_major float4x4 viewProjMtx;
  row_major float4x4 invViewProjMtx;

  row_major float4x4 prev_view_proj_mtx;

  float4 frustum_planes_ws[6];

  float3 viewPos;
  float near_clip_plane;

  float far_clip_plane;
  float2 screenSize;
  float pad;
};


struct ShaderPerInstanceConstants {
  row_major float4x4 modelMtx;
  row_major float4x4 invTranspModelMtx;

  row_major float4x4 model_view_mtx;
  row_major float4x4 model_view_proj_mtx;

  row_major float4x4 prev_model_mtx;

  float max_abs_scaling;
};


struct DepthOnlyDrawParams {
  uint meshlet_count;
  uint meshlet_offset;
  uint base_vertex;
  uint mtl_idx;

  uint pos_buf_idx;
  uint uv_buf_idx;
  uint vertex_idx_buf_idx;
  uint prim_idx_buf_idx;

  uint meshlet_buf_idx;
  uint cull_data_buf_idx;
  BOOL idx32;
  uint samp_idx;

  uint rt_idx;
  uint per_inst_cb_idx;
  uint per_view_cb_idx;
};


struct DepthResolveDrawParams {
  uint in_tex_idx;
  uint out_tex_idx;
};


struct GizmoDrawParams {
  uint vertex_buf_idx;
  uint color_buf_idx;
  uint per_view_cb_idx;
};


struct GBufferDrawParams {
  uint meshlet_count;
  uint meshlet_offset;
  uint base_vertex;
  uint mtl_idx;

  uint pos_buf_idx;
  uint norm_buf_idx;
  uint tan_buf_idx;
  uint uv_buf_idx;

  uint vertex_idx_buf_idx;
  uint prim_idx_buf_idx;
  uint meshlet_buf_idx;
  BOOL idx32;

  uint mtl_samp_idx;
  uint per_inst_cb_idx;
  uint cull_data_buf_idx;
  uint per_view_cb_idx;

  uint prev_frame_pos_buf_idx;
  float jitter_x;
  float jitter_y;
  float prev_jitter_x;

  float prev_jitter_y;
};


struct DeferredLightingDrawParams {
  uint gbuffer0_idx;
  uint gbuffer1_idx;
  uint gbuffer2_idx;
  uint depth_tex_idx;

  uint ssao_tex_idx;
  uint per_view_cb_idx;
  uint cam_light_data_buf_idx;
  uint pos_shadow_buf_idx;

  uint visible_light_buf_idx;
  uint light_buf_idx;
  uint pos_shadow_atlas_idx;
  uint dir_shadow_arr_idx;

  uint per_frame_cb_idx;
  uint irradiance_map_idx;
  uint prefiltered_env_map_idx;
  uint brdf_integration_map_idx;

  uint shadow_samp_idx;
  uint point_clamp_samp_idx;
  uint bi_clamp_samp_idx;
  uint tri_clamp_samp_idx;
};


struct PostProcessDrawParams {
  uint in_tex_idx;
  float inv_gamma;
  uint bi_clamp_samp_idx;
};


struct SkyboxDrawParams {
  uint pos_buf_idx;
  uint vertex_idx_buf_idx;
  uint prim_idx_buf_idx;
  uint meshlet_buf_idx;
  uint per_view_cb_idx;
  uint cubemap_idx;
  uint samp_idx;
};


struct SsaoDrawParams {
  uint noise_tex_idx;
  uint depth_tex_idx;
  uint gbuffer1_tex_idx;
  uint samp_buf_idx;
  uint point_clamp_samp_idx;
  uint point_wrap_samp_idx;
  float radius;
  float bias;
  float power;
  int sample_count;
  uint per_view_cb_idx;
  uint per_frame_cb_idx;
};


struct SsaoBlurDrawParams {
  uint in_tex_idx;
  uint point_clamp_samp_idx;
};


struct VertexSkinningDrawParams {
  uint vtx_buf_idx;
  uint norm_buf_idx;
  uint tan_buf_idx;
  uint bone_weight_buf_idx;

  uint bone_idx_buf_idx;
  uint bone_buf_idx;
  uint skinned_vtx_buf_idx;
  uint skinned_norm_buf_idx;

  uint skinned_tan_buf_idx;
  uint vtx_count;
};


struct SsrDrawParams {
  uint depth_tex_idx;
  uint lit_scene_tex_idx;
  uint gbuffer1_tex_idx;
  uint gbuffer2_tex_idx;

  uint point_clamp_samp_idx;
  uint per_view_cb_idx;
  float max_roughness;
  float thickness_vs;

  float stride;
  float max_trace_dist_vs;
  float ray_start_bias_vs;
};


struct SsrComposeDrawParams {
  uint lit_scene_tex_idx;
  uint ssr_tex_idx;
  uint point_clamp_samp_idx;
};


struct TaaResolveDrawParams {
  uint accum_tex_idx;
  uint color_tex_idx;
  uint depth_tex_idx;
  uint velocity_tex_idx;
  uint linear_samp_idx;
  float jitter_x;
  float jitter_y;
};


struct IrradianceDrawParams {
  row_major float4x4 view_proj_mtx;

  uint rt_idx;
  uint meshlet_buf_idx;
  uint vertex_idx_buf_idx;
  uint prim_idx_buf_idx;

  uint pos_buf_idx;
  uint environment_map_idx;
  uint bi_clamp_samp_idx;
};


struct EnvmapPrefilterDrawParams {
  row_major float4x4 view_proj_mtx;

  uint rt_idx;
  float roughness;
  uint meshlet_buf_idx;
  uint vertex_idx_buf_idx;

  uint prim_idx_buf_idx;
  uint pos_buf_idx;
  uint env_map_idx;
  uint tri_clamp_samp_idx;
};

#ifdef __cplusplus
} // namespace sorcery
#endif

#endif
