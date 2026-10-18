#ifndef MATERIAL_HELPERS_HLSLI
#define MATERIAL_HELPERS_HLSLI

#include "shader_interop.h"


float SelectChannel(float4 const v, uint const channel) {
  return dot(v, float4(channel == 0, channel == 1, channel == 2, channel == 3));
}


// Returns float4(base_color.rgb * base_color_map.rgb, opacity * base_color_map.a)
float4 SampleBaseColorOpacity(ShaderMaterial const mtl, SamplerState const samp, float2 const uv) {
  float4 base_color_opacity = float4(mtl.base_color, mtl.opacity);

  if (mtl.base_color_map_idx != INVALID_RES_IDX) {
    Texture2D const base_color_map = ResourceDescriptorHeap[mtl.base_color_map_idx];
    base_color_opacity *= base_color_map.Sample(samp, uv);
  }

  return base_color_opacity;
}


// base_opacity = opacity * base_color_map.a
float CalculateOpacity(ShaderMaterial const mtl, SamplerState const samp, float2 const uv, float const base_opacity) {
  float opacity = base_opacity;

  if (mtl.opacity_map_idx != INVALID_RES_IDX) {
    Texture2D const opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];
    opacity *= SelectChannel(opacity_map.Sample(samp, uv), mtl.opacity_channel);
  }

  return opacity;
}


bool IsClipped(ShaderMaterial const mtl, float const opacity) {
  return opacity < mtl.alpha_threshold;
}

#endif
