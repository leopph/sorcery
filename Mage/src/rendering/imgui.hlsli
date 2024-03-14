#ifndef IMGUI_HLSLI
#define IMGUI_HLSLI

#include "imgui_shader_interop.h"

ConstantBuffer<ImGuiDrawParams> g_draw_params : register(b0, space0);


struct VertexOut {
  float4 pos_cs : SV_Position;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
};


VertexOut VsMain(const uint vertex_id : SV_VertexID) {
  const StructuredBuffer<VertexData> vertex_data_buf = ResourceDescriptorHeap[g_draw_params.vb_idx];
  const VertexData vertex_data = vertex_data_buf[vertex_id];

  VertexOut vertex_out;
  vertex_out.pos_cs = mul(float4(vertex_data.pos_os, 0, 1), g_draw_params.proj_mtx);
  vertex_out.color = vertex_data.col;
  vertex_out.uv = vertex_data.uv;
  return vertex_out;
}


float4 PsMain(const VertexOut vertex_out) : SV_Target {
  const SamplerState samp = SamplerDescriptorHeap[g_draw_params.samp_idx];
  const Texture2D tex = ResourceDescriptorHeap[g_draw_params.tex_idx];
  return vertex_out.color * tex.Sample(samp, vertex_out.uv);
}

#endif
