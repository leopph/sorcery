#pragma pack_matrix(row_major)

cbuffer matrices : register(b0)
{
	float4x4 viewProj;
};

struct VS_IN
{
	float3 vertexPos : VERTEXPOS;
	float4x4 modelMat : MODELMATRIX;
};

float4 vs_main(VS_IN vs_in) : SV_POSITION
{
	return mul(float4(vs_in.vertexPos, 1), mul(vs_in.modelMat, viewProj));
}

float4 ps_main() : SV_TARGET
{
	return float4(1, 1, 1, 1);
}