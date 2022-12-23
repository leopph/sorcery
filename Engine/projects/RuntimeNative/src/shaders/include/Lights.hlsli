#ifndef LIGHTS_HLSLI
#define LIGHTS_HLSLI

struct Light {
    float3 color;
    float intensity;
};

struct DirectionalLight : Light {
    float3 direction;
};

#endif