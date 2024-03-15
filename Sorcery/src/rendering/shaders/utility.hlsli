#ifndef UTIL_HLSLI
#define UTIL_HLSLI

float2 UvToNdc(const float2 uv) {
  return uv * float2(2, -2) + float2(-1, 1);
}

float2 NdcToUv(const float2 ndc) {
  return ndc * float2(0.5, -0.5) + float2(0.5, 0.5);
}

#endif