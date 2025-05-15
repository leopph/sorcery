#ifndef UTIL_HLSLI
#define UTIL_HLSLI

#include "shader_interop.h"


float2 UvToNdc(float2 const uv) {
  return uv * float2(2, -2) + float2(-1, 1);
}


float2 NdcToUv(float2 const ndc) {
  return ndc * float2(0.5, -0.5) + float2(0.5, 0.5);
}


// Convert from perspective depth buffer value to linear view space depth
float LinearizeDepth(float depth, float const near_clip, float const far_clip) {
#ifdef REVERSE_Z
  depth = 1.0 - depth;
#endif
  return -near_clip * far_clip / (depth * (far_clip - near_clip) - far_clip);
}

#endif
