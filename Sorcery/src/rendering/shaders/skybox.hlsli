#ifndef SKYBOX_HLSLI
#define SKYBOX_HLSLI

#define MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#define MESH_SHADER_NO_PAYLOAD

#include "common.hlsli"
#include "mesh_shader_core.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(SkyboxDrawParams);


struct VertexAttributes {
  float4 pos_cs : SV_Position;
  float3 uv : TEXCOORD;
};


class VertexProcessor {
  static VertexAttributes GetVertexAttributes(uint const vertex_id) {
    StructuredBuffer<float4> const positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
    float4 const pos_os = positions[vertex_id];

    const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];

    VertexAttributes ret;
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


[outputtopology("triangle")]
[numthreads(MS_THREAD_GROUP_SIZE, 1, 1)]
void MsMain(uint const gid : SV_GroupID,
            uint const gtid : SV_GroupThreadID,
            out vertices VertexAttributes out_vertices[MESHLET_MAX_VERTS],
            out indices uint3 out_indices[MESHLET_MAX_PRIMS]) {
  MeshShaderCore<VertexProcessor>(gtid, gid, 0, true,
    GetResource(g_params.meshlet_buf_idx), GetResource(g_params.vertex_idx_buf_idx),
    GetResource(g_params.prim_idx_buf_idx), out_vertices, out_indices);
}


float4 PsMain(VertexAttributes const vs_out) : SV_Target {
  TextureCube const cubemap = ResourceDescriptorHeap[g_params.cubemap_idx];
  SamplerState const samp = SamplerDescriptorHeap[g_params.samp_idx];
  return float4(cubemap.Sample(samp, vs_out.uv).rgb, 1);
}
#endif
