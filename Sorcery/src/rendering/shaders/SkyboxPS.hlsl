#include "SkyboxVSOut.hlsli"

struct DrawParams {
  uint cubemap_idx;
  uint samp_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

float4 main(const SkyboxVSOut vs_out) : SV_Target {
  const TextureCube cubemap = ResourceDescriptorHeap[g_draw_params.cubemap_idx];
  const SamplerState samp = SamplerDescriptorHeap[g_draw_params.samp_idx];
  return float4(cubemap.Sample(samp, vs_out.uv).rgb, 1);
}
