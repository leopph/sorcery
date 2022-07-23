//? #version 450 core

#ifndef LIGHTING_BUFFER_GLSL
#define LIGHTING_BUFFER_GLSL

#include "Lighting.glsl"


layout(std140, binding = 1) uniform LightingBuffer
{
	vec3 u_CamPos;

	vec3 u_AmbientLight;

	#if DIRLIGHT
	DirLight u_DirLight;
	#endif

	#if NUM_SPOT_NO_SHADOW
	SpotLight u_NonCastingSpotLights[NUM_SPOT_NO_SHADOW];
	#endif

	#if NUM_SPOT_SHADOW
	SpotLight u_CastingSpotLights[NUM_SPOT_SHADOW];
	#endif

	#if NUM_POINT_NO_SHADOW
	PointLight u_NonCastingPointLights[NUM_POINT_NO_SHADOW];
	#endif

	#if NUM_POINT_SHADOW
	PointLight u_CastingPointLights[NUM_POINT_SHADOW];
	#endif
};

#endif