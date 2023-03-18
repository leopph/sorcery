#include "CBuffers.hlsli"

cbuffer LightVPMtx : register(b4) {
    row_major float4x4 lightVPMtx;
}

float4 main(float3 pos : POSITION) : SV_Position {
    return mul(mul(float4(pos, 1), modelMat), lightVPMtx);

}