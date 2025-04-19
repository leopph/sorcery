#ifndef FULLSCREEN_TRI_HLSLI
#define FULLSCREEN_TRI_HLSLI

#include "shader_interop.h"
#include "utility.hlsli"


struct PsIn {
  float4 pos_cs : SV_Position;
  float2 uv : TEXCOORD;
};


PsIn VsMain(uint const vertex_id : SV_VertexID) {
  PsIn ret;
  ret.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
  ret.pos_cs = float4(UvToNdc(ret.uv), DEPTH_CLEAR_VALUE, 1);
  return ret;
}

#endif
