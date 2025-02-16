#ifndef IMGUI_HLSLI
#define IMGUI_HLSLI

#include "imgui_shader_interop.h"


struct SorceryDrawCallParams {
  int base_vertex;
};


ConstantBuffer<ImGuiDrawParams> g_draw_params : register(b0, space0);
ConstantBuffer<SorceryDrawCallParams> g_draw_call_params : register(b1, space0);


struct PsIn {
  float4 pos_cs : SV_Position;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
};


PsIn VsMain(uint vertex_id : SV_VertexID) {
  vertex_id = (uint) ((int) vertex_id + g_draw_call_params.base_vertex);

  const StructuredBuffer<VertexData> vertex_data_buf = ResourceDescriptorHeap[g_draw_params.vb_idx];
  const VertexData vertex_data = vertex_data_buf[vertex_id];

  PsIn vertex_out;
  vertex_out.pos_cs = mul(float4(vertex_data.pos_os, 0, 1), g_draw_params.proj_mtx);
  vertex_out.color = float4(vertex_data.col & 0xFF, vertex_data.col >> 8 & 0xFF, vertex_data.col >> 16 & 0xFF,
                       vertex_data.col >> 24 & 0xFF) / 255;
  vertex_out.uv = vertex_data.uv;
  return vertex_out;
}


float4 PsMain(const PsIn vertex_out) : SV_Target {
  const SamplerState samp = SamplerDescriptorHeap[g_draw_params.samp_idx];
  const Texture2D tex = ResourceDescriptorHeap[g_draw_params.tex_idx];
  return vertex_out.color * tex.Sample(samp, vertex_out.uv);
}

#endif
