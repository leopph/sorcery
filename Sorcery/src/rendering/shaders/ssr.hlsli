#ifndef SSR_HLSLI
#define SSR_HLSLI

#include "common.hlsli"
#include "fullscreen_tri.hlsli"
#include "gbuffer_utils.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"

DECLARE_PARAMS(SsrDrawParams);
DECLARE_PARAMS1(SsrComposeDrawParams, g_compose_params);

// https://sugulee.wordpress.com/2021/01/16/performance-optimizations-for-screen-space-reflections-technique-part-1-linear-tracing-method/


void ComputePosAndReflection(float2 const uv, float3 const frag_norm_vs, float4x4 const proj_mtx,
                             float4x4 const inv_proj_mtx, Texture2D<float> const depth_tex,
                             SamplerState const point_clamp_samp, out float3 frag_pos_ts, out float3 refl_dir_ts,
                             out float max_trace_dist) {
  float const depth = depth_tex.Sample(point_clamp_samp, uv).r;

  float3 const frag_pos_ndc = float3(UvToNdc(uv.xy), depth);
  float4 const frag_pos4_vs = mul(float4(frag_pos_ndc, 1), inv_proj_mtx);
  float3 const frag_pos_vs = frag_pos4_vs.xyz / frag_pos4_vs.w;

  float3 const dir_cam_to_frag_vs = normalize(frag_pos_vs);
  float3 const refl_dir_vs = reflect(dir_cam_to_frag_vs, frag_norm_vs);

  float3 refl_ray_end_pos_vs = frag_pos_vs + refl_dir_vs * g_params.ray_length_vs;
  refl_ray_end_pos_vs /= refl_ray_end_pos_vs.z < 0 ? refl_ray_end_pos_vs.z : 1;

  float4 const refl_ray_end_pos4_ndc = mul(float4(refl_ray_end_pos_vs, 1), proj_mtx);
  float3 const refl_ray_end_pos_ndc = refl_ray_end_pos4_ndc.xyz / refl_ray_end_pos4_ndc.w;

  float3 const refl_dir_ndc = normalize(refl_ray_end_pos_ndc - frag_pos_ndc);

  frag_pos_ts = float3(uv, frag_pos_ndc.z);
  refl_dir_ts = float3(NdcToUv(refl_dir_ndc.xy), refl_dir_ndc.z);

  max_trace_dist = refl_dir_ts.x >= 0 ? (1 - frag_pos_ts.x) / refl_dir_ts.x : -frag_pos_ts.x / refl_dir_ts.x;
  max_trace_dist = min(max_trace_dist,
    refl_dir_ts.y < 0 ? -frag_pos_ts.y / refl_dir_ts.y : (1 - frag_pos_ts.y) / refl_dir_ts.y);
  max_trace_dist = min(max_trace_dist,
    refl_dir_ts.z < 0 ? -frag_pos_ts.z / refl_dir_ts.z : (1 - frag_pos_ts.z) / refl_dir_ts.z);
}


float FindIntersection_Linear(float3 const frag_pos_ts, float3 const refl_dir_ts, float const max_trace_dist,
                              Texture2D<float> const depth_tex, SamplerState const point_clamp_samp,
                              out float3 intersection) {
  float2 depth_tex_size;
  depth_tex.GetDimensions(depth_tex_size.x, depth_tex_size.y);

  float3 const refl_ray_end_pos_ts = frag_pos_ts + refl_dir_ts * max_trace_dist;

  float3 dp = refl_ray_end_pos_ts - frag_pos_ts;
  int2 const frag_pos_ss = int2(frag_pos_ts.xy * depth_tex_size);
  int2 const refl_ray_end_pos_ss = int2(refl_ray_end_pos_ts.xy * depth_tex_size);
  int2 const dp2 = refl_ray_end_pos_ss - frag_pos_ss;
  int const max_dist = max(abs(dp2.x), abs(dp2.y));
  dp /= max_dist;

  float3 refl_ray_pos_ts = frag_pos_ts + dp;
  float3 const refl_ray_dir_ts = dp;
  float3 const refl_ray_start_pos_ts = refl_ray_pos_ts;

  int hit_index = -1;

  for (int i = 0; i <= max_dist && i < g_params.max_march_steps; i += 4) {
    float3 const refl_ray_pos_ts0 = refl_ray_pos_ts + refl_ray_dir_ts * 0;
    float3 const refl_ray_pos_ts1 = refl_ray_pos_ts + refl_ray_dir_ts * 1;
    float3 const refl_ray_pos_ts2 = refl_ray_pos_ts + refl_ray_dir_ts * 2;
    float3 const refl_ray_pos_ts3 = refl_ray_pos_ts + refl_ray_dir_ts * 3;

    float const depth3 = depth_tex.Sample(point_clamp_samp, refl_ray_pos_ts3.xy).r;
    float const depth2 = depth_tex.Sample(point_clamp_samp, refl_ray_pos_ts2.xy).r;
    float const depth1 = depth_tex.Sample(point_clamp_samp, refl_ray_pos_ts1.xy).r;
    float const depth0 = depth_tex.Sample(point_clamp_samp, refl_ray_pos_ts0.xy).r;

    {
      float const thickness = refl_ray_pos_ts3.z - depth3;
      hit_index = (thickness >= 0 && thickness < g_params.depth_tolerance_ndc) ? i + 3 : hit_index;
    }

    {
      float const thickness = refl_ray_pos_ts2.z - depth2;
      hit_index = (thickness >= 0 && thickness < g_params.depth_tolerance_ndc) ? i + 2 : hit_index;
    }

    {
      float const thickness = refl_ray_pos_ts1.z - depth1;
      hit_index = (thickness >= 0 && thickness < g_params.depth_tolerance_ndc) ? i + 1 : hit_index;
    }

    {
      float const thickness = refl_ray_pos_ts0.z - depth0;
      hit_index = (thickness >= 0 && thickness < g_params.depth_tolerance_ndc) ? i + 0 : hit_index;
    }

    if (hit_index != -1) {
      break;
    }

    refl_ray_pos_ts = refl_ray_pos_ts3 + refl_ray_dir_ts;
  }

  bool intersected = hit_index >= 0;
  intersection = refl_ray_start_pos_ts + refl_ray_dir_ts * hit_index;

  float const intensity = intersected ? 1 : 0;
  return intensity;
}


float4 ComputeReflectedColor(float const intensity, float3 const intersection, Texture2D const lit_scene_tex,
                             SamplerState const point_clamp_samp) {
  float3 const refl_color = lit_scene_tex.Sample(point_clamp_samp, intersection.xy).rgb;
  return float4(refl_color, 1);
}


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

  float3 const frag_norm_vs = mul(float4(frag_norm_ws, 0), per_view_cb.viewMtx).xyz;

  float4 reflection_color = float4(0, 0, 0, 1);

  if (roughness <= g_params.max_roughness) {
    float3 frag_pos_ts;
    float3 refl_dir_ts;
    float max_trace_dist;
    ComputePosAndReflection(ps_in.uv, frag_norm_vs, per_view_cb.projMtx, per_view_cb.invProjMtx, depth_tex,
      point_clamp_samp, frag_pos_ts, refl_dir_ts, max_trace_dist);

    float3 intersection;
    float const intensity = FindIntersection_Linear(frag_pos_ts, refl_dir_ts, max_trace_dist, depth_tex,
      point_clamp_samp, intersection);

    reflection_color = ComputeReflectedColor(intensity, intersection, lit_scene_tex, point_clamp_samp);
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
