#include "shader_interop.h"
#include "DepthNormalVsOut.hlsli"

struct DrawParams {
  uint mtl_idx;
  uint samp_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

float3 main(const DepthNormalVsOut vs_out) : SV_Target {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_draw_params.mtl_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];
    const SamplerState samp = SamplerDescriptorHeap[g_draw_params.samp_idx];

    if (opacity_map.Sample(samp, vs_out.uv) < mtl.alphaThreshold) {
      discard;
    }
  }

  if (mtl.normal_map_idx != INVALID_RES_IDX) {
    const Texture2D<float3> normal_map = ResourceDescriptorHeap[mtl.normal_map_idx];
    const SamplerState samp = SamplerDescriptorHeap[g_draw_params.samp_idx];

    const float3 normal_ts = normal_map.Sample(samp, vs_out.uv).rgb * 2.0 - 1.0;
    return normalize(mul(normal_ts, vs_out.tbnMtxWS));
  }

  return normalize(vs_out.normalWS);
}
