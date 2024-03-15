#ifndef DEPTH_NORMAL_HLSLI
#define DEPTH_NORMAL_HLSLI

#include "common.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(DepthNormalDrawParams);
DECLARE_DRAW_CALL_PARAMS(g_draw_call_params);


struct VertexOut {
  float4 pos_cs : SV_POSITION;
  float3 norm_ws : NORMAL;
  float2 uv : TEXCOORD;
  float3x3 tbn_mtx_ws : TBNMTXWS;
  uint rt_idx : SV_RenderTargetArrayIndex;
};


VertexOut VsMain(uint vertex_id : SV_VertexID) {
  vertex_id += g_draw_call_params.base_vertex;

  const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_params.pos_buf_idx];

  const float4 pos_os = positions[vertex_id];

  const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_params.per_draw_cb_idx];
  const float4 pos_ws = mul(pos_os, per_draw_cb.modelMtx);

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
  const float4 pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

  const StructuredBuffer<float4> normals = ResourceDescriptorHeap[g_params.norm_buf_idx];
  const float4 norm_os = normals[vertex_id];
  const float3 norm_ws = normalize(mul(norm_os.xyz, (float3x3)per_draw_cb.invTranspModelMtx));

  const StructuredBuffer<float4> tangents = ResourceDescriptorHeap[g_params.tan_buf_idx];
  const float4 tan_os = tangents[vertex_id];
  float3 tan_ws = normalize(mul(tan_os.xyz, (float3x3)per_draw_cb.modelMtx));
  tan_ws = normalize(tan_ws - dot(tan_ws, norm_ws) * norm_ws);
  const float3 bitan_ws = cross(norm_ws, tan_ws);
  const float3x3 tbn_mtx_ws = float3x3(tan_ws, bitan_ws, norm_ws);

  const StructuredBuffer<float2> uvs = ResourceDescriptorHeap[g_params.uv_buf_idx];
  const float2 uv = uvs[vertex_id];

  VertexOut ret;
  ret.pos_cs = pos_cs;
  ret.norm_ws = norm_ws;
  ret.uv = uv;
  ret.tbn_mtx_ws = tbn_mtx_ws;
  ret.rt_idx = g_params.rt_idx;
  return ret;
}


float4 PsMain(const VertexOut vs_out) : SV_Target {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_params.mtl_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];
    const SamplerState samp = SamplerDescriptorHeap[g_params.samp_idx];

    if (opacity_map.Sample(samp, vs_out.uv) < mtl.alphaThreshold) {
      discard;
    }
  }

  if (mtl.normal_map_idx != INVALID_RES_IDX) {
    const Texture2D<float3> normal_map = ResourceDescriptorHeap[mtl.normal_map_idx];
    const SamplerState samp = SamplerDescriptorHeap[g_params.samp_idx];

    const float3 normal_ts = normal_map.Sample(samp, vs_out.uv).rgb * 2.0 - 1.0;
    return float4(normalize(mul(normal_ts, vs_out.tbn_mtx_ws)), 0);
  }

  return float4(normalize(vs_out.norm_ws), 0);
}
#endif
