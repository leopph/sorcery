#ifndef TAA_RESOLVE_HLSLI
#define TAA_RESOLVE_HLSLI

#include "common.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"

DECLARE_PARAMS(TaaResolveDrawParams);


[numthreads(TAA_RESOLVE_CS_THREADS_X, TAA_RESOLVE_CS_THREADS_Y, 1)]
void main(uint3 const id : SV_DispatchThreadID) {
  Texture2D const in_tex = ResourceDescriptorHeap[g_params.in_tex_idx];
  RWTexture2D<float4> const accum_tex = ResourceDescriptorHeap[g_params.accum_tex_idx];
  Texture2D const in_depth_tex = ResourceDescriptorHeap[g_params.in_depth_tex_idx];
  ConstantBuffer<ShaderPerViewConstants> const per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

  uint2 in_tex_size;
  in_tex.GetDimensions(in_tex_size.x, in_tex_size.y);

  if (id.x >= in_tex_size.x || id.y >= in_tex_size.y) {
    return; // Out of bounds
  }

  uint2 in_depth_tex_size;
  in_depth_tex.GetDimensions(in_depth_tex_size.x, in_depth_tex_size.y);

  if (in_depth_tex_size.x != in_tex_size.x || in_depth_tex_size.y != in_tex_size.y) {
    return; // Depth texture size must match input texture size
  }

  uint2 accum_tex_size;
  accum_tex.GetDimensions(accum_tex_size.x, accum_tex_size.y);

  if (accum_tex_size.x != in_tex_size.x || accum_tex_size.y != in_tex_size.y) {
    return; // Accumulation texture size must match input texture size
  }

  float const depth = in_depth_tex[id.xy].r;
  float2 const uv = float2(id.x, id.y) / float2(in_tex_size);

  // Reproject
  float4 pos_ws = mul(float4(UvToNdc(uv), depth, 1.0), per_view_cb.invViewProjMtx);
  float4 const reprojected_ndc4 = mul(pos_ws, per_view_cb.prev_view_proj_mtx);
  float2 const reprojected_uv = NdcToUv(reprojected_ndc4.xy / reprojected_ndc4.w);
  uint2 const reprojected_id = uint2(reprojected_uv * float2(in_tex_size));

  // Color clamping

  float3 min_color = 9999;
  float3 max_color = -9999;

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      int2 const neighbor_id = (int2)id.xy + int2(x, y);

      if (neighbor_id.x < in_tex_size.x &&
          neighbor_id.x >= 0 &&
          neighbor_id.y < in_tex_size.y &&
          neighbor_id.y >= 0) {
        float3 const neighbor_color = in_tex[neighbor_id].rgb;
        min_color = min(min_color, neighbor_color);
        max_color = max(max_color, neighbor_color);
      }
    }
  }

  float3 const accum_color = accum_tex[reprojected_id.xy].rgb;
  float3 const clamped_accum_color = clamp(accum_color, min_color, max_color);
  float3 const in_color = in_tex[id.xy].rgb;

  accum_tex[id.xy] = float4(in_color * g_params.blend_factor +
                            clamped_accum_color * (1.0 - g_params.blend_factor), 1.0);
}

#endif
