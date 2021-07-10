#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/shader.h"
#include <string>
std::string leopph::impl::Shader::s_FragmentSource{ R"#fileContents#(#version 460 core

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


in vec3 normal;
in vec2 textureCoords;
in vec3 fragPos;

uniform Material material;

uniform PointLight pointLights[64];
uniform int lightNumber;

uniform DirLight dirLight;
uniform bool existsDirLight;

out vec4 fragmentColor;


vec3 CalculateLightEffect(vec3 direction, vec3 normal, vec3 matDiff, vec3 matSpec, vec3 lightDiff, vec3 lightSpec)
{
	float diffuseDot = max(dot(direction, normal), 0);
	vec3 diffuse = matDiff * diffuseDot * lightDiff;

	if (diffuseDot > 0)
	{
		vec3 reflection = normalize(2 * diffuseDot * normal - direction);
		vec3 specular = matSpec * pow(max(dot(reflection, normalize(-fragPos)), 0), material.shininess) * lightSpec;
		return diffuse + specular;
	}

	return diffuse;
}

vec3 CalculatePointLight(PointLight pointLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 directionToLight = normalize(pointLight.position - fragPos);
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
		diffuseMapColor = texture(material.diffuseMap, textureCoords);

		if (diffuseMapColor.a < ALPHA_THRESHOLD)
			discard;

		diffuseColor *= diffuseMapColor.rgb;
	}

	vec3 specularColor = material.specularColor;
	vec4 specularMapColor = vec4(0, 0, 0, 1);
	if (material.hasSpecularMap != 0)
	{
		specularMapColor = texture(material.specularMap, textureCoords);

		if (specularMapColor.a < ALPHA_THRESHOLD)
			discard;

		specularColor *= specularMapColor.rgb;
	}

	vec3 normalizedNormal = normalize(normal);

	vec3 colorSum = vec3(0);

	if (existsDirLight)
		colorSum += CalculateDirLight(dirLight, normalizedNormal, diffuseColor, specularColor);

	for (int i = 0; i < lightNumber; i++)
		colorSum += CalculatePointLight(pointLights[i], normalizedNormal, diffuseColor, specularColor);

	fragmentColor = vec4(colorSum, min(diffuseMapColor.a, specularMapColor.a));
})#fileContents#" };