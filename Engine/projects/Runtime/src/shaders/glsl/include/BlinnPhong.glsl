//? #version 450 core

#ifndef BLINN_PHONG_GLSL
#define BLINN_PHONG_GLSL

#include "Lighting.glsl"


vec3 BlinnPhongEffect(Fragment frag, vec3 preMulLightColor, vec3 dirToLight, vec3 camPos)
{
	float diffuse = max(dot(dirToLight, frag.normal), 0);
	vec3 diffEffect = frag.diff * preMulLightColor * diffuse;

	if (diffuse <= 0)
	{
		return diffEffect;
	}
		
	vec3 dirToCam =  normalize(camPos - frag.pos);
	vec3 halfway = normalize(dirToLight + dirToCam);
	float specAngle = max(dot(frag.normal, halfway), 0);

	float specular = pow(specAngle, 4 * frag.gloss);
	vec3 specEffect = frag.spec * preMulLightColor * specular;
	return diffEffect + specEffect;
}



float CalculateAttenuation(float dist)
{
	return 1 / pow(dist, 2);
}



vec3 DirLightBlinnPhongEffect(Fragment frag, Light dirLight, vec3 camPos)
{
	return BlinnPhongEffect(frag, dirLight.color * dirLight.intensity, -dirLight.direction, camPos);
}



vec3 SpotLightBlinnPhongEffect(Fragment frag, Light spotLight, vec3 camPos)
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

	vec3 effect = BlinnPhongEffect(frag, spotLight.color * spotLight.intensity, dirToLight, camPos);
	effect *= intensity;
	effect *= CalculateAttenuation(lightFragDist);
	return effect;
}



vec3 PointLightBlinnPhongEffect(Fragment frag, Light pointLight, vec3 camPos)
{
	vec3 dirToLight = pointLight.position - frag.pos;
	float dist = length(dirToLight);

	if (dist > pointLight.range)
	{
		return vec3(0);
	}

	dirToLight = normalize(dirToLight);

	vec3 effect = BlinnPhongEffect(frag, pointLight.color * pointLight.intensity, dirToLight, camPos);
	effect *= CalculateAttenuation(dist);
	return effect;
}

#endif