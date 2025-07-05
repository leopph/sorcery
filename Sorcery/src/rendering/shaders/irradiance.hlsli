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


static float const kPi = 3.14159265359;


float4 PsMain(VertexAttributes const ps_in) : SV_Target {
  TextureCube const environment_map = GetResource(g_params.environment_map_idx);
  SamplerState const samp = GetSampler(g_params.point_clamp_samp_idx);

  float3 const normal = normalize(ps_in.pos_os);

  float3 irradiance = float3(0, 0, 0);

  float3 up = float3(0, 1, 0);
  float3 const right = normalize(cross(up, normal));
  up = normalize(cross(normal, right));

  float const sample_delta = 0.025;
  float sample_count = 0;

  for (float phi = 0; phi < 2 * kPi; phi += sample_delta) {
    for (float theta = 0; theta < 0.5 * kPi; theta += sample_delta) {
      float3 const tangent_sample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      float3 const sample_vec = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal;

      irradiance += environment_map.Sample(samp, sample_vec).rgb * cos(theta) * sin(theta);
      sample_count++;
    }
  }

  irradiance = kPi * irradiance / sample_count;

  return float4(irradiance, 1);
}

#endif
