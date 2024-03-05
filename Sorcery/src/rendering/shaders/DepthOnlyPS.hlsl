#include "shader_interop.h"
#include "DepthOnlyVsOut.hlsli"

struct DrawParams {
  uint mtl_idx;
  uint samp_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

void main(const DepthOnlyVsOut vs_out) {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_draw_params.mtl_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];
    const SamplerState samp = SamplerDescriptorHeap[g_draw_params.samp_idx];

    if (opacity_map.Sample(samp, vs_out.uv) < mtl.alphaThreshold) {
      discard;
    }
  }
}
