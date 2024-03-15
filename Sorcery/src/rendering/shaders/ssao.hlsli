#ifndef SSAO_HLSLI
#define SSAO_HLSLI

#include "common.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"


DECLARE_PARAMS1(SsaoDrawParams, g_params);
DECLARE_PARAMS1(SsaoBlurDrawParams, g_blur_params);


struct VertexOut {
  float4 pos_cs : SV_POSITION;
  float2 uv : TEXCOORD;
};


VertexOut VsMain(const uint vertex_id : SV_VertexID) {
  VertexOut ret;
  ret.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
  ret.pos_cs = float4(UvToNdc(ret.uv), 0, 1);
  return ret;
}


float3 CalculatePositionVsAtUv(const Texture2D<float> depth_tex, const SamplerState point_clamp_samp,
                               const ConstantBuffer<ShaderPerViewConstants> per_view_cb, const float2 uv) {
  const float depth = depth_tex.Sample(point_clamp_samp, uv).r;
  const float4 pos_vs = mul(float4(UvToNdc(uv), depth, 1), per_view_cb.invProjMtx);
  return pos_vs.xyz / pos_vs.w;
}


// MAIN PASS ######################################################


float PsMain(const VertexOut vs_out) : SV_Target {
  float2 noise_tex_size;

  const Texture2D<float3> noise_tex = ResourceDescriptorHeap[g_params.noise_tex_idx];
  noise_tex.GetDimensions(noise_tex_size.x, noise_tex_size.y);

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_params.per_frame_cb_idx];
  const float2 noise_scale = per_frame_cb.screenSize / noise_tex_size;

  const SamplerState point_wrap_samp = SamplerDescriptorHeap[g_params.point_wrap_samp_idx];
  const float3 noise = normalize(noise_tex.Sample(point_wrap_samp, vs_out.uv * noise_scale).xyz);

  const Texture2D<float> depth_tex = ResourceDescriptorHeap[g_params.depth_tex_idx];
  const SamplerState point_clamp_samp = SamplerDescriptorHeap[g_params.point_clamp_samp_idx];
  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
  const float3 hemisphere_origin_vs = CalculatePositionVsAtUv(depth_tex, point_clamp_samp, per_view_cb, vs_out.uv);

  const Texture2D<float3> normal_tex = ResourceDescriptorHeap[g_params.normal_tex_idx];
  const float3 normal_vs = normalize(mul(float4(normal_tex.Sample(point_clamp_samp, vs_out.uv).xyz, 0),
    per_view_cb.viewMtx).xyz);
  const float3 tangent_vs = normalize(noise - normal_vs * dot(noise, normal_vs));
  const float3 bitangent_vs = cross(normal_vs, tangent_vs);
  const float3x3 tbn_mtx_vs = float3x3(tangent_vs, bitangent_vs, normal_vs);

  float occlusion = 0.0;
  uint sample_count, stride;

  const StructuredBuffer<float4> samples = ResourceDescriptorHeap[g_params.samp_buf_idx];
  samples.GetDimensions(sample_count, stride);

  for (uint i = 0; i < sample_count; i++) {
    const float3 samplePos = mul(samples[i].xyz, tbn_mtx_vs) * g_params.radius + hemisphere_origin_vs;

    float4 sampleOffset = mul(float4(samplePos, 1), per_view_cb.projMtx);
    sampleOffset /= sampleOffset.w;

    const float sampleDepth = CalculatePositionVsAtUv(depth_tex, point_clamp_samp, per_view_cb,
      NdcToUv(sampleOffset.xy)).z;

    const float rangeCheck = smoothstep(0.0, 1.0, g_params.radius / abs(hemisphere_origin_vs.z - sampleDepth));
    occlusion += step(sampleDepth, samplePos.z - g_params.bias) * rangeCheck;
  }

  return pow(1 - occlusion / sample_count, g_params.power);
}


// BLUR PASS ##########################################


float PsMainBlur(const float4 pixel_coord : SV_POSITION, const float2 uv : TEXCOORD) : SV_TARGET {
  uint2 texture_size;

  const Texture2D<float> in_tex = ResourceDescriptorHeap[g_blur_params.in_tex_idx];
  in_tex.GetDimensions(texture_size.x, texture_size.y);
  const float2 texel_size = 1.0 / float2(texture_size);

  float result = 0;

  const int lo = -SSAO_NOISE_TEX_DIM / 2;
  const int hi = SSAO_NOISE_TEX_DIM / 2;

  const SamplerState point_clamp_samp = SamplerDescriptorHeap[g_blur_params.point_clamp_samp_idx];

  for (int x = lo; x < hi; x++) {
    for (int y = lo; y < hi; y++) {
      const float2 uv_offset = float2(x, y) * texel_size;
      result += in_tex.Sample(point_clamp_samp, uv + uv_offset).r;
    }
  }

  return result / (SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM);
}
#endif
