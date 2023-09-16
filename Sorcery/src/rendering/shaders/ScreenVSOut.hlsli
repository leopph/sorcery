#ifndef SCREEN_VS_OUT_HLSLI
#define SCREEN_VS_OUT_HLSLI

struct ScreenVsOut {
  float4 positionCS : SV_POSITION;
  float2 uv : TEXCOORD;
};

#endif
