#ifndef BLINN_PHONG_HLSLI
#define BLINN_PHONG_HLSLI

#include "Lights.hlsli"

#pragma pack_matrix(row_major)

cbuffer matrices : register(b0) {
	float4x4 viewProjMat;
    float4x4 modelMat;
    float3x3 normalMat;
};

cbuffer lights : register(b0) {
	DirectionalLight dirLight;
	bool calcDirLight;
}

struct VS_IN {
	float3 vertexPos : VERTEXPOS;
    float3 vertexNorm : VERTEXNORMAL;
    float2 vertexUV : VERTEXUV;
};

struct VS_OUT {
	float3 worldPos : WORLDPOS;
	float3 normal : NORMAL;
	float4 clipPos : SV_POSITION;
};

VS_OUT vs_main(VS_IN vs_in)
{
	VS_OUT vsOut;
	float4 worldPos4 = mul(float4(vs_in.vertexPos, 1), modelMat);
	vsOut.worldPos = worldPos4.xyz;
	vsOut.clipPos = mul(worldPos4, viewProjMat);
    vsOut.normal = mul(vs_in.vertexNorm, normalMat);
	return vsOut;
}

float4 ps_main(VS_OUT psIn) : SV_TARGET
{
    float4 ret = float4(0.05, 0.05, 0.05, 1);
	
	if (calcDirLight) {
        float3 dirToLight = normalize(-dirLight.direction);
        float diffuse = saturate(dot(dirToLight, normalize(psIn.normal)));
        ret.xyz += diffuse * dirLight.color * dirLight.intensity;
	}

    return ret;
}

#endif