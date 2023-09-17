#include "ShaderInterop.h"
#include "Samplers.hlsli"

TEXTURE2D(gSsaoInput, float, RES_SLOT_SSAO_BLUR_INPUT);

float main(const float4 pixelCoord : SV_POSITION, const float2 uv : TEXCOORD) : SV_TARGET {
  uint2 textureSize;
  gSsaoInput.GetDimensions(textureSize.x, textureSize.y);
  const float2 texelSize = 1.0 / float2(textureSize);

  float result = 0;

  const int lo = -SSAO_NOISE_TEX_DIM / 2;
  const int hi = SSAO_NOISE_TEX_DIM / 2;

  for (int x = lo; x < hi; x++) {
    for (int y = lo; y < hi; y++) {
      const float2 uvOffset = float2(x, y) * texelSize;
      result += gSsaoInput.Sample(gSamplerPointClamp, uv + uvOffset).r;
    }
  }

  return result / (SSAO_NOISE_TEX_DIM * SSAO_NOISE_TEX_DIM);
}