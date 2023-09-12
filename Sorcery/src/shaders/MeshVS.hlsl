#include "ShaderInterop.h"
#include "MeshVSOut.hlsli"

struct MeshVSIn {
  float3 positionOS : POSITION;
  float3 normalOS : NORMAL;
  float2 uv : TEXCOORD;
  float3 tangentOS : TANGENT;
};

MeshVsOut main(MeshVSIn vsIn) {
  const float4 positionWS = mul(float4(vsIn.positionOS, 1), gPerDrawConstants.modelMtx);
  const float4 positionVS = mul(positionWS, gPerViewConstants.viewMtx);
  const float4 positionCS = mul(positionVS, gPerViewConstants.projMtx);

  const float3 normalWS = normalize(mul(vsIn.normalOS, (float3x3)gPerDrawConstants.invTranspModelMtx));
  float3 tangentWS = normalize(mul(vsIn.tangentOS, (float3x3)gPerDrawConstants.invTranspModelMtx));
  tangentWS = normalize(tangentWS - dot(tangentWS, normalWS) * normalWS);
  const float3 bitangentWS = cross(normalWS, tangentWS);
  const float3x3 tbnMtxWS = float3x3(tangentWS, bitangentWS, normalWS);

  MeshVsOut ret;
  ret.positionWS = positionWS.xyz;
  ret.positionVS = positionVS.xyz;
  ret.positionCS = positionCS;
  ret.normalWS = normalWS;
  ret.uv = vsIn.uv;
  ret.tbnMtxWS = tbnMtxWS;

  return ret;
}