#include "ShaderInterop.h"
#include "DepthNormalVsOut.hlsli"
#include "Samplers.hlsli"

TEXTURE2D(gOpacityMask, float, RES_SLOT_OPACITY_MASK);
TEXTURE2D(gNormalMap, float3, RES_SLOT_NORMAL_MAP);

float3 main(const DepthNormalVsOut vsOut) : SV_TARGET {
  if (material.blendMode == BLEND_MODE_ALPHA_CLIP && material.sampleOpacityMap && gOpacityMask.Sample(gSamplerAf16Wrap, vsOut.uv) < material.alphaThreshold) {
    discard;
  }

  if (material.sampleNormal) {
    const float3 normalTS = gNormalMap.Sample(gSamplerAf16Wrap, vsOut.uv).rgb * 2.0 - 1.0;
    return normalize(mul(normalTS, vsOut.tbnMtxWS));
  }

  return normalize(vsOut.normalWS);
}