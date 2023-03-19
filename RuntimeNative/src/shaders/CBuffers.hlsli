#ifndef CBUFFERS_HLSLI
#define CBUFFERS_HLSLI

#include "PbrMaterial.hlsli"
#include "ShaderInterop.h"

cbuffer PerFrame : register(b0) {
    
}

cbuffer PerCamera : register(b1) {
    row_major float4x4 viewProjMat;
    float3 camPos;
    int lightCount;
    ShaderLight lights[MAX_LIGHT_COUNT];
};

cbuffer PerModel : register(b2)
{
    row_major float4x4 modelMat;
    row_major float3x3 normalMat;
};

cbuffer PerMaterial : register(b3) {
    PbrMaterial material;
};

#endif