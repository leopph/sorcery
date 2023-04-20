#include "ShaderInterop.h"

TEXTURE2D(gSrcTex, float4, RES_SLOT_TONE_MAP_SRC);


float4 main(const float4 pixelCoord : SV_POSITION) : SV_TARGET {
    float3 pixelColor = gSrcTex.Load(pixelCoord).rgb;

    pixelColor = pixelColor / (pixelColor + 1.0);
#pragma warning(push)
	// warns about negative pow base, irrelevant here as pixel color should never be negative
#pragma warning(disable: 3571)
    pixelColor = pow(pixelColor, invGamma);
#pragma warning(pop)

    return float4(pixelColor, 1);
}