#ifndef SKYBOX_HLSLI
#define SKYBOX_HLSLI

#include "shader_interop.h"


struct DrawParams {
  uint pos_buf_idx;
  uint per_view_cb_idx;
  uint per_frame_cb_idx;
  uint cubemap_idx;
  uint samp_idx;
};


ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);


struct VertexOut {
  float4 pos_cs : SV_Position;
  float3 uv : TEXCOORD;
};


VertexOut VsMain(const uint vertex_id : SV_VertexID) {
  const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_draw_params.pos_buf_idx];
  const float4 pos_os = positions[vertex_id];

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_draw_params.per_view_cb_idx];

  VertexOut ret;
  ret.pos_cs = mul(float4(mul(pos_os.xyz, (float3x3)per_view_cb.viewMtx), 1), per_view_cb.projMtx);
  ret.uv = pos_os.xyz;

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_draw_params.per_frame_cb_idx];

  if (per_frame_cb.isUsingReversedZ) {
    ret.pos_cs.z = 0;
  } else {
    ret.pos_cs = ret.pos_cs.xyww;
  }

  return ret;
}


float4 PsMain(const VertexOut vs_out) : SV_Target {
  const TextureCube cubemap = ResourceDescriptorHeap[g_draw_params.cubemap_idx];
  const SamplerState samp = SamplerDescriptorHeap[g_draw_params.samp_idx];
  return float4(cubemap.Sample(samp, vs_out.uv).rgb, 1);
}
#endif
