#ifndef MESH_VS_OUT_HLSLI
#define MESH_VS_OUT_HLSLI

struct MeshVsOut {
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float4 clipPos : SV_POSITION;
    float2 uv : TEXCOORD;
};

#endif