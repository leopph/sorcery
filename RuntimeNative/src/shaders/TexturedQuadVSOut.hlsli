#ifndef TEXTURED_QUAD_VS_OUT
#define TEXTURED_QUAD_VS_OUT

struct VsOut {
    float4 posClip : SV_POSITION;
    float2 uv : TEXCOORD;
};

#endif