#ifndef SKYBOX_VS_OUT_HLSLI
#define SKYBOX_VS_OUT_HLSLI

struct SkyboxVSOut
{
    float4 positionCS : SV_POSITION;
    float3 uv : TEXCOORD;
};

#endif