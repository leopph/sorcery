#include "ShaderInterop.h"
#include "SkyboxVSOut.hlsli"

SkyboxVSOut main(const float3 pos : POSITION)
{
    SkyboxVSOut ret;
    ret.uv = pos;
    ret.clipPos = mul(float4(pos, 1), skyboxViewProjMtx).xyww;
    return ret;
}