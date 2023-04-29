#include "ShaderInterop.h"
#include "MeshVSOut.hlsli"

struct MeshVSIn {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
};

MeshVsOut main(MeshVSIn vsIn) {
    float4 worldPos4 = mul(float4(vsIn.position, 1), gPerDrawConstants.modelMtx);

    MeshVsOut ret;
    ret.worldPos = worldPos4.xyz;
    ret.clipPos = mul(worldPos4, gPerCamConstants.viewProjMtx);
    ret.normal = mul(vsIn.normal, gPerDrawConstants.normalMtx);
    ret.uv = vsIn.uv;
    ret.viewPosZ = ret.clipPos.w;

    const float3 T = normalize(mul(vsIn.tangent, gPerDrawConstants.normalMtx));
    const float3 B = normalize(mul(vsIn.bitangent, gPerDrawConstants.normalMtx));
    const float3 N = normalize(mul(vsIn.normal, gPerDrawConstants.normalMtx));
   ret.tbnMtx = float3x3(T, B, N);

    return ret;
}