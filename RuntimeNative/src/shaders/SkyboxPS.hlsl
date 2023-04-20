#include "ShaderInterop.h"
#include "SkyboxVSOut.hlsli"

TEXTURECUBE(gCubemap, float4, RES_SLOT_SKYBOX_CUBEMAP);
SAMPLERSTATE(gSampler, SAMPLER_SLOT_SKYBOX_CUBEMAP);


float4 main(const SkyboxVSOut vsOut) : SV_TARGET
{
    return float4(pow(gCubemap.Sample(gSampler, vsOut.uv).rgb, 2.2), 1);
}