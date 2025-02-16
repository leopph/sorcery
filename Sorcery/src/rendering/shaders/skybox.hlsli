#ifndef SKYBOX_HLSLI
#define SKYBOX_HLSLI

#include "common.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(SkyboxDrawParams);


struct PsIn {
  float4 pos_cs : SV_Position;
  float3 uv : TEXCOORD;
};


PsIn VsMain(const uint vertex_id : SV_VertexID) {
  const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
  const float4 pos_os = positions[vertex_id];

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

  PsIn ret;
  ret.pos_cs = mul(float4(mul(pos_os.xyz, (float3x3)per_view_cb.viewMtx), 1), per_view_cb.projMtx);
  ret.uv = pos_os.xyz;
  ret.pos_cs.z = 0;
  return ret;
}


float4 PsMain(const PsIn vs_out) : SV_Target {
  const TextureCube cubemap = ResourceDescriptorHeap[g_params.cubemap_idx];
  const SamplerState samp = SamplerDescriptorHeap[g_params.samp_idx];
  return float4(cubemap.Sample(samp, vs_out.uv).rgb, 1);
}
#endif
