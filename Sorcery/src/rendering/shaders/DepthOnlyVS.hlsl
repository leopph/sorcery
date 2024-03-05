#include "ShaderInterop.h"
#include "DepthOnlyVsOut.hlsli"

struct DrawParams {
  uint pos_buf_idx;
  uint uv_buf_idx;
  uint per_draw_cb_idx;
  uint per_view_cb_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

DepthOnlyVsOut main(const uint vertex_id : SV_VertexID) {
  const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_draw_params.pos_buf_idx];
  const float4 pos_os = positions[vertex_id];

  const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_draw_params.per_draw_cb_idx];
  const float4 pos_ws = mul(pos_os, per_draw_cb.modelMtx);

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_draw_params.per_view_cb_idx];
  const float4 pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

  const StructuredBuffer<float2> uvs = ResourceDescriptorHeap[g_draw_params.uv_buf_idx];
  const float2 uv = uvs[vertex_id];

  DepthOnlyVsOut ret;
  ret.positionCS = pos_cs;
  ret.uv = uv;
  return ret;
}
