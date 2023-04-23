#ifndef UTIL_HLSLI
#define UTIL_HLLSI

#include "ShaderInterop.h"

float3 VisualizeShadowCascades(const float viewPosZ) {
    int cascadeIdx = 0;

    while (cascadeIdx < gPerFrameConstants.shadowCascadeCount && viewPosZ > gPerCamConstants.shadowCascadeFarBounds[cascadeIdx]) {
        cascadeIdx += 1;
    }

    if (cascadeIdx == gPerFrameConstants.shadowCascadeCount) {
        return float3(1, 1, 1);
    }

    float3 ret;

    switch (cascadeIdx) {
    case 0:
            ret = float3(108, 110, 160);
            break;
    case 1:
            ret = float3(184, 216, 186);
            break;
    case 2:
            ret = float3(252, 221, 188);
            break;
    case 3:
            ret = float3(239, 149, 157);
            break;
    default:
            ret = float3(0, 0, 0); // This should never be reached
            break;
    }

    return pow(ret / 255.0, 2.2);
}

#endif