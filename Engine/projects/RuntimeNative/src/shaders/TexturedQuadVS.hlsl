#include "TexturedQuadVSOut.hlsli"

struct VsIn {
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
};

VsOut main(const VsIn vsIn) {
    VsOut vsOut;
    vsOut.posClip = float4(vsIn.pos, 0, 1);
    vsOut.uv = vsIn.uv;
    return vsOut;
}