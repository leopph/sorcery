#include "ShaderInterop.h"
#include "GizmoVsOut.hlsli"

STRUCTUREDBUFFER(ShaderLineGizmoVertexSB, ShaderLineGizmoVertexData, SB_SLOT_LINE_GIZMO_VERTEX);


GizmoVsOut main(const uint instanceId : SV_InstanceID, const uint vertexId : SV_VertexID) {
    const ShaderLineGizmoVertexData data = ShaderLineGizmoVertexSB[instanceId];
    GizmoVsOut ret;
    ret.colorIdx = data.colorIdx;
    ret.position = mul(float4(vertexId == 0 ? data.from : data.to, 1), viewProjMtx);
    return ret;
}