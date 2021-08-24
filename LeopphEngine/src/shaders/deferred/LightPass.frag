#version 460 core

#define MAX_POINT_LIGHT_COUNT 64
#define MAX_SPOT_LIGHT_COUNT 64


struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;
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


layout (location = 0) in vec2 inTexCoords;

out vec4 fragmentColor;

uniform vec3 ambientLight;

uniform DirLight dirLight;
uniform bool existsDirLight;

uniform PointLight pointLights[MAX_POINT_LIGHT_COUNT];
uniform int pointLightCount;

uniform SpotLight spotLights[MAX_SPOT_LIGHT_COUNT];
uniform int spotLightCount;

uniform vec3 cameraPosition;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;


float CalculateAttenuation(float constant, float linear, float quadratic, float dist)
{
	return 1.0 / (constant + linear * dist + quadratic * pow(dist, 2));
}


vec3 CalculateLightEffect(vec3 fragPos, vec3 directionToLight, vec3 normal, vec3 matDiff, vec3 matSpec, float matShiny, vec3 lightDiff, vec3 lightSpec)
{
	float diffuseDot = max(dot(directionToLight, normal), 0);
	vec3 diffuse = matDiff * diffuseDot * lightDiff;

	if (diffuseDot > 0)
	{
		// Blinn-Phong
		vec3 halfway = normalize(directionToLight + normalize(cameraPosition - fragPos));
		vec3 specular = matSpec * pow(max(dot(normal, halfway), 0), 4 * matShiny) * lightSpec;
		return diffuse + specular;

		// Phong
		/*vec3 reflection = normalize(2 * diffuseDot * normal - direction);
		vec3 specular = matSpec * pow(max(dot(reflection, normalize(-inFragPos)), 0), material.shininess) * lightSpec;
		return diffuse + specular;*/
	}

	return diffuse;
}


vec3 CalculateDirLight(DirLight dirLight, vec3 fragPos, vec3 normal, vec3 matDiff, vec3 matSpec, float matShiny)
{
	vec3 directionToLight = -dirLight.direction;
	//float shadow = (1 - CalculateShadow(inFragPosDirSpace, directionToLight, surfaceNormal, dirLight.shadowMap));
	vec3 light = CalculateLightEffect(fragPos, directionToLight, normal, matDiff, matSpec, matShiny, dirLight.diffuseColor, dirLight.specularColor);
	return /*shadow */ light;
}


vec3 CalculatePointLight(PointLight pointLight, vec3 fragPos, vec3 normal, vec3 matDiff, vec3 matSpec, float matShiny)
{
	vec3 posDiff = pointLight.position - fragPos;
	float dist = length(posDiff);
	vec3 directionToLight = normalize(posDiff);

	float attenuation = CalculateAttenuation(pointLight.constant, pointLight.linear, pointLight.quadratic, dist);
	vec3 light = CalculateLightEffect(fragPos, directionToLight, normal, matDiff, matSpec, matShiny, pointLight.diffuseColor, pointLight.specularColor);
	return attenuation * light;
}


vec3 CalculateSpotLight(SpotLight spotLight, vec3 fragPos, vec3 normal, vec3 matDiff, vec3 matSpec, float matShiny)
{
	vec3 posDiff = spotLight.position - fragPos;
	float dist = length(posDiff);
	vec3 directionToLight = normalize(posDiff);
	float thetaCosine = dot(directionToLight, -spotLight.direction);
	float epsilon = spotLight.innerAngleCosine - spotLight.outerAngleCosine;

	float intensity = clamp((thetaCosine - spotLight.outerAngleCosine) / epsilon, 0.0, 1.0);

	if (intensity > 0)
	{
		float attenuation = CalculateAttenuation(spotLight.constant, spotLight.linear, spotLight.quadratic, dist);
		vec3 light = CalculateLightEffect(fragPos, directionToLight, normal, matDiff, matSpec, matShiny, spotLight.diffuseColor, spotLight.specularColor);
		return intensity * attenuation * light;
	}

	return vec3(0);
}


void main()
{
    vec3 position = texture(positionTexture, inTexCoords).rgb;
    vec3 normal = texture(normalTexture, inTexCoords).rgb;
    vec3 ambientColor = texture(ambientTexture, inTexCoords).rgb;
    vec3 diffuseColor = texture(diffuseTexture, inTexCoords).rgb;
    vec4 specularColorAndShininess = texture(specularTexture, inTexCoords);

    vec3 colorSum = ambientColor;

    if (existsDirLight)
	{
		colorSum += CalculateDirLight(dirLight, position, normal, diffuseColor, specularColorAndShininess.rgb, specularColorAndShininess.a);
	}

	for (int i = 0; i < pointLightCount; i++)
	{
		colorSum += CalculatePointLight(pointLights[i], position, normal, diffuseColor, specularColorAndShininess.rgb, specularColorAndShininess.a);
	}

	for (int i = 0; i < spotLightCount; i++)
	{
		colorSum += CalculateSpotLight(spotLights[i], position, normal, diffuseColor, specularColorAndShininess.rgb, specularColorAndShininess.a);
	}

    fragmentColor = vec4(colorSum, 1);
}