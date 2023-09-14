#include "ScreenVsOut.hlsli"
#include "Util.hlsli"

ScreenVsOut main(const uint vertexId : SV_VertexID) {
	ScreenVsOut ret;
	ret.uv = float2((vertexId << 1) & 2, vertexId & 2);
	ret.positionCS = float4(UvToNdc(ret.uv), 0, 1);
	return ret;
}