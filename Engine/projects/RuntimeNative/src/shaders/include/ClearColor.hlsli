#ifndef CLEAR_COLOR_HLSLI
#define CLEAR_COLOR_HLSLI

cbuffer color : register(b0) {
    float4 clearColor;
}

float4 VsMain(float2 pos : VertexPos) : SV_Position {
    return float4(pos, 0, 1);

}

float4 PsMain() : SV_Target {
    return clearColor;
}

#endif