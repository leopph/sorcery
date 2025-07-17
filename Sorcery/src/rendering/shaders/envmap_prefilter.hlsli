#ifndef ENVMAP_PREFILTER_HLSLI
#define ENVMAP_PREFILTER_HLSLI

#define MESH_SHADER_NO_PAYLOAD

#include "brdf.hlsli"
#include "common.hlsli"
#include "mesh_shader_core.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(EnvmapPrefilterDrawParams);


struct VertexAttributes {
  float4 pos_cs : SV_Position;
  float3 pos_os : POSITIONOS;
};


class VertexProcessor {
  static VertexAttributes GetVertexAttributes(uint const vertex_id) {
    StructuredBuffer<float4> const positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
    float4 const pos_os = positions[vertex_id];

    VertexAttributes ret;
    ret.pos_os = pos_os.xyz;
    ret.pos_cs = mul(pos_os, g_params.view_proj_mtx);
    return ret;
  }
};


struct PrimitiveAttributes {
  uint rt_idx : SV_RenderTargetArrayIndex;
};


class PrimitiveProcessor {
  static PrimitiveAttributes GetPrimitiveAttributes(uint const prim_idx) {
    PrimitiveAttributes ret;
    ret.rt_idx = g_params.rt_idx;
    return ret;
  }
};


[outputtopology("triangle")]
[numthreads(MS_THREAD_GROUP_SIZE, 1, 1)]
void MsMain(uint const gid : SV_GroupID,
            uint const gtid : SV_GroupThreadID,
            out vertices VertexAttributes out_vertices[MESHLET_MAX_VERTS],
            out primitives PrimitiveAttributes out_primitives[MESHLET_MAX_PRIMS],
            out indices uint3 out_indices[MESHLET_MAX_PRIMS]) {
  MeshShaderCore<VertexProcessor, PrimitiveProcessor>(gtid, gid, 0, true, GetResource(g_params.meshlet_buf_idx),
    GetResource(g_params.vertex_idx_buf_idx), GetResource(g_params.prim_idx_buf_idx), out_vertices, out_primitives,
    out_indices);
}


float4 PsMain(VertexAttributes const attr) : SV_Target {
  TextureCube const env_map = GetResource(g_params.env_map_idx);
  SamplerState const tri_clamp_samp = GetSampler(g_params.tri_clamp_samp_idx);

  float3 const prefiltered = PrefilterEnvMap(normalize(attr.pos_os), g_params.roughness, env_map, tri_clamp_samp);
  return float4(prefiltered, 1);
}


#endif
