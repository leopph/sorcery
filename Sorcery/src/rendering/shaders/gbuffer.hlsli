#ifndef GBUFFER_HLSLI
#define GBUFFER_HLSLI

#define MESH_SHADER_NO_PRIMITIVE_ATTRIBUTES
#include "mesh_shader_core.hlsli"

#include "common.hlsli"
#include "gbuffer_utils.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(GBufferDrawParams);
DECLARE_DRAW_CALL_PARAMS(g_draw_call_params);


struct PsIn {
  float4 pos_cs : SV_POSITION;
  float3 pos_ws : POSITIONWS;
  float3 pos_vs : POSITIONVS;
  float3 norm_ws : NORMAL;
  float2 uv : TEXCOORD;
  float3x3 tbn_mtx_ws : TBNMTXWS;
};


class VertexProcessor {
  static PsIn CalculateVertex(uint const vertex_idx, uint const instance_idx) {
    StructuredBuffer<float4> const positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
    float4 const pos_os = positions[vertex_idx];

    const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_params.per_draw_cb_idx];
    float4 const pos_ws = mul(pos_os, per_draw_cb.modelMtx);

    const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
    float4 const pos_vs = mul(pos_ws, per_view_cb.viewMtx);
    float4 const pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

    StructuredBuffer<float4> const normals = ResourceDescriptorHeap[g_params.norm_buf_idx];
    float4 const norm_os = normals[vertex_idx];
    float3 const norm_ws = normalize(mul(norm_os.xyz, (float3x3)per_draw_cb.invTranspModelMtx));

    StructuredBuffer<float4> const tangents = ResourceDescriptorHeap[g_params.tan_buf_idx];
    float4 const tan_os = tangents[vertex_idx];
    float3 tan_ws = normalize(mul(tan_os.xyz, (float3x3)per_draw_cb.modelMtx));
    tan_ws = normalize(tan_ws - dot(tan_ws, norm_ws) * norm_ws);
    float3 const bitan_ws = cross(norm_ws, tan_ws);
    float3x3 const tbn_mtx_ws = float3x3(tan_ws, bitan_ws, norm_ws);

    StructuredBuffer<float2> const uvs = ResourceDescriptorHeap[g_params.uv_buf_idx];
    float2 const uv = uvs[vertex_idx];

    PsIn ret;
    ret.pos_ws = pos_ws.xyz;
    ret.pos_vs = pos_vs.xyz;
    ret.pos_cs = pos_cs;
    ret.norm_ws = norm_ws;
    ret.uv = uv;
    ret.tbn_mtx_ws = tbn_mtx_ws;

    return ret;
  }
};


DECLARE_MESH_SHADER_MAIN(MsMain) {
  MeshShaderCore<VertexProcessor>(gid, gtid, g_params.meshlet_buf_idx, g_params.vertex_idx_buf_idx,
    g_params.prim_idx_buf_idx, g_params.meshlet_offset, g_params.meshlet_count, g_params.instance_offset,
    g_params.instance_count, g_params.base_vertex, g_params.idx32, out_vertices, out_indices);
}


void PsMain(PsIn const ps_in, out float4 out0 : SV_Target0, out float2 out1 : SV_Target1,
            out float2 out2 : SV_Target2) {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_params.mtl_idx];
  SamplerState const mtl_samp = SamplerDescriptorHeap[g_params.mtl_samp_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    Texture2D<float> const opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];

    if (opacity_map.Sample(mtl_samp, ps_in.uv) < mtl.alphaThreshold) {
      discard;
    }
  }

  float3 albedo = mtl.albedo;

  if (mtl.albedo_map_idx != INVALID_RES_IDX) {
    Texture2D const albedo_map = ResourceDescriptorHeap[mtl.albedo_map_idx];
    albedo *= albedo_map.Sample(mtl_samp, ps_in.uv).rgb;
  }

  float metallic = mtl.metallic;

  if (mtl.metallic_map_idx != INVALID_RES_IDX) {
    Texture2D<float> const metallic_map = ResourceDescriptorHeap[mtl.metallic_map_idx];
    metallic *= metallic_map.Sample(mtl_samp, ps_in.uv).r;
  }

  float roughness = mtl.roughness;

  if (mtl.roughness_map_idx != INVALID_RES_IDX) {
    Texture2D<float> const roughness_map = ResourceDescriptorHeap[mtl.roughness_map_idx];
    roughness *= roughness_map.Sample(mtl_samp, ps_in.uv).r;
  }

  float ao = mtl.ao;

  if (mtl.ao_map_idx != INVALID_RES_IDX) {
    Texture2D<float> const ao_map = ResourceDescriptorHeap[mtl.ao_map_idx];
    ao *= ao_map.Sample(mtl_samp, ps_in.uv).r;
  }

  float3 norm_ws = normalize(ps_in.norm_ws);

  if (mtl.normal_map_idx != INVALID_RES_IDX) {
    Texture2D<float3> const normal_map = ResourceDescriptorHeap[mtl.normal_map_idx];
    norm_ws = normal_map.Sample(mtl_samp, ps_in.uv).rgb;
    norm_ws *= 2.0;
    norm_ws -= 1.0;
    norm_ws = normalize(mul(normalize(norm_ws), ps_in.tbn_mtx_ws));
  }

  out0 = PackGBuffer0(albedo, ao);
  out1 = PackGBuffer1(norm_ws);
  out2 = PackGBuffer2(roughness, metallic);
}

#endif
