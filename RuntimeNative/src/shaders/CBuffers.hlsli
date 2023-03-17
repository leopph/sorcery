#ifndef CBUFFERS_HLSLI
#define CBUFFERS_HLSLI

#define MAX_LIGHT_COUNT 128

#include "Light.hlsli"
#include "PbrMaterial.hlsli"

cbuffer PerFrame : register(b0) {
    Light lights[MAX_LIGHT_COUNT];
    int lightCount;
}

cbuffer PerCamera : register(b1) {
    row_major float4x4 viewProjMat;
    float3 camPos;
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