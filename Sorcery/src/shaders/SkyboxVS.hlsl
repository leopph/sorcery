#include "ShaderInterop.h"
#include "SkyboxVSOut.hlsli"

SkyboxVSOut main(const float3 positionOS : POSITION)
{
  SkyboxVSOut ret;
  ret.positionCS = mul(float4(mul(positionOS, (float3x3)gPerViewConstants.viewMtx), 1), gPerViewConstants.projMtx);
  ret.uv = positionOS;

  if (gPerFrameConstants.isUsingReversedZ) {
    ret.positionCS.z = 0;
  } else {
    ret.positionCS = ret.positionCS.xyww;
  }

  return ret;
}