#ifndef IRRADIANCE_HLSLI
#define IRRADIANCE_HLSLI

#include "common.hlsli"
#include "shader_interop.h"
#include "utility.hlsli"

DECLARE_PARAMS(IrradianceDrawParams);


struct PsIn {
  float4 pos_cs : SV_Position;
  float2 uv : TEXCOORD;
  uint rt_idx : SV_RenderTargetArrayIndex;
};


PsIn VsMain(uint const vertex_id : SV_VertexID, uint const view_id : SV_ViewID) {
  PsIn ret;
  ret.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
  ret.pos_cs = float4(UvToNdc(ret.uv), DEPTH_CLEAR_VALUE, 1);
  ret.rt_idx = view_id + g_params.view_id_offset;
  return ret;
}


static float3 const colors[6] = {
  float3(1, 0, 0), // Red
  float3(0, 1, 0), // Green
  float3(0, 0, 1), // Blue
  float3(1, 1, 0), // Yellow
  float3(1, 0, 1), // Magenta
  float3(0, 1, 1) // Cyan
};


float4 PsMain(PsIn const ps_in) : SV_Target {
  if (ps_in.rt_idx < 6) {
    return float4(colors[ps_in.rt_idx], 1.0f);
  }

  return float4(1, 1, 1, 1);
  //Texture2D const environment_map = ResourceDescriptorHeap[g_params.environment_map_idx];
}

#endif
