#ifndef OBJECT_PBR_HLSLI
#define OBJECT_PBR_HLSLI

#include "common.hlsli"
#include "lighting.hlsli"
#include "mesh_shader_core.hlsli"
#include "shader_interop.h"

DECLARE_PARAMS(ObjectDrawParams);
DECLARE_DRAW_CALL_PARAMS(g_draw_call_params);

struct PsIn {
  float4 pos_cs : SV_POSITION;
  float3 pos_ws : POSITIONWS;
  float3 pos_vs : POSITIONVS;
  float3 norm_ws : NORMAL;
  float2 uv : TEXCOORD;
  float3x3 tbn_mtx_ws : TBNMTXWS;
};

class VertexProcessor
{
  static PsIn CalculateVertex(const uint vertex_idx, const uint instance_idx) {
    const StructuredBuffer<float4> positions = ResourceDescriptorHeap[g_params.pos_buf_idx];
    const float4 pos_os = positions[vertex_idx];

    const ConstantBuffer<ShaderPerDrawConstants> per_draw_cb = ResourceDescriptorHeap[g_params.per_draw_cb_idx];
    const float4 pos_ws = mul(pos_os, per_draw_cb.modelMtx);

    const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
    const float4 pos_vs = mul(pos_ws, per_view_cb.viewMtx);
    const float4 pos_cs = mul(pos_ws, per_view_cb.viewProjMtx);

    const StructuredBuffer<float4> normals = ResourceDescriptorHeap[g_params.norm_buf_idx];
    const float4 norm_os = normals[vertex_idx];
    const float3 norm_ws = normalize(mul(norm_os.xyz, (float3x3) per_draw_cb.invTranspModelMtx));

    const StructuredBuffer<float4> tangents = ResourceDescriptorHeap[g_params.tan_buf_idx];
    const float4 tan_os = tangents[vertex_idx];
    float3 tan_ws = normalize(mul(tan_os.xyz, (float3x3) per_draw_cb.modelMtx));
    tan_ws = normalize(tan_ws - dot(tan_ws, norm_ws) * norm_ws);
    const float3 bitan_ws = cross(norm_ws, tan_ws);
    const float3x3 tbn_mtx_ws = float3x3(tan_ws, bitan_ws, norm_ws);

    const StructuredBuffer<float2> uvs = ResourceDescriptorHeap[g_params.uv_buf_idx];
    const float2 uv = uvs[vertex_idx];

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
  MeshShaderCore < VertexProcessor > (gid, gtid, g_params.meshlet_buf_idx, g_params.vertex_idx_buf_idx,
    g_params.prim_idx_buf_idx, g_params.meshlet_offset, g_params.meshlet_count, g_params.instance_offset,
    g_params.instance_count, out_verts, out_tris);
}


float4 PsMain(const PsIn vs_out) : SV_Target {
  const ConstantBuffer<ShaderMaterial> mtl = ResourceDescriptorHeap[g_params.mtl_idx];
  const SamplerState mtl_samp = SamplerDescriptorHeap[g_params.mtl_samp_idx];

  if (mtl.blendMode == BLEND_MODE_ALPHA_CLIP && mtl.opacity_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> opacity_map = ResourceDescriptorHeap[mtl.opacity_map_idx];

    if (opacity_map.Sample(mtl_samp, vs_out.uv) < mtl.alphaThreshold) {
      discard;
    }
  }

  float3 albedo = mtl.albedo;

  if (mtl.albedo_map_idx != INVALID_RES_IDX) {
    const Texture2D albedo_map = ResourceDescriptorHeap[mtl.albedo_map_idx];
    albedo *= albedo_map.Sample(mtl_samp, vs_out.uv).rgb;
  }

  float metallic = mtl.metallic;

  if (mtl.metallic_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> metallic_map = ResourceDescriptorHeap[mtl.metallic_map_idx];
    metallic *= metallic_map.Sample(mtl_samp, vs_out.uv).r;
  }

  float roughness = mtl.roughness;

  if (mtl.roughness_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> roughness_map = ResourceDescriptorHeap[mtl.roughness_map_idx];
    roughness *= roughness_map.Sample(mtl_samp, vs_out.uv).r;
  }

  const ConstantBuffer<ShaderPerFrameConstants> per_frame_cb = ResourceDescriptorHeap[g_params.per_frame_cb_idx];
  const float2 screen_uv = vs_out.pos_cs.xy / per_frame_cb.screenSize;

  const SamplerState point_clamp_samp = SamplerDescriptorHeap[g_params.point_clamp_samp_idx];
  const Texture2D<float> ssao_tex = ResourceDescriptorHeap[g_params.ssao_tex_idx];
  float ao = mtl.ao * ssao_tex.Sample(point_clamp_samp, screen_uv).r;

  if (mtl.ao_map_idx != INVALID_RES_IDX) {
    const Texture2D<float> ao_map = ResourceDescriptorHeap[mtl.ao_map_idx];
    ao *= ao_map.Sample(mtl_samp, vs_out.uv).r;
  }

  float3 norm_ws = normalize(vs_out.norm_ws);

  if (mtl.normal_map_idx != INVALID_RES_IDX) {
    const Texture2D<float3> normal_map = ResourceDescriptorHeap[mtl.normal_map_idx];
    norm_ws = normal_map.Sample(mtl_samp, vs_out.uv).rgb;
    norm_ws *= 2.0;
    norm_ws -= 1.0;
    norm_ws = normalize(mul(normalize(norm_ws), vs_out.tbn_mtx_ws));
  }

  const ConstantBuffer<ShaderPerViewConstants> per_view_cb = ResourceDescriptorHeap[g_params.per_view_cb_idx];
  const float3 dir_to_cam_ws = normalize(per_view_cb.viewPos - vs_out.pos_ws);

  float3 out_color = per_frame_cb.ambientLightColor * albedo * ao;

  const StructuredBuffer<ShaderLight> lights = ResourceDescriptorHeap[g_params.light_buf_idx];

  const Texture2DArray<float> dir_light_shadow_map_arr = ResourceDescriptorHeap[g_params.dir_shadow_arr_idx];
  const Texture2D<float> punc_light_shadow_atlas = ResourceDescriptorHeap[g_params.punc_shadow_atlas_idx];
  const SamplerComparisonState shadow_samp = SamplerDescriptorHeap[g_params.shadow_samp_idx];

  for (uint i = 0; i < g_params.light_count; i++) {
    if (lights[i].type == 0) {
      out_color += CalculateDirLight(lights[i], vs_out.pos_ws, norm_ws, dir_to_cam_ws, vs_out.pos_vs.z, albedo,
        metallic, roughness, dir_light_shadow_map_arr, shadow_samp, per_frame_cb.shadowFilteringMode,
        per_view_cb.shadowCascadeSplitDistances, per_frame_cb.shadowCascadeCount, per_frame_cb.visualizeShadowCascades);
    } else if (lights[i].type == 1) {
      out_color += CalculateSpotLight(lights[i], vs_out.pos_ws, norm_ws, dir_to_cam_ws, albedo, metallic,
        roughness, punc_light_shadow_atlas, shadow_samp, per_frame_cb.shadowFilteringMode);
    } else if (lights[i].type == 2) {
      out_color += CalculatePointLight(lights[i], vs_out.pos_ws, norm_ws, dir_to_cam_ws, albedo, metallic,
        roughness, punc_light_shadow_atlas, shadow_samp, per_frame_cb.shadowFilteringMode);
    }
  }

  return float4(out_color, 1);
}


#endif
