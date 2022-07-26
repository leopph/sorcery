//? #version 450 core

#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL

// Keep in sync with C++ definitions

#define NUM_MAX_CASCADE 4
#define NUM_MAX_SPOT 8
#define NUM_MAX_POINT 8

#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.01


struct Fragment
{
	vec3 pos;
	vec3 normal;
	vec3 diff;
	vec3 spec;
	float gloss;
};


struct Light
{
	vec3 color;
	float intensity;
};


struct DirLight
{
	Light light;
	vec3 direction;
	bool shadow;
};


struct SpotLight
{
	Light light;
	vec3 position;
	float range;
	vec3 direction;
	float innerCos;
	float outerCos;
};


struct PointLight
{
	Light light;
	vec3 position;
	float range;
};


struct DirShadowCascade
{
	mat4 worldToClipMat;
	float nearZ;
	float farZ;
};



layout(std140, binding = 1) uniform LightingBuffer
{
	vec3 ambient;

	bool existsDirLight;
	DirLight dirLight;

	SpotLight spotLights[NUM_MAX_SPOT];
	uint numSpot;

	PointLight pointLights[NUM_MAX_POINT];
	uint numPoint;
} u_Lighting;



vec3 GetAmbientIntensity()
{
	return u_Lighting.ambient;
}



bool ExistsDirLight()
{
	return u_Lighting.existsDirLight;
}



DirLight GetDirLight()
{
	return u_Lighting.dirLight;
}



uint GetNumSpotLights()
{
	return u_Lighting.numSpot;
}



SpotLight GetSpotLight(uint index)
{
	return u_Lighting.spotLights[index];
}



uint GetNumPointLights()
{
	return u_Lighting.numPoint;
}



PointLight GetPointLight(uint index)
{
	return u_Lighting.pointLights[index];
}



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