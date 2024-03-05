#include "ShaderInterop.h"

struct DrawParams {
  uint in_tex_idx;
  uint point_clamp_samp_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

float main(const float4 pixel_coord : SV_POSITION, const float2 uv : TEXCOORD) : SV_TARGET {
  uint2 texture_size;

  const Texture2D<float> in_tex = ResourceDescriptorHeap[g_draw_params.in_tex_idx];
  in_tex.GetDimensions(texture_size.x, texture_size.y);
  const float2 texel_size = 1.0 / float2(texture_size);

  float result = 0;

  const int lo = -SSAO_NOISE_TEX_DIM / 2;
  const int hi = SSAO_NOISE_TEX_DIM / 2;

  const SamplerState point_clamp_samp = SamplerDescriptorHeap[g_draw_params.point_clamp_samp_idx];

  for (int x = lo; x < hi; x++) {
    for (int y = lo; y < hi; y++) {
      const float2 uv_offset = float2(x, y) * texel_size;
      result += in_tex.Sample(point_clamp_samp, uv + uv_offset).r;
    }
  }

  return result / (SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM);
}
