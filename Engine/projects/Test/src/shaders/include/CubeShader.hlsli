cbuffer matrices : register(b0)
{
	row_major float4x4 viewProj;
};

struct VS_IN
{
	float3 vertexPos : VERTEXPOS;
	float3 objectPos : OBJECTPOS;
};

float4 vs_main(VS_IN vs_in) : SV_POSITION
{
	float4x4 model = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, vs_in.objectPos, 1);
	return mul(float4(vs_in.vertexPos, 1), mul(model, viewProj));
}

float4 ps_main() : SV_TARGET
{
	return float4(1, 1, 1, 1);
}