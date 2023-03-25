#ifndef GIZMO_VS_OUT_HLSLI
#define GIZMO_VS_OUT_HLSLI

struct GizmoVsOut {
    float4 position : SV_Position;
    uint colorIdx : COLORIDX;
};

#endif