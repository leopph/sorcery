//? #version 330 core
#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.01
//! #define NUM_CASCADES 3



struct Fragment
{
	vec3 pos;
	vec3 normal;
	vec3 diff;
	vec3 spec;
	float gloss;
};



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



struct DirLight
{
	vec3 direction;
	vec3 diffuseColor;
	vec3 specularColor;
};



struct SpotLight
{
	vec3 position;
	vec3 direction;
	vec3 diffuseColor;
	vec3 specularColor;
	float range;
	float innerCos;
	float outerCos;
};



struct PointLight
{
	vec3 position;
	vec3 diffuseColor;
	vec3 specularColor;
	float range;
};



struct DirShadowCascade
{
	mat4 worldToClipMat;
	float nearZ;
	float farZ;
};



vec3 BlinnPhongEffect(Fragment frag, vec3 dirToLight, vec3 lightDiff, vec3 lightSpec, vec3 camPos)
{
	float diffuse = max(dot(dirToLight, frag.normal), 0);
	vec3 diffEffect = frag.diff * lightDiff * diffuse;

	if (diffuse <= 0)
	{
		return diffEffect;
	}
		
	vec3 dirToCam =  normalize(camPos - frag.pos);
	vec3 halfway = normalize(dirToLight + dirToCam);
	float specAngle = max(dot(frag.normal, halfway), 0);

	// 0^0 is UB, thus it may produce nan or 0 but we have to make sure
	// transparent objects stay lit from the back too
	#ifdef TRANSPARENT
	if (specAngle <= 0 && frag.gloss == 0)
	{
		return diffEffect;
	}
	#endif

	float specular = pow(specAngle, 4 * frag.gloss);
	vec3 specEffect = frag.spec * lightSpec * specular;
	return diffEffect + specEffect;
}


float CalcAtten(float dist, float range)
{
	return clamp(1 - pow(dist, 2) / pow(range, 2), 0, 1);
}



vec3 DirLightEffect(Fragment frag, DirLight dirLight, vec3 camPos)
{
	return BlinnPhongEffect(frag, -dirLight.direction, dirLight.diffuseColor, dirLight.specularColor, camPos);
}



vec3 SpotLightEffect(Fragment frag, SpotLight spotLight, vec3 camPos)
{
	vec3 dirToLight = spotLight.position - frag.pos;
	float lightFragDist = length(dirToLight);

	if (lightFragDist > spotLight.range)
	{
		return vec3(0);
	}

	dirToLight = normalize(dirToLight);
	float lightFragAngleCos = dot(dirToLight, -spotLight.direction);
	float cutOffCosDiff = spotLight.innerCos - spotLight.outerCos;
	float intensity = clamp((lightFragAngleCos - spotLight.outerCos) / cutOffCosDiff, 0, 1);

	if (intensity == 0)
	{
		return vec3(0);
	}

	vec3 effect = BlinnPhongEffect(frag, dirToLight, spotLight.diffuseColor, spotLight.specularColor, camPos);
	effect *= intensity;
	effect *= CalcAtten(lightFragDist, spotLight.range);
	return effect;
}



vec3 PointLightEffect(Fragment frag, PointLight pointLight, vec3 camPos)
{
	vec3 dirToLight = pointLight.position - frag.pos;
	float dist = length(dirToLight);

	if (dist > pointLight.range)
	{
		return vec3(0);
	}

	dirToLight = normalize(dirToLight);

	vec3 effect = BlinnPhongEffect(frag, dirToLight, pointLight.diffuseColor, pointLight.specularColor, camPos);
	effect *= CalcAtten(dist, pointLight.range);
	return effect;
}



float ShadowBias(vec3 dirToLight, vec3 fragNormal)
{
	return max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, dirToLight)), MIN_SHADOW_BIAS);
}



float DirShadow(Fragment frag, DirLight light, float fragZNdc, DirShadowCascade[NUM_CASCADES] cascades, sampler2DShadow shadowMaps[NUM_CASCADES])
{
	for (int i = 0; i < NUM_CASCADES; ++i)
	{
		if (fragZNdc < cascades[i].farZ)
		{
			vec3 fragPosLightNorm = vec3(vec4(frag.pos, 1) * cascades[i].worldToClipMat) * 0.5 + 0.5;
			float bias = ShadowBias(-light.direction, frag.normal);
			return texture(shadowMaps[i], vec3(fragPosLightNorm.xy, fragPosLightNorm.z - bias));
		}
	}
	return 1.0;
}



float SpotShadow(Fragment frag, SpotLight light, mat4 lightWordToClip, sampler2DShadow shadowMap)
{
	vec4 fragPosLightSpace = vec4(frag.pos, 1) * lightWordToClip;
	vec3 normalizedPos = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
	vec3 dirToLight = normalize(light.position - frag.pos);
	float bias = ShadowBias(dirToLight, frag.normal);
	return texture(shadowMap, vec3(normalizedPos.xy, normalizedPos.z - bias));
}



float PointShadow(Fragment frag, PointLight light, samplerCube shadowMap)
{
	vec3 dirToFrag = frag.pos - light.position;
	float bias = ShadowBias(normalize(-dirToFrag), frag.normal);
	return texture(shadowMap, dirToFrag).r > length(dirToFrag) - bias ? 1 : 0;
}