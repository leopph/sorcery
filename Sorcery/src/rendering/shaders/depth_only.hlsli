#ifndef DEPTH_ONLY_HLSLI
#define DEPTH_ONLY_HLSLI

#include "common.hlsli"
#include "mesh_shader_core.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(DepthOnlyDrawParams);
DECLARE_DRAW_CALL_PARAMS(g_draw_call_params);


struct PsIn {
  float4 pos_cs : SV_POSITION;
  float2 uv : TEXCOORD;
};


struct PrimitiveAttributes {
  uint rt_idx : SV_RenderTargetArrayIndex;
};


class VertexProcessor {
  static PsIn CalculateVertex(uint const vertex_idx, uint const instance_idx) {
    StructuredBuffer<float4> const positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
    float4 const pos_os = positions[vertex_idx];

    const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_params.per_draw_cb_idx];
    float4 const pos_ws = mul(pos_os, per_draw_cb.modelMtx);

    const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
    float4 const pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

    StructuredBuffer<float2> const uvs = ResourceDescriptorHeap[g_params.uv_buf_idx];
    float2 const uv = uvs[vertex_idx];

    PsIn ret;
    ret.pos_cs = pos_cs;
    ret.uv = uv;
    return ret;
  }
};


class PrimitiveProcessor {
  static PrimitiveAttributes CalculatePrimitive(uint const prim_idx) {
    PrimitiveAttributes ret;
    ret.rt_idx = g_params.rt_idx;
    return ret;
  }
};


DECLARE_AMP_SHADER_MAIN(AsMain) {
  AmpShaderCore(dtid, g_params.meshlet_count, g_params.cull_data_buf_idx, g_params.per_draw_cb_idx,
    g_params.per_view_cb_idx);
}


DECLARE_MESH_SHADER_MAIN(MsMain) {
  MESH_SHADER_CORE(gid, gtid, g_params.meshlet_buf_idx, g_params.vertex_idx_buf_idx, g_params.prim_idx_buf_idx,
    g_params.meshlet_offset, g_params.meshlet_count, g_params.base_vertex, g_params.idx32, payload, out_vertices,
    out_primitives, out_indices);
}


void PsMain(PsIn const ps_in) {
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
