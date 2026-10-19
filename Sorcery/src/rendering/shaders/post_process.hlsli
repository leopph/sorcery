#ifndef POST_PROCESS_HLSLI
#define POST_PROCESS_HLSLI

#include "common.hlsli"
#include "fullscreen_tri.hlsli"
#include "shader_interop.h"
#include "tonemapping.hlsli"


DECLARE_PARAMS(PostProcessDrawParams);


// Interleaved gradient noise (Jimenez 2014)
float InterleavedGradientNoise(float2 const pixel) {
  return frac(52.9829189 * frac(dot(pixel, float2(0.06711056, 0.00583715))));
}


float4 PsMain(PsIn const ps_in) : SV_TARGET {
  Texture2D const in_tex = ResourceDescriptorHeap[g_params.in_tex_idx];
  SamplerState const bi_clamp_samp = SamplerDescriptorHeap[g_params.bi_clamp_samp_idx];
  float3 pixel_color = in_tex.Sample(bi_clamp_samp, ps_in.uv).rgb;

  // Tone mapping
  pixel_color = TonemapAgx(pixel_color);

  // Gamma correction
  pixel_color = pow(pixel_color, g_params.inv_gamma);

  // Dither
  float const noise = InterleavedGradientNoise(ps_in.pos_cs.xy) + InterleavedGradientNoise(ps_in.pos_cs.xy + 0.5) - 1.0;
  pixel_color += noise / 255.0;

  return float4(pixel_color, 1);
}

#endif
