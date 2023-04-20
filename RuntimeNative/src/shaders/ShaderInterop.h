#ifndef SHADER_INTEROP_H
#define SHADER_INTEROP_H


#ifdef __cplusplus

#include "../Math.hpp"


namespace leopph {
using float2 = Vector2;
using float3 = Vector3;
using float4 = Vector4;

using float3x3 = Matrix4;
using float4x4 = Matrix4;
using uint = unsigned;

#define row_major
#define CBUFFER(name, slot) struct name

#else

typedef bool BOOL;
#define CBUFFER(name, slot) cbuffer name : register(b##slot)
#define TEXTURE2D(name, type, slot) Texture2D<type> name : register(t##slot)
#define TEXTURECUBE(name, type, slot) TextureCube<type> name : register(t##slot)
#define SAMPLERCOMPARISONSTATE(name, slot) SamplerComparisonState name : register(s##slot)
#define SAMPLERSTATE(name, slot) SamplerState name : register(s##slot)
#define STRUCTUREDBUFFER(name, type, slot) StructuredBuffer<type> name : register(t##slot)
#endif


#define MAX_LIGHT_COUNT 128

#define CB_SLOT_PER_FRAME 0
#define CB_SLOT_PER_CAM 1
#define CB_SLOT_PER_MODEL 2
#define CB_SLOT_PER_MATERIAL 3
#define CB_SLOT_TONE_MAP_GAMMA 0
#define CB_SLOT_SKYBOX_PASS 0
#define CB_SLOT_SHADOW_PASS 0

#define RES_SLOT_ALBEDO_MAP 0
#define RES_SLOT_METALLIC_MAP 1
#define RES_SLOT_ROUGHNESS_MAP 2
#define RES_SLOT_AO_MAP 3
#define RES_SLOT_PUNCTUAL_SHADOW_ATLAS 4
#define RES_SLOT_DIR_SHADOW_ATLAS 5
#define RES_SLOT_LIGHTS 6
#define RES_SLOT_TONE_MAP_SRC 0
#define RES_SLOT_SKYBOX_CUBEMAP 0
#define RES_SLOT_LINE_GIZMO_VERTEX 1
#define RES_SLOT_GIZMO_COLOR 0

#define SAMPLER_SLOT_MATERIAL 0
#define SAMPLER_SLOT_SHADOW 1
#define SAMPLER_SLOT_SKYBOX_CUBEMAP 0


struct ShaderLight {
	row_major float4x4 shadowViewProjMatrices[6];

	float3 color;
	float intensity;

	float3 direction;
	int type;

	BOOL isCastingShadow;
	float range;
	float innerAngleCos;
	float outerAngleCos;

	float2 shadowUvOffsets[6];

	float shadowUvScales[6];
	BOOL sampleCascade[6];

	float3 position;
	float pad;
};


struct ShaderMaterial {
	float3 albedo;
	float metallic;

	float roughness;
	float ao;
	BOOL sampleAlbedo;
	BOOL sampleMetallic;

	BOOL sampleRoughness;
	BOOL sampleAo;
};


struct ShaderLineGizmoVertexData {
	float3 from;
	uint colorIdx;
	float3 to;
	float pad;
};


CBUFFER(PerFrameCB, CB_SLOT_PER_FRAME) {};


CBUFFER(PerCameraCB, CB_SLOT_PER_CAM) {
row_major float4x4 viewProjMtx;
float3 camPos;
int lightCount;
};


CBUFFER(PerModelCB, CB_SLOT_PER_MODEL) {
row_major float4x4 modelMtx;
row_major float3x3 normalMtx;
};


CBUFFER(PerMaterialCB, CB_SLOT_PER_MATERIAL) {
ShaderMaterial material;
};


CBUFFER(ToneMapGammaCB, CB_SLOT_TONE_MAP_GAMMA) {
float invGamma;
};


CBUFFER(SkyboxCB, CB_SLOT_SKYBOX_PASS) {
row_major float4x4 skyboxViewProjMtx;
};


CBUFFER(ShadowCB, CB_SLOT_SHADOW_PASS) {
row_major float4x4 shadowViewProjMtx;
float shadowNormalBias;
float3 pad;
};


#ifdef __cplusplus
} // namespace leopph
#endif

#endif
