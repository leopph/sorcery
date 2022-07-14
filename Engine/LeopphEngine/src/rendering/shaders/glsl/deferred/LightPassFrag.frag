#version 420 core


//! #define DIRLIGHT
//! #define SHADOW
//! #define NUM_CASCADES 3


struct Fragment
{
	vec3 pos;
	vec3 normal;
	vec3 diff;
	vec3 spec;
	float gloss;
};


#ifdef DIRLIGHT
struct DirLight
{
	vec3 direction;
	vec3 diffuseColor;
	vec3 specularColor;
};
#endif


#ifdef SPOTLIGHT
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
#endif


#ifdef POINTLIGHT
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
#endif


#if defined DIRLIGHT && defined SHADOW
struct ShadowCascade
{
	mat4 worldToClipMat;
	float nearZ;
	float farZ;
};
#endif



layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(location = 0) out vec3 out_FragColor;


layout(binding = 0) uniform usampler2D u_NormColorGlossTex;
layout(binding = 1) uniform sampler2D u_DepthTex;


layout(std140, binding = 0) uniform Commons
{
	layout(row_major) mat4 u_CamViewProjInv;
	vec3 u_CamPos;
};


#ifdef AMBIENTLIGHT
uniform vec3 u_Light;

#elif defined DIRLIGHT
uniform DirLight u_Light;

#elif defined SPOTLIGHT
uniform SpotLight u_Light;

#elif defined POINTLIGHT
uniform PointLight u_Light;

#endif


#ifdef SHADOW
#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.01

#ifdef DIRLIGHT
uniform ShadowCascade u_ShadowCascades[NUM_CASCADES];
layout(binding = 2) uniform sampler2DShadow u_ShadowMaps[NUM_CASCADES];

#elif defined SPOTLIGHT
uniform mat4 u_ShadowWorldToClipMat;
layout(binding = 2) uniform sampler2DShadow u_ShadowMap;

#elif defined POINTLIGHT
layout(binding = 2) uniform samplerCube u_ShadowMap;

#endif
#endif



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



float CalcAtten(float constant, float linear, float quadratic, float dist)
{
	return 1.0 / (constant + linear * dist + quadratic * pow(dist, 2));
}



#ifdef SHADOW
float ShadowBias(vec3 dirToLight, vec3 fragNormal)
{
	return max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, dirToLight)), MIN_SHADOW_BIAS);
}
#endif



#if defined DIRLIGHT && defined SHADOW
float CalcDirShadow(vec3 fragPos, float fragPosNdcZ, vec3 fragNormal)
{
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		if (fragPosNdcZ < u_ShadowCascades[i].farZ)
		{
			vec3 fragPosLightNorm = vec3(vec4(fragPos, 1) * u_ShadowCascades[i].worldToClipMat) * 0.5 + 0.5;
			float bias = ShadowBias(-u_Light.direction, fragNormal);
			return texture(u_ShadowMaps[i], vec3(fragPosLightNorm.xy, fragPosLightNorm.z - bias));
		}
	}
	return 1.0;
}
#endif



#ifdef SPOTLIGHT
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



#ifdef SHADOW
float CalcSpotShadow(vec3 dirToLight, vec3 fragPos, vec3 fragNormal)
{
	vec4 fragPosLightSpace = vec4(fragPos, 1) * u_ShadowWorldToClipMat;
	vec3 normalizedPos = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
	float bias = ShadowBias(dirToLight, fragNormal);
	return texture(u_ShadowMap, vec3(normalizedPos.xy, normalizedPos.z - bias));
}
#endif
#endif



#ifdef POINTLIGHT
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



#ifdef SHADOW
float CalcPointShadow(vec3 fragPos, vec3 fragNormal, vec3 lightPos, float lightRange)
{
	vec3 dirToFrag = fragPos - lightPos;
	float bias = ShadowBias(normalize(-dirToFrag), fragNormal);
	return texture(u_ShadowMap, dirToFrag).r > length(dirToFrag) - bias ? 1 : 0;
}
#endif
#endif



void main()
{	
	// Parse gbuffer contents
	Fragment frag;

	// Reconstruct pos from depth
	vec4 fragPosNdc = vec4(gl_FragCoord.xy / textureSize(u_DepthTex, 0).xy * 2 - 1, texelFetch(u_DepthTex, ivec2(gl_FragCoord.xy), 0).r * 2 - 1, 1);
	vec4 fragPosWorld = fragPosNdc * u_CamViewProjInv;
	fragPosWorld /= fragPosWorld.w;
	frag.pos = fragPosWorld.xyz;

	uvec3 packedNormColorGloss = texelFetch(u_NormColorGlossTex, ivec2(gl_FragCoord.xy), 0).rgb;

	// Decode normal
	vec2 comprNorm = unpackSnorm2x16(packedNormColorGloss.x);
	frag.normal = vec3(comprNorm.xy, 1 - abs(comprNorm.x) - abs(comprNorm.y));
	frag.normal.xy = frag.normal.z < 0 ? (1 - abs(frag.normal.yx)) * vec2(frag.normal.x >= 0 ? 1 : -1, frag.normal.y >= 0 ? 1 : -1) : frag.normal.xy;
	frag.normal = normalize(frag.normal);

	// Unpack color and gloss bits
	vec4 unPack = unpackUnorm4x8(packedNormColorGloss.y);
	frag.diff = unPack.xyz;
	frag.spec.r = unPack.w;
	frag.spec.gb = unpackUnorm4x8(packedNormColorGloss.z).xy;
	frag.gloss = unpackHalf2x16(packedNormColorGloss.z).y;

	vec3 color;

	#if defined AMBIENTLIGHT
	color = u_Light * frag.diff;

	#elif defined DIRLIGHT
	color = CalcBlinnPhong(frag, -u_Light.direction, u_Light.diffuseColor, u_Light.specularColor);

	#if defined SHADOW
	color *= CalcDirShadow(frag.pos, fragPosNdc.z, frag.normal);
	#endif

	#elif defined SPOTLIGHT
	color = CalcSpotLightEffect(frag, u_Light);

	#if defined SHADOW
	color *= CalcSpotShadow(normalize(u_Light.position - frag.pos), frag.pos, frag.normal);
	#endif

	#elif defined POINTLIGHT
	color = CalcPointLightEffect(frag, u_Light);

	#if defined SHADOW
	color *= CalcPointShadow(frag.pos, frag.normal, u_Light.position, u_Light.range);
	#endif

	#endif

	out_FragColor = color;
}