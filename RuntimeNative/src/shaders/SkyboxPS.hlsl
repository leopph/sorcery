#include "SkyboxVSOut.hlsli"

sampler gSampler : register(s0);
TextureCube gCubemap : register(t0);

float4 main(const SkyboxVSOut vsOut) : SV_TARGET
{
    return float4(gCubemap.Sample(gSampler, vsOut.uv).rgb, 1);
}