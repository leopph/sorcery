#ifndef POST_PROCESS_HLSLI
#define POST_PROCESS_HLSLI

#include "common.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"


DECLARE_PARAMS(PostProcessDrawParams);


struct ScreenVsOut {
  float4 pos_cs : SV_POSITION;
  float2 uv : TEXCOORD;
};


ScreenVsOut VsMain(const uint vertex_id : SV_VertexID) {
  ScreenVsOut ret;
  ret.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
  ret.pos_cs = float4(UvToNdc(ret.uv), 0, 1);
  return ret;
}


float4 PsMain(const float4 pixel_coord : SV_POSITION) : SV_TARGET {
  const Texture2D in_tex = ResourceDescriptorHeap[g_params.in_tex_idx];
  float3 pixel_color = in_tex.Load(int3(pixel_coord.xy, 0)).rgb;

  // Tone mapping
  pixel_color = pixel_color / (pixel_color + 1.0);

  // Gamma correction
  pixel_color = pow(pixel_color, g_params.inv_gamma);

  return float4(pixel_color, 1);
}

#endif
