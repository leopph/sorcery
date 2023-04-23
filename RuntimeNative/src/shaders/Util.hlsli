#ifndef UTIL_HLSLI
#define UTIL_HLLSI

#include "ShaderInterop.h"

float3 VisualizeShadowCascades(const float viewPosZ) {
    int cascadeIdx = 0;

    while (cascadeIdx < gPerFrameConstants.shadowCascadeCount && viewPosZ > gPerCamConstants.shadowCascadeFarBounds[cascadeIdx]) {
        cascadeIdx += 1;
    }

    if (cascadeIdx == gPerFrameConstants.shadowCascadeCount) {
        return float3(0, 0, 0);
    }

    switch (cascadeIdx) {
    case 0:
            return float3(1, 0, 0);
    case 1:
            return float3(0, 1, 0);
    case 2:
            return float3(0, 0, 1);
    case 3:
            return float3(1, 0, 1);
    default:
            return float3(0, 0, 0);
    }
}

#endif