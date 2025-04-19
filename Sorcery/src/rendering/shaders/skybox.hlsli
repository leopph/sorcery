#ifndef SKYBOX_HLSLI
#define SKYBOX_HLSLI

#include "common.hlsli"
#define MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#include "mesh_shader_core.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(SkyboxDrawParams);


struct PsIn {
  float4 pos_cs : SV_Position;
  float3 uv : TEXCOORD;
};


class VertexProcessor {
  static PsIn CalculateVertex(uint const vertex_id, uint const instance_id) {
    StructuredBuffer<float4> const positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
    float4 const pos_os = positions[vertex_id];

    const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

    PsIn ret;
    ret.pos_cs = mul(float4(mul(pos_os.xyz, (float3x3)per_view_cb.viewMtx), 1), per_view_cb.projMtx);
    ret.uv = pos_os.xyz;
#ifdef REVERSE_Z
    ret.pos_cs.z = 0;
#else
    ret.pos_cs.z = ret.pos_cs.w;
#endif
    return ret;
  }
};


DECLARE_MESH_SHADER_MAIN(MsMain) {
  MeshShaderCore<VertexProcessor>(gid, gtid, g_params.meshlet_buf_idx, g_params.vertex_idx_buf_idx,
    g_params.prim_idx_buf_idx, 0, 1, 0, 1, 0, true, out_vertices, out_indices);
}


float4 PsMain(PsIn const vs_out) : SV_Target {
  TextureCube const cubemap = ResourceDescriptorHeap[g_params.cubemap_idx];
  SamplerState const samp = SamplerDescriptorHeap[g_params.samp_idx];
  return float4(cubemap.Sample(samp, vs_out.uv).rgb, 1);
}
#endif
