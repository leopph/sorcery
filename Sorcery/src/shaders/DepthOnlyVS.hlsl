#include "ShaderInterop.h"

struct DepthOnlyVSOut {
  float4 posCS : SV_Position;
  float2 uv : TEXCOORD;
};

DepthOnlyVSOut main(const float3 pos : POSITION, const float2 uv : TEXCOORD) {
  DepthOnlyVSOut ret;
  ret.posCS = mul(mul(float4(pos, 1), gPerDrawConstants.modelMtx), gDepthOnlyViewProjMtx);
  ret.uv = uv;
  return ret;
}