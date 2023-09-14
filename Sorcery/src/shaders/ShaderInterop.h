#ifndef SHADER_INTEROP_H
#define SHADER_INTEROP_H


#ifdef __cplusplus

#include "../Math.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>


namespace sorcery {
using float2 = Vector2;
using float3 = Vector3;
using float4 = Vector4;

using float4x4 = Matrix4;
using uint = unsigned;

#define row_major
#define CBUFFER_BEGIN(name, slot) struct name {
#define CBUFFER_END };

#else

typedef bool BOOL;
#define CBUFFER_BEGIN(name, slot) cbuffer name : register(b##slot) {
#define CBUFFER_END }
#define TEXTURE2D(name, type, slot) Texture2D<type> name : register(t##slot)
#define TEXTURE2DARRAY(name, type, slot) Texture2DArray<type> name : register(t##slot)
#define TEXTURECUBE(name, type, slot) TextureCube<type> name : register(t##slot)
#define SAMPLERCOMPARISONSTATE(name, slot) SamplerComparisonState name : register(s##slot)
#define SAMPLERSTATE(name, slot) SamplerState name : register(s##slot)
#define STRUCTUREDBUFFER(name, type, slot) StructuredBuffer<type> name : register(t##slot)
#endif

#define CB_SLOT_PER_FRAME 0
#define CB_SLOT_PER_VIEW 1
#define CB_SLOT_PER_DRAW 2
#define CB_SLOT_PER_MATERIAL 3
#define CB_SLOT_POST_PROCESS 4
#define CB_SLOT_SSAO 4

#define RES_SLOT_ALBEDO_MAP 0
#define RES_SLOT_METALLIC_MAP 1
#define RES_SLOT_ROUGHNESS_MAP 2
#define RES_SLOT_AO_MAP 3
#define RES_SLOT_NORMAL_MAP 4
#define RES_SLOT_OPACITY_MASK 5
#define RES_SLOT_PUNCTUAL_SHADOW_ATLAS 6
#define RES_SLOT_DIR_SHADOW_ATLAS 7
#define RES_SLOT_DIR_SHADOW_MAP_ARRAY 7
#define RES_SLOT_LIGHTS 8
#define RES_SLOT_POST_PROCESS_SRC 0
#define RES_SLOT_SKYBOX_CUBEMAP 0
#define RES_SLOT_LINE_GIZMO_VERTEX 1
#define RES_SLOT_GIZMO_COLOR 0
#define RES_SLOT_SSAO_DEPTH 6
#define RES_SLOT_SSAO_NORMAL 7
#define RES_SLOT_SSAO_NOISE 8
#define RES_SLOT_SSAO_SAMPLES 9

#define SAMPLER_SLOT_CMP_PCF 0
#define SAMPLER_SLOT_CMP_POINT 1
#define SAMPLER_SLOT_AF16 2
#define SAMPLER_SLOT_AF8 3
#define SAMPLER_SLOT_AF4 4
#define SAMPLER_SLOT_TRI 5
#define SAMPLER_SLOT_BI 6
#define SAMPLER_SLOT_POINT 7

#define MAX_CASCADE_COUNT 4
#define MAX_PER_LIGHT_SHADOW_MAP_COUNT 6

#define BLEND_MODE_OPAQUE 0
#define BLEND_MODE_ALPHA_CLIP 1


struct ShaderLight {
  row_major float4x4 shadowViewProjMatrices[MAX_PER_LIGHT_SHADOW_MAP_COUNT];

  float3 color;
  float intensity;

  float3 direction;
  int type;

  BOOL isCastingShadow;
  float range;
  float halfInnerAngleCos;
  float halfOuterAngleCos;

  float2 shadowAtlasCellOffsets[MAX_PER_LIGHT_SHADOW_MAP_COUNT];

  float shadowAtlasCellSizes[MAX_PER_LIGHT_SHADOW_MAP_COUNT];
  BOOL sampleShadowMap[MAX_PER_LIGHT_SHADOW_MAP_COUNT];

  float3 position;
  float depthBias;

  float normalBias;
  float3 pad;
};


struct ShaderMaterial {
  float3 albedo;
  float metallic;

  float roughness;
  float ao;
  float alphaThreshold;
  BOOL sampleAlbedo;

  BOOL sampleMetallic;
  BOOL sampleRoughness;
  BOOL sampleAo;
  BOOL sampleNormal;

  BOOL sampleOpacityMap;
  int blendMode;
  float2 pad;
};


struct ShaderLineGizmoVertexData {
  float3 from;
  uint colorIdx;
  float3 to;
  float pad;
};


struct ShaderPerFrameConstants {
  float3 ambientLightColor;
  int shadowCascadeCount;
  float2 screenSize;
  BOOL visualizeShadowCascades;
  int shadowFilteringMode;
  BOOL isUsingReversedZ;
  float3 pad;
};


struct ShaderPerViewConstants {
  row_major float4x4 viewMtx;
  row_major float4x4 projMtx;
  row_major float4x4 invProjMtx;
  row_major float4x4 viewProjMtx;
  float4 shadowCascadeSplitDistances;
  float3 viewPos;
  float pad;
};


struct ShaderPerDrawConstants {
  row_major float4x4 modelMtx;
  row_major float4x4 invTranspModelMtx;
};


struct ShaderSsaoConstants {
  float radius;
  float bias;
  float2 pad;
};


CBUFFER_BEGIN(PerFrameCB, CB_SLOT_PER_FRAME)
  ShaderPerFrameConstants gPerFrameConstants;
CBUFFER_END


CBUFFER_BEGIN(PerViewCB, CB_SLOT_PER_VIEW)
  ShaderPerViewConstants gPerViewConstants;
CBUFFER_END


CBUFFER_BEGIN(PerDrawCB, CB_SLOT_PER_DRAW)
  ShaderPerDrawConstants gPerDrawConstants;
CBUFFER_END


CBUFFER_BEGIN(PerMaterialCB, CB_SLOT_PER_MATERIAL)
  ShaderMaterial material;
CBUFFER_END


CBUFFER_BEGIN(PostProcessCB, CB_SLOT_POST_PROCESS)
  float invGamma;
  float3 pad;
CBUFFER_END


CBUFFER_BEGIN(SsaoCB, CB_SLOT_SSAO)
  ShaderSsaoConstants gSsaoConstants;
CBUFFER_END


#ifdef __cplusplus
} // namespace sorcery
#endif

#endif
