#version 410 core

#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.01

layout (location = 0) in vec3 in_FragPos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;


#if TRANSPARENT
layout (location = 0) out vec4 out_Accum;
layout (location = 1) out float out_Reveal;
#else
layout (location = 0) out vec4 out_FragColor;
#endif

struct Material
{
	vec3 diffuseColor;
	vec3 specularColor;

	float gloss;
	float opacity;

	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D opacityMap;

	bool hasDiffuseMap;
	bool hasSpecularMap;
	bool hasOpacityMap;
};

uniform Material u_Material;
uniform vec3 u_AmbientLight;
uniform vec3 u_CamPos;


struct Fragment
{
	vec3 pos;
	vec3 normal;
	vec3 diff;
	vec3 spec;
	float gloss;
};


// General functions

float CalcAtten(float constant, float linear, float quadratic, float dist)
{
	return 1.0 / (constant + linear * dist + quadratic * pow(dist, 2));
}

vec3 CalcBlinnPhong(Fragment frag, vec3 dirToLight, vec3 lightDiff, vec3 lightSpec)
{
	float diffuseDot = max(dot(dirToLight, frag.normal), 0);
	vec3 light = frag.diff * diffuseDot * lightDiff;

	if (diffuseDot > 0)
	{
		vec3 halfway = normalize(dirToLight + normalize(u_CamPos - frag.pos));
		light += frag.spec * pow(max(dot(frag.normal, halfway), 0), 4 * frag.gloss) * lightSpec;
	}

	return light;
}


// Only when there is a DirectionalLight
#if DIRLIGHT
struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;
};

uniform DirLight u_DirLight;

#if DIRLIGHT_SHADOW
layout (location = 3) in float in_FragPosNdcZ;
uniform mat4 u_CascadeMatrices[NUM_CASCADES];
uniform float u_CascadeBoundsNdc[NUM_CASCADES];
uniform sampler2DShadow u_DirShadowMaps[NUM_CASCADES];

float CalcDirShadow(vec3 fragPos, float fragPosNdcZ, vec3 fragNormal)
{
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		if (fragPosNdcZ < u_CascadeBoundsNdc[i])
		{
			vec3 fragPosLightNorm = vec3(vec4(fragPos, 1) * u_CascadeMatrices[i]) * 0.5 + 0.5;
			float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, -u_DirLight.direction)), MIN_SHADOW_BIAS);
			return texture(u_DirShadowMaps[i], vec3(fragPosLightNorm.xy, fragPosLightNorm.z - bias));
		}
	}
	return 1.0;
}
#endif
#endif


// Only when there are SpotLights
#if NUM_SPOTLIGHTS > 0
struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
	float range;
	
	float innerAngleCosine;
	float outerAngleCosine;
};

vec3 CalcSpotLightEffect(Fragment frag, SpotLight spotLight)
{
	vec3 dirToLight = spotLight.position - frag.pos;
	float dist = length(dirToLight);

	if (dist > spotLight.range)
	{
		return vec3(0);
	}

	dirToLight = normalize(dirToLight);

	/* Let theta be the angle between
	 * the direction vector pointing from the fragment to the light
	 * and the reverse of the light's direction vector. */
	float thetaCosine = dot(dirToLight, -spotLight.direction);

	/* Let epsilon be the difference between
	 * the cosines of the light's cutoff angles. */
	float epsilon = spotLight.innerAngleCosine - spotLight.outerAngleCosine;

	/* Determine if the frag is
	 * inside the inner angle, or
	 * between the inner and outer angles, or
	 * outside the outer angle. */
	float intensity = clamp((thetaCosine - spotLight.outerAngleCosine) / epsilon, 0.0, 1.0);

	if (intensity == 0)
	{
		return vec3(0);
	}

	vec3 spotLightEffect = CalcBlinnPhong(frag, dirToLight, spotLight.diffuseColor, spotLight.specularColor);
	spotLightEffect *= CalcAtten(spotLight.constant, spotLight.linear, spotLight.quadratic, dist);
	return spotLightEffect;
}

#if NUM_SPOTLIGHTS > NUM_SPOTLIGHT_SHADOWS
uniform SpotLight u_SpotLightsNoShadow[NUM_SPOTLIGHTS - NUM_SPOTLIGHT_SHADOWS];
#endif
#if NUM_SPOTLIGHT_SHADOWS > 0
uniform SpotLight u_SpotLightsShadow[NUM_SPOTLIGHT_SHADOWS];
uniform sampler2DShadow u_SpotShadowMaps[NUM_SPOTLIGHT_SHADOWS];
uniform mat4 u_SpotShadowMats[NUM_SPOTLIGHT_SHADOWS];

float CalcSpotShadow(int shadowIndex, vec3 dirToLight, vec3 fragPos, vec3 fragNormal)
{
	vec4 fragPosLightSpace = vec4(fragPos, 1) * u_SpotShadowMats[shadowIndex];
	vec3 normalizedPos = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
	float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, dirToLight)), MIN_SHADOW_BIAS);
	return texture(u_SpotShadowMaps[shadowIndex], vec3(normalizedPos.xy, normalizedPos.z - bias));
}
#endif
#endif


