#include "ShaderInterop.h"
#include "GizmoVsOut.hlsli"

struct DrawParams {
  uint vertex_buf_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

GizmoVsOut main(const uint vertex_id : SV_VertexID, const uint instance_id : SV_InstanceID) {
  const StructuredBuffer<ShaderLineGizmoVertexData> vertices = ResourceDescriptorHeap[g_draw_params.vertex_buf_idx];
  const ShaderLineGizmoVertexData data = vertices[instance_id];
  
  GizmoVsOut ret;
  ret.colorIdx = data.colorIdx;
  ret.position = mul(float4(vertex_id == 0 ? data.from : data.to, 1), gPerViewConstants.viewProjMtx);
  return ret;
}
