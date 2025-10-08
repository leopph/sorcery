#ifndef POST_PROCESS_HLSLI
#define POST_PROCESS_HLSLI

#include "common.hlsli"
#include "fullscreen_tri.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(PostProcessDrawParams);


float3 TonemapReinhard(float3 const color) {
  return color / (color + 1.0);
}


float4 PsMain(PsIn const ps_in) : SV_TARGET {
  Texture2D const in_tex = ResourceDescriptorHeap[g_params.in_tex_idx];
  SamplerState const bi_clamp_samp = SamplerDescriptorHeap[g_params.bi_clamp_samp_idx];
  float3 pixel_color = in_tex.Sample(bi_clamp_samp, ps_in.uv).rgb;

  // Tone mapping
  pixel_color = TonemapReinhard(pixel_color);

  // Gamma correction
  pixel_color = pow(pixel_color, g_params.inv_gamma);

  return float4(pixel_color, 1);
}

#endif
