#include "ShaderInterop.h"
#include "MeshVSOut.hlsli"

struct MeshVSIn {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

MeshVsOut main(MeshVSIn vsIn) {
    float4 worldPos4 = mul(float4(vsIn.position, 1), gPerDrawConstants.modelMtx);

    MeshVsOut ret;
    ret.worldPos = worldPos4.xyz;
    ret.clipPos = mul(worldPos4, gPerCamConstants.viewProjMtx);
    ret.normal = mul(vsIn.normal, gPerDrawConstants.normalMtx);
    ret.uv = vsIn.uv;
    ret.viewPosZ = ret.clipPos.w;

    float3 tangentWS = normalize(mul(vsIn.tangent, gPerDrawConstants.normalMtx));
    const float3 normalWS = normalize(mul(vsIn.normal, gPerDrawConstants.normalMtx));
    tangentWS = normalize(tangentWS - dot(tangentWS, normalWS) * normalWS);
    const float3 bitangentWS = cross(normalWS, tangentWS);
    ret.tbnMtx = float3x3(tangentWS, bitangentWS, normalWS);

    return ret;
}