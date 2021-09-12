#version 460 core

#define MAX_POINT_LIGHT_COUNT 64
#define MAX_SPOT_LIGHT_COUNT 64
#define ALPHA_THRESHOLD 0.01
#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.001


struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;

	sampler2D shadowMap;
};

struct PointLight
{
	vec3 position;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
	
	float innerAngleCosine;
	float outerAngleCosine;
};

struct Material
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float shininess;

	sampler2D ambientMap;
	sampler2D diffuseMap;
	sampler2D specularMap;

	int hasAmbientMap;
	int hasDiffuseMap;
	int hasSpecularMap;
};


layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in vec3 inFragPos;
layout (location = 3) in vec4 inFragPosDirSpace;

layout (location = 0) out vec4 fragmentColor;

layout (location = 2) uniform Material material;

layout (location = 12) uniform vec3 ambientLight;

layout (location = 13) uniform DirLight dirLight;
layout (location = 17) uniform bool existsDirLight;

layout (location = 21) uniform PointLight pointLights[MAX_POINT_LIGHT_COUNT];
layout (location = 18) uniform int pointLightCount;

layout (location = 21 + MAX_POINT_LIGHT_COUNT * 6) uniform SpotLight spotLights[MAX_SPOT_LIGHT_COUNT];
layout (location = 19) uniform int spotLightCount;

layout (location = 20) uniform vec3 cameraPosition;


float CalculateAttenuation(float constant, float linear, float quadratic, float dist)
{
	return 1.0 / (constant + linear * dist + quadratic * pow(dist, 2));
}


float CalculateShadow(vec4 lightSpaceFragPos, vec3 lightDirection, vec3 surfaceNormal, sampler2D shadowMap)
{
	vec3 normalizedPos = lightSpaceFragPos.xyz / lightSpaceFragPos.w;
	normalizedPos *= 0.5;
	normalizedPos += 0.5;

	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	float shadow = 0;
	float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(surfaceNormal, lightDirection)), MIN_SHADOW_BIAS);

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			float pcfDepth = texture(shadowMap, normalizedPos.xy + vec2(i, j) * texelSize).r;
			shadow += normalizedPos.z - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}


vec3 CalculateLightEffect(vec3 direction, vec3 normal, vec3 matDiff, vec3 matSpec, vec3 lightDiff, vec3 lightSpec)
{
	float diffuseDot = max(dot(direction, normal), 0);
	vec3 diffuse = matDiff * diffuseDot * lightDiff;

	if (diffuseDot > 0)
	{
		// Blinn-Phong
		vec3 halfway = normalize(direction + normalize(cameraPosition - inFragPos));
		vec3 specular = matSpec * pow(max(dot(normal, halfway), 0), 4 * material.shininess) * lightSpec;
		return diffuse + specular;

		// Phong
		/*vec3 reflection = normalize(2 * diffuseDot * normal - direction);
		vec3 specular = matSpec * pow(max(dot(reflection, normalize(-inFragPos)), 0), material.shininess) * lightSpec;
		return diffuse + specular;*/
	}

	return diffuse;
}


vec3 CalculateDirLight(DirLight dirLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 directionToLight = -dirLight.direction;
	float shadow = (1 - CalculateShadow(inFragPosDirSpace, directionToLight, surfaceNormal, dirLight.shadowMap));
	vec3 light = CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, dirLight.diffuseColor, dirLight.specularColor);
	return shadow * light;
}


vec3 CalculatePointLight(PointLight pointLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 posDiff = pointLight.position - inFragPos;
	float dist = length(posDiff);
	vec3 directionToLight = normalize(posDiff);

	float attenuation = CalculateAttenuation(pointLight.constant, pointLight.linear, pointLight.quadratic, dist);
	vec3 light = CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, pointLight.diffuseColor, pointLight.specularColor);
	return attenuation * light;
}


vec3 CalculateSpotLight(SpotLight spotLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 posDiff = spotLight.position - inFragPos;
	float dist = length(posDiff);
	vec3 directionToLight = normalize(posDiff);
	float thetaCosine = dot(directionToLight, -spotLight.direction);
	float epsilon = spotLight.innerAngleCosine - spotLight.outerAngleCosine;

	float intensity = clamp((thetaCosine - spotLight.outerAngleCosine) / epsilon, 0.0, 1.0);

	if (intensity > 0)
	{
		float attenuation = CalculateAttenuation(spotLight.constant, spotLight.linear, spotLight.quadratic, dist);
		vec3 light = CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, spotLight.diffuseColor, spotLight.specularColor);
		return intensity * attenuation * light;
	}

	return vec3(0);
}


void main()
{
	/* Calculate ambient RGB */
	vec3 ambientColor = material.ambientColor;
	vec4 ambientMapColor = vec4(0, 0, 0, 1);
	if (material.hasAmbientMap != 0)
	{
		ambientMapColor = texture(material.ambientMap, inTexCoords);

		if (ambientMapColor.a < ALPHA_THRESHOLD)
			discard;

		ambientColor *= ambientMapColor.rgb;
	}

	/* Calculate diffuse RGB */
	vec3 diffuseColor = material.diffuseColor;
	vec4 diffuseMapColor = vec4(0, 0, 0, 1);
	if (material.hasDiffuseMap != 0)
	{
		diffuseMapColor = texture(material.diffuseMap, inTexCoords);

		if (diffuseMapColor.a < ALPHA_THRESHOLD)
			discard;

		diffuseColor *= diffuseMapColor.rgb;
	}

	/* Calculate specular RGB */
	vec3 specularColor = material.specularColor;
	vec4 specularMapColor = vec4(0, 0, 0, 1);
	if (material.hasSpecularMap != 0)
	{
		specularMapColor = texture(material.specularMap, inTexCoords);

		if (specularMapColor.a < ALPHA_THRESHOLD)
			discard;

		specularColor *= specularMapColor.rgb;
	}

	vec3 normal = normalize(inNormal);

	/* Base color is ambient */
	vec3 colorSum = ambientColor;

	/* Process and add diffuse and specular colors */
	if (existsDirLight)
	{
		colorSum += CalculateDirLight(dirLight, normal, diffuseColor, specularColor);
	}

	for (int i = 0; i < pointLightCount; i++)
	{
		colorSum += CalculatePointLight(pointLights[i], normal, diffuseColor, specularColor);
	}

	for (int i = 0; i < spotLightCount; i++)
	{
		colorSum += CalculateSpotLight(spotLights[i], normal, diffuseColor, specularColor);
	}

	/* Combie fragment RGB color with the smallest of the maps' alpha values for transparency */
	fragmentColor = vec4(colorSum, min(min(ambientMapColor.a, diffuseMapColor.a), specularMapColor.a));
}