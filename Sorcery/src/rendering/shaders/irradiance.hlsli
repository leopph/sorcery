#ifndef IRRADIANCE_HLSLI
#define IRRADIANCE_HLSLI

#define MESH_SHADER_NO_PAYLOAD

#include "common.hlsli"
#include "mesh_shader_core.hlsli"
#include "shader_interop.h"

DECLARE_PARAMS(IrradianceDrawParams);


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


float4 PsMain(VertexAttributes const ps_in) : SV_Target {
  Texture2D const environment_map = GetResource(g_params.environment_map_idx);

  float3 const normal = normalize(ps_in.pos_os);

  float3 irradiance = float3(0, 0, 0);

  // ...

  return float4(irradiance, 1);
}

#endif
