#include "GizmoVsOut.hlsli"

struct DrawParams {
  uint color_buf_idx;
};

ConstantBuffer<DrawParams> g_draw_params : register(b0, space0);

float4 main(const GizmoVsOut input) : SV_Target {
  const StructuredBuffer<float4> colors = ResourceDescriptorHeap[g_draw_params.color_buf_idx];
	return float4(colors[input.colorIdx]);
}
