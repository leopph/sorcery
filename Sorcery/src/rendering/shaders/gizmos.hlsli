#ifndef GIZMOS_HLSLI
#define GIZMOS_HLSLI

#include "common.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(GizmoDrawParams);
DECLARE_DRAW_CALL_PARAMS(g_draw_call_params);


struct PsIn {
  float4 position : SV_Position;
  uint color_idx : COLORIDX;
};


PsIn VsMainLine(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) {
  vertex_id += g_draw_call_params.base_vertex;
  instance_id += g_draw_call_params.base_instance;

  const StructuredBuffer<ShaderLineGizmoVertexData> vertices = ResourceDescriptorHeap[g_params.vertex_buf_idx];
  const ShaderLineGizmoVertexData data = vertices[instance_id];

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

  PsIn ret;
  ret.color_idx = data.colorIdx;
  ret.position = mul(float4(vertex_id == 0 ? data.from : data.to, 1), per_view_cb.viewProjMtx);
  return ret;
}


float4 PsMain(const PsIn input) : SV_Target {
  const StructuredBuffer<float4> colors = ResourceDescriptorHeap[g_params.color_buf_idx];
  return float4(colors[input.color_idx]);
}
#endif
