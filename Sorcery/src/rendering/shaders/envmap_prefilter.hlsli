#ifndef ENVMAP_PREFILTER_HLSLI
#define ENVMAP_PREFILTER_HLSLI

#define MESH_SHADER_NO_PAYLOAD

#include "brdf.hlsli"
#include "common.hlsli"
#include "mesh_shader_core.hlsli"
#include "sequences.hlsli"
#include "shader_interop.h"


DECLARE_PARAMS(EnvmapPrefilterDrawParams);


static uint const kSampleCount = 1024u;


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

  float3 const N = normalize(attr.pos_os);
  float3 const R = N;
  float3 const V = R;

  float total_weight = 0.0;
  float3 prefiltered_color = 0;

  for (uint i = 0u; i < kSampleCount; ++i) {
    float2 const Xi = Hammersley(i, kSampleCount);
    float3 const H = ImportanceSampleGGX(Xi, N, g_params.roughness);
    float3 const L = normalize(2.0 * dot(V, H) * H - V);

    float const NdotL = max(dot(N, L), 0.0);

    if (NdotL > 0.0) {
      float const NdotH = saturate(dot(N, H));
      // sample from the environment's mip level based on roughness/pdf
      float const D = DistributionTrowbridgeReitz(NdotH, g_params.roughness);
      float const HdotV = max(dot(H, V), 0.0);
      float const pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

      float2 cubemap_size;
      env_map.GetDimensions(cubemap_size.x, cubemap_size.y);
      float const resolution = max(cubemap_size.x, cubemap_size.y); // resolution of source cubemap (per face)
      float const sa_texel = 4.0 * PI / (6.0 * resolution * resolution);
      float const sa_sample = 1.0 / (float(kSampleCount) * pdf + 0.0001);

      float const mip_level = g_params.roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample / sa_texel);

      prefiltered_color += env_map.SampleLevel(tri_clamp_samp, L, mip_level).rgb * NdotL;
      total_weight += NdotL;
    }
  }

  prefiltered_color = prefiltered_color / total_weight;
  return float4(prefiltered_color, 1.0);
}


#endif
