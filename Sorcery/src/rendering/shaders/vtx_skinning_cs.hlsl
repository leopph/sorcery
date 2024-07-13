#include "common.hlsli"
#include "shader_interop.h"

DECLARE_PARAMS(VertexSkinningDrawParams);


[numthreads(SKINNING_CS_THREADS, 1, 1)]
void main(uint3 const id : SV_DispatchThreadID) {
  uint const vtx_idx = id.x;

  if (vtx_idx >= g_params.vtx_count) {
    return;
  }

  StructuredBuffer<float4> const vtx_buf = ResourceDescriptorHeap[g_params.vtx_buf_idx];
  StructuredBuffer<float4> const norm_buf = ResourceDescriptorHeap[g_params.norm_buf_idx];
  StructuredBuffer<float4> const tan_buf = ResourceDescriptorHeap[g_params.tan_buf_idx];
  StructuredBuffer<float4> const weight_buf = ResourceDescriptorHeap[g_params.bone_weight_buf_idx];
  StructuredBuffer<uint4> const bone_idx_buf = ResourceDescriptorHeap[g_params.bone_idx_buf_idx];
  StructuredBuffer<float4x4> const bone_mtx_buf = ResourceDescriptorHeap[g_params.bone_buf_idx];
  RWStructuredBuffer<float4> const skinned_vtx_buf = ResourceDescriptorHeap[g_params.skinned_vtx_buf_idx];
  RWStructuredBuffer<float4> const skinned_norm_buf = ResourceDescriptorHeap[g_params.skinned_norm_buf_idx];
  RWStructuredBuffer<float4> const skinned_tan_buf = ResourceDescriptorHeap[g_params.skinned_tan_buf_idx];

  float4 const vtx = vtx_buf[vtx_idx];
  float4 const norm = norm_buf[vtx_idx];
  float4 const tan = tan_buf[vtx_idx];
  float4 const weights = weight_buf[vtx_idx];
  uint4 const indices = bone_idx_buf[vtx_idx];

  float4 skinned_vtx = float4(0, 0, 0, 0);
  float4 skinned_norm = float4(0, 0, 0, 0);
  float4 skinned_tan = float4(0, 0, 0, 0);

  for (uint i = 0; i < 4; i++) {
    // We flip the multiplication order here because HLSL reads the matrices
    // in the buffer as column major whereas in C++ they are row-major
    float4x4 const bone_mtx = bone_mtx_buf[indices[i]];
    skinned_vtx += mul(bone_mtx, vtx) * weights[i];
    skinned_norm += float4(mul((float3x3)bone_mtx, norm.xyz) * weights[i], 0);
    skinned_tan += float4(mul((float3x3)bone_mtx, tan.xyz) * weights[i], 0);
  }

  skinned_vtx_buf[vtx_idx] = skinned_vtx;
  skinned_norm_buf[vtx_idx] = normalize(skinned_norm);
  skinned_tan_buf[vtx_idx] = normalize(skinned_tan);
}
