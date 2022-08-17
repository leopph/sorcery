//? #version 450 core

#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL

#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.01


struct Fragment
{
	vec3 pos;
	vec3 normal;
	vec3 diff;
	vec3 spec;
	float alpha;
	float gloss;
};

// Keep in sync with C++

const int LIGHT_TYPE_DIR = 0;
const int LIGHT_TYPE_SPOT = 1;
const int LIGHT_TYPE_POINT = 2;
const int NUM_MAX_LIGHTS = 8;


struct Light
{
	vec3 color;
	float intensity;
	vec3 position;
	float range;
	vec3 direction;
	float innerCos;
	float outerCos;
	int type;
};

layout(std140, binding = 0) uniform PerFrameData
{
	vec3 uAmbientIntensity;
	int uNumLights;
	Light uLights[NUM_MAX_LIGHTS];
};


struct DirShadowCascade
{
	mat4 worldToClipMat;
	float nearZ;
	float farZ;
};




#if NUM_DIRLIGHT_SHADOW_CASCADE || NUM_SPOT_SHADOW || NUM_POINT_SHADOW
float ShadowBias(vec3 dirToLight, vec3 fragNormal)
{
	return max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, dirToLight)), MIN_SHADOW_BIAS);
}
#endif



#if NUM_DIRLIGHT_SHADOW_CASCADE
float DirShadow(Fragment frag, DirLight light, float fragPosNdcZ, DirShadowCascade cascades[NUM_DIRLIGHT_SHADOW_CASCADE], sampler2DShadow shadowMaps[NUM_DIRLIGHT_SHADOW_CASCADE])
{
	for (int i = 0; i < NUM_DIRLIGHT_SHADOW_CASCADE; i++)
	{
		if (fragPosNdcZ < cascades[i].farZ)
		{
			vec3 fragPosLightNorm = vec3(vec4(frag.pos, 1) * cascades[i].worldToClipMat) * 0.5 + 0.5;
			float bias = ShadowBias(-light.direction, frag.normal);
			return texture(shadowMaps[i], vec3(fragPosLightNorm.xy, fragPosLightNorm.z - bias));
		}
	}
	return 1.0;
}
#endif



#if NUM_SPOT_SHADOW
float SpotShadow(Fragment frag, SpotLight light, mat4 lightWordToClip, sampler2DShadow shadowMap)
{
	vec4 fragPosLightSpace = vec4(frag.pos, 1) * lightWordToClip;
	vec3 normalizedPos = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
	vec3 dirToLight = normalize(light.position - frag.pos);
	float bias = ShadowBias(dirToLight, frag.normal);
	return texture(shadowMap, vec3(normalizedPos.xy, normalizedPos.z - bias));
}
#endif



#if NUM_POINT_SHADOW
float PointShadow(Fragment frag, PointLight light, samplerCube shadowMap)
{
	vec3 dirToFrag = frag.pos - light.position;
	float bias = ShadowBias(normalize(-dirToFrag), frag.normal);
	return texture(shadowMap, dirToFrag).r > length(dirToFrag) - bias ? 1 : 0;
}
#endif



#if NUM_DIRLIGHT_SHADOW_CASCADE
uniform DirShadowCascade u_DirShadowCascades[NUM_DIRLIGHT_SHADOW_CASCADE];
uniform sampler2DShadow u_DirShadowMaps[NUM_DIRLIGHT_SHADOW_CASCADE];
#endif



#if NUM_SPOT_SHADOW
uniform mat4 u_SpotShadowWorldToClipMats[NUM_SPOT_SHADOW];
uniform sampler2DShadow u_SpotShadowMaps[NUM_SPOT_SHADOW];
#endif



#if NUM_POINT_SHADOW
uniform samplerCube u_PointShadowMaps[NUM_POINT_SHADOW];
#endif

#endif