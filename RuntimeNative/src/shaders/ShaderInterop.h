#ifndef SHADER_INTEROP_H
#define SHADER_INTEROP_H

#define MAX_LIGHT_COUNT 128

#ifdef __cplusplus
#include "../Math.hpp"

namespace leopph {
using float2 = Vector2;
using float3 = Vector3;
using float4 = Vector4;

using float4x4 = Matrix4;
using uint = unsigned;

#define row_major
#endif

struct ShaderLight {
	float3 color;
	float intensity;

	float3 direction;
	int type;

	float shadowNearPlane;
	float range;
	float innerAngleCos;
	bool isCastingShadow;

	float3 position;
	float outerAngleCos;

	row_major float4x4 lightViewProjMtx;

	uint atlasQuadrantIdx;
	uint atlasCellIdx;
};


#ifdef __cplusplus
} // namespace leopph
#endif

#endif
