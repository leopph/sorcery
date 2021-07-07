#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/shader.h"
#include <string>
std::string leopph::impl::Shader::s_FragmentSource{ R"#fileContents#(#version 460 core

#define ALPHA_THRESHOLD 0.01

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct DirLight
{
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 normal;
in vec2 textureCoords;

uniform vec3 materialDiffuseColor;
uniform vec3 materialSpecularColor;

uniform sampler2D materialDiffuseMap;
uniform sampler2D materialSpecularMap;

uniform int materialHasDiffuseMap;
uniform int materialHasSpecularMap;

uniform int materialDiffuseMapIsTransparent;
uniform int materialSpecularMapIsTransparent;

uniform PointLight pointLights[64];
uniform int lightNumber;

uniform DirLight dirLight;
uniform bool existsDirLight;

out vec4 fragmentColor;

vec4 CalculatePointLight(PointLight light, vec3 surfaceNormal, vec3 fragmentPosition, vec3 viewDirection, vec4 diffuseColor, vec4 specularColor)
{
	vec3 lightDirection = normalize(light.position - fragmentPosition);
	vec3 reflectionDirection = reflect(-lightDirection, surfaceNormal);

	float lightDistance = length(light.position - fragmentPosition);
	float attenuation = 1.0f / (light.constant + light.linear * lightDistance + light.quadratic * pow(lightDistance, 2));

	float diffuseComponent = max(dot(surfaceNormal, lightDirection), 0.0f);
	float specularComponent = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 32);

	vec4 ambient = attenuation * vec4(light.ambient, 1) * diffuseColor;
	vec4 diffuse = attenuation * vec4(light.diffuse, 1) * diffuseComponent * diffuseColor;
	vec4 specular = attenuation * vec4(light.specular, 1) * specularComponent * specularColor;

	return ambient + diffuse + specular;
}

vec4 CalculateDirLight(DirLight light, vec3 surfaceNormal, vec3 viewDirection, vec4 diffuseColor, vec4 specularColor)
{
	vec3 lightDirection = normalize(-light.direction);
	vec3 reflectionDirection = reflect(-lightDirection, surfaceNormal);

	float diffuseComponent = max(dot(surfaceNormal, lightDirection), 0.0f);
	float specularComponent = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 32);

	vec4 ambient = vec4(light.ambient, 1) * diffuseColor;
	vec4 diffuse = vec4(light.diffuse, 1) * diffuseComponent * diffuseColor;
	vec4 specular = vec4(light.specular, 1) * specularComponent * specularColor;

	return ambient + diffuse + specular;
}

void main()
{
	vec3 norm = normalize(normal);
	vec3 fragCoord = vec3(gl_FragCoord);
	vec3 viewDirection = normalize(-fragCoord);

	vec4 diffuseColor = vec4(materialDiffuseColor, 1);
	
	if (materialHasDiffuseMap != 0)
	{
		if (materialDiffuseMapIsTransparent != 0)
		{
			diffuseColor *= texture(materialDiffuseMap, textureCoords);
			
			if (diffuseColor.a < ALPHA_THRESHOLD)
				discard;
		}
		else
		{
			diffuseColor *= vec4(texture(materialDiffuseMap, textureCoords).rgb, 1);
		}
	}

	vec4 specularColor = vec4(materialSpecularColor, 1);
	
	if (materialHasSpecularMap != 0)
	{
		if (materialSpecularMapIsTransparent != 0)
			specularColor *= texture(materialSpecularMap, textureCoords);
		else
			specularColor *= vec4(texture(materialSpecularMap, textureCoords).rgb, 1);
	}

	fragmentColor = vec4(0.0f);

	if (existsDirLight)
		fragmentColor += CalculateDirLight(dirLight, norm, viewDirection, diffuseColor, specularColor);
	
	for (int i = 0; i < lightNumber; i++)
		fragmentColor += CalculatePointLight(pointLights[i], norm, fragCoord, viewDirection, diffuseColor, specularColor);

	if (fragmentColor.a < 0.1)
		discard;
})#fileContents#" };