#include "ShaderInterop.h"
#include "Samplers.hlsli"

TEXTURE2D(gSsaoInput, float, RES_SLOT_SSAO_BLUR_INPUT);

float main(const float4 pixelCoord : SV_POSITION, const float2 uv : TEXCOORD) : SV_TARGET {
  uint2 textureSize;
  gSsaoInput.GetDimensions(textureSize.x, textureSize.y);
  const float2 texelSize = 1.0 / float2(textureSize);

  float result = 0;

  for (int x = -2; x < 2; x++) {
    for (int y = -2; y < 2; y++) {
      const float2 uvOffset = float2(x, y) * texelSize;
      result += gSsaoInput.Sample(gSamplerPoint, uv + uvOffset).r;
    }
  }

  return result / (4 * 4);
}