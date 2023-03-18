#include "TexturedQuadVSOut.hlsli"

struct VsIn {
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

VsOut main(const VsIn vsIn) {
    VsOut vsOut;
    vsOut.posClip = float4(vsIn.pos, 1);
    vsOut.uv = vsIn.uv;
    return vsOut;
}