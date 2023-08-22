#include "ShaderInterop.h"
#include "Samplers.hlsli"

TEXTURE2D(gOpacityMask, float, RES_SLOT_OPACITY_MASK);

void main(const float2 uv : TEXCOORD) {
  if (material.blendMode == BLEND_MODE_ALPHA_CLIP && material.sampleOpacityMap && gOpacityMask.Sample(gSamplerAf16, uv) < material.alphaThreshold) {
    discard;
  }
}