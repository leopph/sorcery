#include "SkyboxVSOut.hlsli"

cbuffer Matrices : register(b0) {
    row_major float4x4 viewProjMtx;
};

SkyboxVSOut main(const float3 pos : POSITION)
{
    SkyboxVSOut ret;
    ret.uv = pos;
    ret.clipPos = mul(float4(pos, 1), viewProjMtx).xyww;
    return ret;
}