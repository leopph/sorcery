#include "ScreenVsOut.hlsli"
#include "utility.hlsli"

ScreenVsOut main(const uint vertex_id : SV_VertexID) {
	ScreenVsOut ret;
	ret.uv = float2((vertex_id << 1) & 2, vertex_id & 2);
	ret.positionCS = float4(UvToNdc(ret.uv), 0, 1);
	return ret;
}
