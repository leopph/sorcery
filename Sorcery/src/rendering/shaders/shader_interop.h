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

#define MESHLET_MAX_VERTS 128
#define MESHLET_MAX_PRIMS 128

#define MAX_CASCADE_COUNT (uint) 4
#define MAX_PER_LIGHT_SHADOW_MAP_COUNT 6

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

struct ShaderLight {
  row_major float4x4 shadowViewProjMatrices[MAX_PER_LIGHT_SHADOW_MAP_COUNT];

  float3 color;
  float intensity;

  float3 direction;
  int type;

  BOOL isCastingShadow;
  float range;
  float halfInnerAngleCos;
  float halfOuterAngleCos;

  float2 shadowAtlasCellOffsets[MAX_PER_LIGHT_SHADOW_MAP_COUNT];

  float shadowAtlasCellSizes[MAX_PER_LIGHT_SHADOW_MAP_COUNT];
  BOOL sampleShadowMap[MAX_PER_LIGHT_SHADOW_MAP_COUNT];

  float3 position;
  float depthBias;

  float normalBias;
  float3 pad;
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
  float3 ambientLightColor;
  uint shadowCascadeCount;
  float2 screenSize;
  BOOL visualizeShadowCascades;
  int shadowFilteringMode;
};


struct ShaderPerViewConstants {
  row_major float4x4 viewMtx;
  row_major float4x4 invViewMtx;

  row_major float4x4 projMtx;
  row_major float4x4 invProjMtx;

  row_major float4x4 viewProjMtx;
  row_major float4x4 invViewProjMtx;

  float4 shadowCascadeSplitDistances;
  float3 viewPos;
  float pad;
};


struct ShaderPerDrawConstants {
  row_major float4x4 modelMtx;
  row_major float4x4 invTranspModelMtx;

  row_major float4x4 model_view_mtx;
  row_major float4x4 model_view_proj_mtx;
};


struct DepthOnlyDrawParams {
  uint meshlet_count;
  uint meshlet_offset;
  uint instance_count;
  uint instance_offset;

  uint base_vertex;
  uint mtl_idx;
  uint pos_buf_idx;
  uint uv_buf_idx;

  uint vertex_idx_buf_idx;
  uint prim_idx_buf_idx;
  uint meshlet_buf_idx;
  BOOL idx32;

  uint samp_idx;
  uint rt_idx;
  uint per_draw_cb_idx;

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
  uint instance_count;
  uint instance_offset;

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
  uint per_draw_cb_idx;

  uint per_view_cb_idx;
};


struct DeferredLightingDrawParams {
  uint gbuffer0_idx;
  uint gbuffer1_idx;
  uint gbuffer2_idx;
  uint depth_tex_idx;

  uint ssao_tex_idx;
  uint dir_shadow_arr_idx;
  uint punc_shadow_atlas_idx;
  uint shadow_samp_idx;

  uint point_clamp_samp_idx;
  uint light_buf_idx;
  uint light_count;
  uint per_view_cb_idx;

  uint per_frame_cb_idx;
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

#ifdef __cplusplus
} // namespace sorcery
#endif

#endif
