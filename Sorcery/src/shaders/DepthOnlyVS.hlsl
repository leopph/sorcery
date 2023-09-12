#include "ShaderInterop.h"

struct VsIn {
  float3 posOS : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD;
};

struct VsOut {
  float4 posCS : SV_POSITION;
  float2 uv : TEXCOORD;
};

VsOut main(const VsIn vsIn) {
  VsOut ret;
  ret.posCS = mul(mul(float4(vsIn.posOS, 1), gPerDrawConstants.modelMtx), gPerViewConstants.viewProjMtx);
  ret.uv = vsIn.uv;
  return ret;
}
