#include "ShaderInterop.h"
#include "SkyboxVSOut.hlsli"
#include "Samplers.hlsli"

TEXTURECUBE(gCubemap, float4, RES_SLOT_SKYBOX_CUBEMAP);


float4 main(const SkyboxVSOut vsOut) : SV_TARGET
{
    return float4(gCubemap.Sample(gSamplerAf16, vsOut.uv).rgb, 1);
}