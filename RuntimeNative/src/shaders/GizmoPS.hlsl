#include "GizmoVsOut.hlsli"
#include "ShaderInterop.h"

STRUCTUREDBUFFER(GizmoColorSB, float4, RES_SLOT_GIZMO_COLOR);

float4 main(const GizmoVsOut input) : SV_TARGET
{
	return float4(GizmoColorSB[input.colorIdx]);
}