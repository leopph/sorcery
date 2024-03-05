#include "shader_interop.h"
#include "DepthNormalVsOut.hlsli"

struct DrawParams {
  uint pos_buf_idx;
  uint norm_buf_idx;
  uint tan_buf_idx;
  uint uv_buf_idx;
  uint per_draw_cb_idx;
  uint per_view_cb_idx;
  uint per_frame_cb_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);


DepthNormalVsOut main(const uint vertex_id : SV_VertexID) {
  const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_draw_params.pos_buf_idx];

  const float4 pos_os = positions[vertex_id];

  const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_draw_params.per_draw_cb_idx];
  const float4 pos_ws = mul(pos_os, per_draw_cb.modelMtx);

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_draw_params.per_view_cb_idx];
  const float4 pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

  const StructuredBuffer<float4> normals = ResourceDescriptorHeap[g_draw_params.norm_buf_idx];
  const float4 norm_os = normals[vertex_id];
  const float3 norm_ws = normalize(mul(norm_os.xyz, (float3x3)per_draw_cb.invTranspModelMtx));

  const StructuredBuffer<float4> tangents = ResourceDescriptorHeap[g_draw_params.tan_buf_idx];
  const float4 tan_os = tangents[vertex_id];
  float3 tan_ws = normalize(mul(tan_os.xyz, (float3x3)per_draw_cb.modelMtx));
  tan_ws = normalize(tan_ws - dot(tan_ws, norm_ws) * norm_ws);
  const float3 bitan_ws = cross(norm_ws, tan_ws);
  const float3x3 tbn_mtx_ws = float3x3(tan_ws, bitan_ws, norm_ws);

  const StructuredBuffer<float2> uvs = ResourceDescriptorHeap[g_draw_params.uv_buf_idx];
  const float2 uv = uvs[vertex_id];

  DepthNormalVsOut ret;
  ret.positionCS = pos_cs;
  ret.normalWS = norm_ws;
  ret.uv = uv;
  ret.tbnMtxWS = tbn_mtx_ws;
  return ret;
}
