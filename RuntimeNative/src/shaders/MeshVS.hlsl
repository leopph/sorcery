#include "ShaderInterop.h"
#include "MeshVSOut.hlsli"

struct MeshVSIn {
    float3 vertexPos : POSITION;
    float3 vertexNorm : NORMAL;
    float2 vertexUV : TEXCOORD;
};

MeshVsOut main(MeshVSIn vsIn) {
    float4 worldPos4 = mul(float4(vsIn.vertexPos, 1), modelMtx);

    MeshVsOut ret;
    ret.worldPos = worldPos4.xyz;
    ret.clipPos = mul(worldPos4, gPerCamConstants.viewProjMtx);
    ret.normal = mul(vsIn.vertexNorm, normalMtx);
    ret.uv = vsIn.vertexUV;
    ret.viewPosZ = ret.clipPos.w;

    return ret;
}