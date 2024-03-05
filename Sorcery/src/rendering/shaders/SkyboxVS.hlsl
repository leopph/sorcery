#include "ShaderInterop.h"
#include "SkyboxVSOut.hlsli"

struct DrawParams {
  uint pos_buf_idx;
  uint per_view_cb_idx;
  uint per_frame_cb_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

SkyboxVSOut main(const uint vertex_id : SV_VertexID) {
  const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_draw_params.pos_buf_idx];
  const float4 pos_os = positions[vertex_id];

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_draw_params.per_view_cb_idx];

  SkyboxVSOut ret;
  ret.positionCS = mul(float4(mul(pos_os.xyz, (float3x3)per_view_cb.viewMtx), 1), per_view_cb.projMtx);
  ret.uv = pos_os.xyz;

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_draw_params.per_frame_cb_idx];

  if (per_frame_cb.isUsingReversedZ) {
    ret.positionCS.z = 0;
  } else {
    ret.positionCS = ret.positionCS.xyww;
  }

  return ret;
}
