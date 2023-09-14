#ifndef DEPTH_NORMAL_VS_OUT_HLSLI
#define DEPTH_NORMAL_VS_OUT_HLSLI

struct DepthNormalVsOut {
  float4 positionCS : SV_POSITION;
  float3 normalWS : NORMAL;
  float2 uv : TEXCOORD;
  float3x3 tbnMtxWS : TBNMTXWS;
};

#endif