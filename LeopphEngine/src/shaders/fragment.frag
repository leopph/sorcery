#version 460 core

#define ALPHA_THRESHOLD 0.01


struct PointLight
{
	vec3 position;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
};

struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;
};

struct Material
{
	vec3 ambientcolor;
	vec3 diffuseColor;
	vec3 specularColor;

	float shininess;

	sampler2D diffuseMap;
	sampler2D specularMap;

	int hasDiffuseMap;
	int hasSpecularMap;
};


layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in vec3 inFragPos;

out vec4 fragmentColor;

uniform Material material;

uniform PointLight pointLights[64];
uniform int lightNumber;

uniform DirLight dirLight;
uniform bool existsDirLight;


vec3 CalculateLightEffect(vec3 direction, vec3 normal, vec3 matDiff, vec3 matSpec, vec3 lightDiff, vec3 lightSpec)
{
	float diffuseDot = max(dot(direction, normal), 0);
	vec3 diffuse = matDiff * diffuseDot * lightDiff;

	if (diffuseDot > 0)
	{
		vec3 reflection = normalize(2 * diffuseDot * normal - direction);
		vec3 specular = matSpec * pow(max(dot(reflection, normalize(-inFragPos)), 0), material.shininess) * lightSpec;
		return diffuse + specular;
	}

	return diffuse;
}

vec3 CalculatePointLight(PointLight pointLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 directionToLight = normalize(pointLight.position - inFragPos);
	return CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, pointLight.diffuseColor, pointLight.specularColor);
}

vec3 CalculateDirLight(DirLight dirLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 directionToLight = -dirLight.direction;
	return CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, dirLight.diffuseColor, dirLight.specularColor);
}

void main()
{
	vec3 diffuseColor = material.diffuseColor;
	vec4 diffuseMapColor = vec4(0, 0, 0, 1);
	if (material.hasDiffuseMap != 0)
	{
		diffuseMapColor = texture(material.diffuseMap, inTexCoords);

		if (diffuseMapColor.a < ALPHA_THRESHOLD)
			discard;

		diffuseColor *= diffuseMapColor.rgb;
	}

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

	vec3 colorSum = vec3(0);

	if (existsDirLight)
		colorSum += CalculateDirLight(dirLight, normal, diffuseColor, specularColor);

	for (int i = 0; i < lightNumber; i++)
		colorSum += CalculatePointLight(pointLights[i], normal, diffuseColor, specularColor);

	fragmentColor = vec4(colorSum, min(diffuseMapColor.a, specularMapColor.a));
}