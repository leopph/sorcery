#include "ShaderInterop.h"

float4 main(const float3 pos : POSITION) : SV_Position {
    return mul(mul(float4(pos, 1), gPerDrawConstants.modelMtx), shadowViewProjMtx);
}