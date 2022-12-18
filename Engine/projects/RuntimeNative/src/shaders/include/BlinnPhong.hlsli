#ifndef BLINN_PHONG_HLSLI
#define BLINN_PHONG_HLSLI

#pragma pack_matrix(row_major)

struct Light {
	float3 color;
	float intensity;
};

struct DirectionalLight : Light {
	float3 direction;
};

cbuffer matrices : register(b0) {
	float4x4 viewProj;
};

cbuffer lights : register(b0) {
	DirectionalLight dirLight;
	bool calcDirLight;
}

struct VS_IN {
	float3 vertexPos : VERTEXPOS;
	float4x4 modelMat : MODELMATRIX;
};

struct VS_OUT {
	float3 worldPos : TEXCOORD0;
	float3 normal : NORMAL;
	float4 clipPos : SV_POSITION;
};

VS_OUT vs_main(VS_IN vs_in)
{
	VS_OUT vsOut;
	float4 worldPos4 = mul(float4(vs_in.vertexPos, 1), vs_in.modelMat);
	vsOut.worldPos = worldPos4.xyz;
	vsOut.clipPos = mul(worldPos4, viewProj);
    vsOut.normal = normalize(vs_in.vertexPos);
	return vsOut;
}

float4 ps_main(VS_OUT psIn) : SV_TARGET
{
	if (calcDirLight) {
        float3 dirToLight = normalize(-dirLight.direction);
        float diffuse = saturate(dot(dirToLight, normalize(psIn.normal)));
		return float4(diffuse * dirLight.color * dirLight.intensity, 1);
	}

	return float4(1, 1, 1, 1);
}

#endif