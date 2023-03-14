#ifndef LIGHTS_HLSLI
#define LIGHTS_HLSLI

struct Light {
    float3 color;
    float intensity;
    int type;
    float3 direction;
    int isCastingShadow;
    float shadowNearPlane;
    float range;
    float innerAngle;
    float outerAngle;
};

#endif