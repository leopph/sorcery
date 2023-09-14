#include "ShaderInterop.h"
#include "DepthOnlyVsOut.hlsli"


struct VsIn {
  float3 positionOS : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
};


DepthOnlyVsOut main(const VsIn vsIn) {
  const float4 positionWS = mul(float4(vsIn.positionOS, 1), gPerDrawConstants.modelMtx);
  const float4 positionCS = mul(positionWS, gPerViewConstants.viewProjMtx);

  DepthOnlyVsOut ret;
  ret.positionCS = positionCS;
  ret.uv = vsIn.uv;
  return ret;
}
