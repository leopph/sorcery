#include "TexturedQuadVSOut.hlsli"

Texture2D gHdrTexture;
SamplerState gSamplerState;

cbuffer Gamma : register(b0) {
    float invGamma;
}

float4 main(const VsOut vsOut) : SV_TARGET {
    float3 pixelColor = gHdrTexture.Sample(gSamplerState, vsOut.uv).xyz;

    pixelColor = pixelColor / (pixelColor + 1.0);
    pixelColor = pow(pixelColor, invGamma);

    return float4(pixelColor, 1);
}