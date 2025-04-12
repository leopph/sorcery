#ifndef SSAO_HLSLI
#define SSAO_HLSLI

#include "common.hlsli"
#include "gbuffer_utils.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"


DECLARE_PARAMS1(SsaoDrawParams, g_params);
DECLARE_PARAMS1(SsaoBlurDrawParams, g_blur_params);


struct PsIn {
  float4 pos_cs : SV_POSITION;
  float2 uv : TEXCOORD;
};


PsIn VsMain(uint const vertex_id : SV_VertexID) {
  PsIn ret;
  ret.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
  ret.pos_cs = float4(UvToNdc(ret.uv), 0, 1);
  return ret;
}


float3 CalculatePositionVsAtUv(Texture2D<float> const depth_tex, SamplerState const point_clamp_samp,
                               const ConstantBuffer<ShaderPerViewConstants> per_view_cb, float2 const uv) {
  float const depth = depth_tex.Sample(point_clamp_samp, uv).r;
  float4 const pos_vs = mul(float4(UvToNdc(uv), depth, 1), per_view_cb.invProjMtx);
  return pos_vs.xyz / pos_vs.w;
}


// MAIN PASS ######################################################


float PsMain(PsIn const vs_out) : SV_Target {
  float2 noise_tex_size;

  Texture2D<float3> const noise_tex = ResourceDescriptorHeap[g_params.noise_tex_idx];
  noise_tex.GetDimensions(noise_tex_size.x, noise_tex_size.y);

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_params.per_frame_cb_idx];
  float2 const noise_scale = per_frame_cb.screenSize / noise_tex_size;

  SamplerState const point_wrap_samp = SamplerDescriptorHeap[g_params.point_wrap_samp_idx];
  float3 const noise = normalize(noise_tex.Sample(point_wrap_samp, vs_out.uv * noise_scale).xyz);

  Texture2D<float> const depth_tex = ResourceDescriptorHeap[g_params.depth_tex_idx];
  SamplerState const point_clamp_samp = SamplerDescriptorHeap[g_params.point_clamp_samp_idx];
  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
  float3 const hemisphere_origin_vs = CalculatePositionVsAtUv(depth_tex, point_clamp_samp, per_view_cb, vs_out.uv);

  Texture2D<float2> const gbuffer1_tex = ResourceDescriptorHeap[g_params.gbuffer1_tex_idx];
  float3 normal_ws;
  UnpackGBuffer1(gbuffer1_tex.Sample(point_clamp_samp, vs_out.uv), normal_ws);

  float3 const normal_vs = normalize(mul(float4(normal_ws, 0), per_view_cb.viewMtx).xyz);
  float3 const tangent_vs = normalize(noise - normal_vs * dot(noise, normal_vs));
  float3 const bitangent_vs = cross(normal_vs, tangent_vs);
  float3x3 const tbn_mtx_vs = float3x3(tangent_vs, bitangent_vs, normal_vs);

  StructuredBuffer<float4> const samples = ResourceDescriptorHeap[g_params.samp_buf_idx];

  float occlusion = 0.0;

  for (uint i = 0; i < g_params.sample_count; i++) {
    float3 const samplePos = mul(samples[i].xyz, tbn_mtx_vs) * g_params.radius + hemisphere_origin_vs;

    float4 sampleOffset = mul(float4(samplePos, 1), per_view_cb.projMtx);
    sampleOffset /= sampleOffset.w;

    float const sampleDepth = CalculatePositionVsAtUv(depth_tex, point_clamp_samp, per_view_cb,
      NdcToUv(sampleOffset.xy)).z;

    float const rangeCheck = smoothstep(0.0, 1.0, g_params.radius / abs(hemisphere_origin_vs.z - sampleDepth));
    occlusion += step(sampleDepth, samplePos.z - g_params.bias) * rangeCheck;
  }

  return pow(1 - occlusion / g_params.sample_count, g_params.power);
}


// BLUR PASS ##########################################


float PsMainBlur(float4 const pixel_coord : SV_POSITION, float2 const uv : TEXCOORD) : SV_TARGET {
  uint2 texture_size;

  Texture2D<float> const in_tex = ResourceDescriptorHeap[g_blur_params.in_tex_idx];
  in_tex.GetDimensions(texture_size.x, texture_size.y);
  float2 const texel_size = 1.0 / float2(texture_size);

  float result = 0;

  int const lo = -SSAO_NOISE_TEX_DIM / 2;
  int const hi = SSAO_NOISE_TEX_DIM / 2;

  SamplerState const point_clamp_samp = SamplerDescriptorHeap[g_blur_params.point_clamp_samp_idx];

  for (int x = lo; x < hi; x++) {
    for (int y = lo; y < hi; y++) {
      float2 const uv_offset = float2(x, y) * texel_size;
      result += in_tex.Sample(point_clamp_samp, uv + uv_offset).r;
    }
  }

  return result / (SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM);
}
#endif
