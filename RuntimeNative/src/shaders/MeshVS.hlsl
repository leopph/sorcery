#include "CBuffers.hlsli"
#include "MeshVSOut.hlsli"

struct MeshVSIn {
    float3 vertexPos : POSITION;
    float3 vertexNorm : NORMAL;
    float2 vertexUV : TEXCOORD;
};

MeshVsOut main(MeshVSIn vsIn) {
    float4 worldPos4 = mul(float4(vsIn.vertexPos, 1), modelMat);

    MeshVsOut ret;
    ret.worldPos = worldPos4.xyz;
    ret.clipPos = mul(worldPos4, viewProjMat);
    ret.normal = mul(vsIn.vertexNorm, normalMat);

    return ret;
}