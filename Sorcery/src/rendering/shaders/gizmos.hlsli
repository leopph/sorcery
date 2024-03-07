#ifndef GIZMOS_HLSLI
#define GIZMOS_HLSLI

#include "common.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(GizmoDrawParams);


struct VertexOut {
  float4 position : SV_Position;
  uint color_idx : COLORIDX;
};


VertexOut VsMainLine(const uint vertex_id : SV_VertexID, const uint instance_id : SV_InstanceID) {
  const StructuredBuffer<ShaderLineGizmoVertexData> vertices = ResourceDescriptorHeap[g_params.vertex_buf_idx];
  const ShaderLineGizmoVertexData data = vertices[instance_id];

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

  VertexOut ret;
  ret.color_idx = data.colorIdx;
  ret.position = mul(float4(vertex_id == 0 ? data.from : data.to, 1), per_view_cb.viewProjMtx);
  return ret;
}


float4 PsMain(const VertexOut input) : SV_Target {
  const StructuredBuffer<float4> colors = ResourceDescriptorHeap[g_params.color_buf_idx];
  return float4(colors[input.color_idx]);
}
#endif
