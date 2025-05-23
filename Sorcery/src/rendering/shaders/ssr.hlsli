#ifndef SSR_HLSLI
#define SSR_HLSLI

#include "common.hlsli"
#include "fullscreen_tri.hlsli"
#include "gbuffer_utils.hlsli"
#include "raytrace.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"

DECLARE_PARAMS(SsrDrawParams);
DECLARE_PARAMS1(SsrComposeDrawParams, g_compose_params);


float4 PsMain(PsIn const ps_in) : SV_Target {
  Texture2D<float> const depth_tex = ResourceDescriptorHeap[g_params.depth_tex_idx];
  Texture2D<float2> const gbuffer1 = ResourceDescriptorHeap[g_params.gbuffer1_tex_idx];
  Texture2D<float2> const gbuffer2 = ResourceDescriptorHeap[g_params.gbuffer2_tex_idx];
  Texture2D const lit_scene_tex = ResourceDescriptorHeap[g_params.lit_scene_tex_idx];
  SamplerState const point_clamp_samp = SamplerDescriptorHeap[g_params.point_clamp_samp_idx];
  ConstantBuffer<ShaderPerViewConstants> const per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

  float3 frag_norm_ws;
  float roughness;
  float metallic;
  UnpackGBuffer1(gbuffer1.Sample(point_clamp_samp, ps_in.uv.xy), frag_norm_ws);
  UnpackGBuffer2(gbuffer2.Sample(point_clamp_samp, ps_in.uv.xy), roughness, metallic);

  float4 reflection_color = float4(0, 0, 0, 1);

  if (roughness <= g_params.max_roughness) {
    float const depth = depth_tex.Sample(point_clamp_samp, ps_in.uv).r;

    float3 const frag_pos_ndc = float3(UvToNdc(ps_in.uv), depth);
    float4 const frag_pos4_vs = mul(float4(frag_pos_ndc, 1), per_view_cb.invProjMtx);
    float3 const frag_pos_vs = frag_pos4_vs.xyz / frag_pos4_vs.w;

    float3 const frag_norm_vs = mul(float4(frag_norm_ws, 0), per_view_cb.viewMtx).xyz;
    float3 const refl_start_vs = frag_pos_vs + g_params.ray_start_bias_vs * frag_norm_vs;
    float3 const refl_dir_vs = reflect(normalize(frag_pos_vs), frag_norm_vs);

    float2 hit_pixel_coords;
    float3 hit_pos_vs;

    float2 depth_tex_size;
    depth_tex.GetDimensions(depth_tex_size.x, depth_tex_size.y);

    float2 const half_depth_tex_size = depth_tex_size * 0.5;

    float4x4 const cs_to_px_mtx = {
      half_depth_tex_size.x, 0, 0, 0,
      0, -half_depth_tex_size.y, 0, 0,
      0, 0, 1, 0,
      half_depth_tex_size.x, half_depth_tex_size.y, 0, 1
    };

    int2 const ifrag_pos = int2(ps_in.pos_cs.xy);
    float const jitter = ((ifrag_pos.x + ifrag_pos.y) & 1) * 0.5;

    if (traceScreenSpaceRay(refl_start_vs, refl_dir_vs, mul(per_view_cb.projMtx, cs_to_px_mtx), depth_tex,
      depth_tex_size, g_params.thickness_vs, per_view_cb.near_clip_plane, per_view_cb.far_clip_plane, g_params.stride,
      jitter, max(depth_tex_size.x, depth_tex_size.y), g_params.max_trace_dist_vs, hit_pixel_coords,
      hit_pos_vs)) {
      reflection_color = lit_scene_tex.Load(int3(int2(hit_pixel_coords), 0));
    }
  }

  return reflection_color;
}


float4 ComposePsMain(PsIn const ps_in) : SV_Target {
  Texture2D const lit_scene_tex = ResourceDescriptorHeap[g_compose_params.lit_scene_tex_idx];
  Texture2D const ssr_tex = ResourceDescriptorHeap[g_compose_params.ssr_tex_idx];
  SamplerState const point_clamp_samp = SamplerDescriptorHeap[g_compose_params.point_clamp_samp_idx];

  float4 const lit_color = lit_scene_tex.Sample(point_clamp_samp, ps_in.uv);
  float4 const ssr_color = ssr_tex.Sample(point_clamp_samp, ps_in.uv);

  return lit_color + ssr_color;
}

#endif
