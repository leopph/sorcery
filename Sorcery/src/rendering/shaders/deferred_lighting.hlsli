#ifndef DEFERRED_LIGHTING_HLSLI
#define DEFERRED_LIGHTING_HLSLI

#include "common.hlsli"
#include "gbuffer_utils.hlsli"
#include "lighting.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"


DECLARE_PARAMS(DeferredLightingDrawParams);


struct PsIn {
  float4 pos_cs : SV_Position;
  float2 uv : TEXCOORD;
};


PsIn VsMain(uint const vertex_id : SV_VertexID) {
  // A triangle covering the entire screen.

  PsIn ps_in;
  ps_in.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
  ps_in.pos_cs = float4(UvToNdc(ps_in.uv), 0, 1);
  return ps_in;
}


float4 PsMain(PsIn const ps_in) : SV_Target {
  SamplerState const point_clamp_samp = SamplerDescriptorHeap[g_params.point_clamp_samp_idx];
  Texture2D const gbuffer0 = ResourceDescriptorHeap[g_params.gbuffer0_idx];
  Texture2D<float2> const gbuffer1 = ResourceDescriptorHeap[g_params.gbuffer1_idx];
  Texture2D<float2> const gbuffer2 = ResourceDescriptorHeap[g_params.gbuffer2_idx];

  float3 albedo;
  float3 norm_ws;
  float roughness;
  float metallic;
  float ao;

  UnpackGBuffer0(gbuffer0.Sample(point_clamp_samp, ps_in.uv.xy), albedo, ao);
  UnpackGBuffer1(gbuffer1.Sample(point_clamp_samp, ps_in.uv.xy), norm_ws);
  UnpackGBuffer2(gbuffer2.Sample(point_clamp_samp, ps_in.uv.xy), roughness, metallic);

  Texture2D<float> const depth_tex = ResourceDescriptorHeap[g_params.depth_tex_idx];
  float const depth = depth_tex.Sample(point_clamp_samp, ps_in.uv.xy, 0, 0);
  
  Texture2D<float> const ssao_tex = ResourceDescriptorHeap[g_params.ssao_tex_idx];
  ao *= ssao_tex.Sample(point_clamp_samp, ps_in.uv).r;

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

  float4 const pos_ndc = float4(UvToNdc(ps_in.uv.xy), depth, 1);
  float4 pos4_ws = mul(pos_ndc, per_view_cb.invViewProjMtx);
  float3 const pos_ws = pos4_ws.xyz / pos4_ws.w;
  float3 const pos_vs = mul(float4(pos_ws, 1), per_view_cb.viewMtx).xyz;
  float3 const dir_to_cam_ws = normalize(per_view_cb.viewPos - pos_ws);

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_params.per_frame_cb_idx];
  float3 out_color = per_frame_cb.ambientLightColor * albedo * ao;

  StructuredBuffer<ShaderLight> const lights = ResourceDescriptorHeap[g_params.light_buf_idx];

  Texture2DArray<float> const dir_light_shadow_map_arr = ResourceDescriptorHeap[g_params.dir_shadow_arr_idx];
  Texture2D<float> const punc_light_shadow_atlas = ResourceDescriptorHeap[g_params.punc_shadow_atlas_idx];
  SamplerComparisonState const shadow_samp = SamplerDescriptorHeap[g_params.shadow_samp_idx];

  for (uint i = 0; i < g_params.light_count; i++) {
    if (lights[i].type == 0) {
      out_color += CalculateDirLight(lights[i], pos_ws, norm_ws, dir_to_cam_ws, pos_vs.z, albedo,
        metallic, roughness, dir_light_shadow_map_arr, shadow_samp, per_frame_cb.shadowFilteringMode,
        per_view_cb.shadowCascadeSplitDistances, per_frame_cb.shadowCascadeCount, per_frame_cb.visualizeShadowCascades);
    } else if (lights[i].type == 1) {
      out_color += CalculateSpotLight(lights[i], pos_ws, norm_ws, dir_to_cam_ws, albedo, metallic,
        roughness, punc_light_shadow_atlas, shadow_samp, per_frame_cb.shadowFilteringMode);
    } else if (lights[i].type == 2) {
      out_color += CalculatePointLight(lights[i], pos_ws, norm_ws, dir_to_cam_ws, albedo, metallic,
        roughness, punc_light_shadow_atlas, shadow_samp, per_frame_cb.shadowFilteringMode);
    }
  }

  return float4(out_color, 1);
}


#endif
