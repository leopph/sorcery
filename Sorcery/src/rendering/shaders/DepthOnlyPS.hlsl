#include "ShaderInterop.h"
#include "DepthOnlyVsOut.hlsli"
#include "Samplers.hlsli"

TEXTURE2D(gOpacityMask, float, RES_SLOT_OPACITY_MASK);

void main(const DepthOnlyVsOut vsOut) {
  if (material.blendMode == BLEND_MODE_ALPHA_CLIP && material.sampleOpacityMap && gOpacityMask.Sample(gSamplerAf16Wrap, vsOut.uv) < material.alphaThreshold) {
    discard;
  }
}