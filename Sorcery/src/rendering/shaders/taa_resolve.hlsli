#ifndef TAA_RESOLVE_HLSLI
#define TAA_RESOLVE_HLSLI

#include "common.hlsli"
#include "shader_interop.h"

DECLARE_PARAMS(TaaResolveDrawParams);


[numthreads(TAA_RESOLVE_CS_THREADS_X, TAA_RESOLVE_CS_THREADS_Y, 1)]
void main(uint3 const id : SV_DispatchThreadID) {
  Texture2D const in_tex = ResourceDescriptorHeap[g_params.in_tex_idx];
  RWTexture2D<float4> const accum_tex = ResourceDescriptorHeap[g_params.accum_tex_idx];

  uint2 in_tex_size;
  in_tex.GetDimensions(in_tex_size.x, in_tex_size.y);

  if (id.x >= in_tex_size.x || id.y >= in_tex_size.y) {
    return; // Out of bounds
  }

  uint2 accum_tex_size;
  accum_tex.GetDimensions(accum_tex_size.x, accum_tex_size.y);

  if (accum_tex_size.x != in_tex_size.x || accum_tex_size.y != in_tex_size.y) {
    return; // Accumulation texture size must match input texture size
  }

  float3 const in_color = in_tex[id.xy].rgb;
  float3 const accum_color = accum_tex[id.xy].rgb;

  accum_tex[id.xy] = float4(in_color * g_params.blend_factor + accum_color * (1.0 - g_params.blend_factor), 1.0);
}

#endif