// Only if there are PointLights
#if NUM_POINTLIGHTS > 0
struct PointLight
{
	vec3 position;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
	float range;
};

vec3 CalcPointLightEffect(Fragment frag, PointLight pointLight)
{
	vec3 dirToLight = pointLight.position - frag.pos;
	float dist = length(dirToLight);

	if (dist > pointLight.range)
	{
		return vec3(0);
	}

	dirToLight = normalize(dirToLight);

	vec3 pointLightEffect = CalcBlinnPhong(frag, dirToLight, pointLight.diffuseColor, pointLight.specularColor);
	pointLightEffect *= CalcAtten(pointLight.constant, pointLight.linear, pointLight.quadratic, dist);
	return pointLightEffect;
}

#if NUM_POINTLIGHTS > NUM_POINTLIGHT_SHADOWS
uniform PointLight u_PointLightsNoShadow[NUM_POINTLIGHTS - NUM_POINTLIGHT_SHADOWS];
#endif
#if NUM_POINTLIGHT_SHADOWS > 0
uniform PointLight u_PointLightsShadow[NUM_POINTLIGHT_SHADOWS];
uniform samplerCube u_PointShadowMaps[NUM_POINTLIGHT_SHADOWS];

float CalcPointShadow(uint shadowIndex, vec3 fragPos, vec3 fragNormal, vec3 lightPos, float lightRange)
{
	vec3 dirToFrag = fragPos - lightPos;
	float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, normalize(-dirToFrag))), MIN_SHADOW_BIAS);
	return texture(u_PointShadowMaps[shadowIndex], dirToFrag).r > length(dirToFrag) - bias ? 1 : 0;
}
#endif
#endif





void main()
{
	Fragment frag;
	frag.pos = in_FragPos;
	frag.normal = normalize(in_Normal);
	frag.diff = u_Material.diffuseColor;
	frag.spec = u_Material.specularColor;
	frag.gloss = u_Material.gloss;
	float alpha = u_Material.opacity;

	if (u_Material.hasDiffuseMap)
	{
		frag.diff = texture(u_Material.diffuseMap, in_TexCoords).rgb;
	}

	if (u_Material.hasSpecularMap)
	{
		frag.spec *= texture(u_Material.specularMap, in_TexCoords).rgb;
	}
	
	if (u_Material.hasOpacityMap)
	{
		alpha *= texture(u_Material.opacityMap, in_TexCoords).r;
	}

	#if TRANSPARENT
	if (alpha >= 1)
	#else
	if (alpha < 1)
	#endif
	{
		discard;
	}

	// Add ambient effect
	vec3 colorSum = frag.diff * u_AmbientLight;

	// Add directional effects
	#if DIRLIGHT
	vec3 dirLightEffect = CalcBlinnPhong(frag, -u_DirLight.direction, u_DirLight.diffuseColor, u_DirLight.specularColor);
	#if DIRLIGHT_SHADOW
	dirLightEffect *= CalcDirShadow(frag.pos, in_FragPosNdcZ, frag.normal);
	#endif
	colorSum += dirLightEffect;
	#endif

	// Add spot effects
	#if NUM_SPOTLIGHTS > NUM_SPOTLIGHT_SHADOWS
	for (int i = 0; i < NUM_SPOTLIGHTS - NUM_SPOTLIGHT_SHADOWS; ++i)
	{
		colorSum += CalcSpotLightEffect(frag, u_SpotLightsNoShadow[i]);
	}
	#endif
	#if NUM_SPOTLIGHT_SHADOWS > 0
	for (int i = 0; i < NUM_SPOTLIGHT_SHADOWS; ++i)
	{
		vec3 spotLightEffect = CalcSpotLightEffect(frag, u_SpotLightsShadow[i]);
		spotLightEffect *= CalcSpotShadow(i, normalize(u_SpotLightsShadow[i].position - frag.pos), frag.pos, frag.normal);
		colorSum += spotLightEffect;
	}
	#endif

	// Add point effects
	#if NUM_POINTLIGHTS > NUM_POINTLIGHT_SHADOWS
	for (int i = 0; i < NUM_POINTLIGHTS - NUM_POINTLIGHT_SHADOWS; ++i)
	{
		colorSum += CalcPointLightEffect(frag, u_PointLightsNoShadow[i]);
	}
	#endif
	#if NUM_POINTLIGHT_SHADOWS > 0
	for (int i = 0; i < NUM_POINTLIGHT_SHADOWS; ++i)
	{
		vec3 pointLightEffect = CalcPointLightEffect(frag, u_PointLightsShadow[i]);
		pointLightEffect *= CalcPointShadow(i, frag.pos, frag.normal, u_PointLightsShadow[i].position, u_PointLightsShadow[i].range);
		colorSum += pointLightEffect;
	}
	#endif

	#if TRANSPARENT
	float weight = max(min(1.0, max(max(colorSum.r, colorSum.g), colorSum.b) * alpha), alpha) * clamp(0.03 / (1e-5 + pow(frag.pos.z / 200, 4.0)), 1e-2, 3e3);
	out_Accum = vec4(colorSum.rgb * alpha, alpha) * weight;
	out_Reveal = alpha;
	#else
	out_FragColor = vec4(colorSum, 1);
	#endif
}