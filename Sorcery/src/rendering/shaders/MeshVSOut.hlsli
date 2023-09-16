#ifndef MESH_VS_OUT_HLSLI
#define MESH_VS_OUT_HLSLI

struct MeshVsOut {
  float3 positionWS : POSITIONWS;
  float3 positionVS : POSITIONVS;
  float4 positionCS : SV_POSITION;
  float3 normalWS : NORMAL;
  float2 uv : TEXCOORD;
  float3x3 tbnMtxWS : TBNMTXWS;
};

#endif