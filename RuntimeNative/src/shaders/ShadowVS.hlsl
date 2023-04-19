#include "ShaderInterop.h"

float4 main(const float3 pos : POSITION, const float3 normal : NORMAL) : SV_Position {
    return mul(mul(float4(pos - normal * shadowNormalBias, 1), modelMtx), shadowViewProjMtx);
}