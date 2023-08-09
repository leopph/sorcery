#include "ShaderInterop.h"

TEXTURE2D(gSrcTex, float4, RES_SLOT_POST_PROCESS_SRC);


float4 main(const float4 pixelCoord : SV_POSITION) : SV_TARGET {
    float3 pixelColor = gSrcTex.Load(int3(pixelCoord.xy, 0)).rgb;

    // Tone mapping
    pixelColor = pixelColor / (pixelColor + 1.0);

    // Gamma correction
    //pixelColor = pow(pixelColor, invGamma);

    return float4(pixelColor, 1);
}