#ifndef DEPTH_ONLY_HLSLI
#define DEPTH_ONLY_HLSLI

#include "common.hlsli"
#include "mesh_shader_core.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(DepthOnlyDrawParams);
DECLARE_DRAW_CALL_PARAMS(g_draw_call_params);


struct VertexAttributes {
  float4 pos_cs : SV_POSITION;
  float2 uv : TEXCOORD;
};


struct PrimitiveAttributes {
  uint rt_idx : SV_RenderTargetArrayIndex;
};


class VertexProcessor {
  static VertexAttributes GetVertexAttributes(uint const vertex_idx) {
    StructuredBuffer<float4> const positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
    float4 const pos_os = positions[vertex_idx];

    const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_params.per_draw_cb_idx];
    float4 const pos_ws = mul(pos_os, per_draw_cb.modelMtx);

    const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
    float4 const pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

    StructuredBuffer<float2> const uvs = ResourceDescriptorHeap[g_params.uv_buf_idx];
    float2 const uv = uvs[vertex_idx];

    VertexAttributes ret;
    ret.pos_cs = pos_cs;
    ret.uv = uv;
    return ret;
  }
};


class PrimitiveProcessor {
  static PrimitiveAttributes GetPrimitiveAttributes(uint const prim_idx) {
    PrimitiveAttributes ret;
    ret.rt_idx = g_params.rt_idx;
    return ret;
  }
};


[numthreads(AS_THREAD_GROUP_SIZE, 1, 1)]
void AsMain(uint const dtid : SV_DispatchThreadID) {
  AmpShaderCore(dtid, g_params.meshlet_offset, g_params.meshlet_count, g_params.cull_data_buf_idx,
    g_params.per_draw_cb_idx, g_params.per_view_cb_idx);
}


[outputtopology("triangle")]
[numthreads(MS_THREAD_GROUP_SIZE, 1, 1)]
void MsMain(uint const gid : SV_GroupID,
            uint const gtid : SV_GroupThreadID,
            in payload CullingPayload payload,
            out vertices VertexAttributes out_vertices[MESHLET_MAX_VERTS],
            out primitives PrimitiveAttributes out_primitives[MESHLET_MAX_PRIMS],
            out indices uint3 out_indices[MESHLET_MAX_PRIMS]) {
  MeshShaderCore<VertexProcessor, PrimitiveProcessor>(gtid, payload.GetMeshletIndex(gid) + g_params.meshlet_offset,
    g_params.base_vertex, g_params.idx32, GetResource(g_params.meshlet_buf_idx),
    GetResource(g_params.vertex_idx_buf_idx), GetResource(g_params.prim_idx_buf_idx), out_vertices, out_primitives,
    out_indices);
}


void PsMain(VertexAttributes const ps_in) {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_params.mtl_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    Texture2D<float> const opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];
    SamplerState const samp = SamplerDescriptorHeap[g_params.samp_idx];

    if (opacity_map.Sample(samp, ps_in.uv) < mtl.alphaThreshold) {
      discard;
    }
  }
}
#endif
