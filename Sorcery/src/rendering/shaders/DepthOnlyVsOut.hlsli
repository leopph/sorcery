#ifndef DEPTH_ONLY_VS_OUT_HLSLI
#define DEPTH_ONLY_VS_OUT_HLSLI

struct DepthOnlyVsOut {
  float4 positionCS : SV_POSITION;
  float2 uv : TEXCOORD;
};

#endif