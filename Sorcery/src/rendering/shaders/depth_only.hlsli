#ifndef DEPTH_ONLY_HLSLI
#define DEPTH_ONLY_HLSLI

#include "common.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(DepthOnlyDrawParams);


struct VertexOut {
  float4 pos_cs : SV_POSITION;
  float2 uv : TEXCOORD;
  uint rt_idx : SV_RenderTargetArrayIndex;
};


VertexOut VsMain(const uint vertex_id : SV_VertexID) {
  const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
  const float4 pos_os = positions[vertex_id];

  const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_params.per_draw_cb_idx];
  const float4 pos_ws = mul(pos_os, per_draw_cb.modelMtx);

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
  const float4 pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

  const StructuredBuffer<float2> uvs = ResourceDescriptorHeap[g_params.uv_buf_idx];
  const float2 uv = uvs[vertex_id];

  VertexOut ret;
  ret.pos_cs = pos_cs;
  ret.uv = uv;
  ret.rt_idx = g_params.rt_idx;
  return ret;
}


void PsMain(const VertexOut vs_out) {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_params.mtl_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];
    const SamplerState samp = SamplerDescriptorHeap[g_params.samp_idx];

    if (opacity_map.Sample(samp, vs_out.uv) < mtl.alphaThreshold) {
      discard;
    }
  }
}
#endif
