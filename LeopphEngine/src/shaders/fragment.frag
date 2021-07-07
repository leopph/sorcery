#version 460 core

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

uniform PointLight pointLights[64];
uniform int lightNumber;

uniform DirLight dirLight;
uniform bool existsDirLight;

out vec4 fragmentColor;

vec3 CalculatePointLight(PointLight light, vec3 surfaceNormal, vec3 fragmentPosition, vec3 viewDirection, vec3 diffuseColor, vec3 specularColor)
{
	vec3 lightDirection = normalize(light.position - fragmentPosition);
	vec3 reflectionDirection = reflect(-lightDirection, surfaceNormal);

	float lightDistance = length(light.position - fragmentPosition);
	float attenuation = 1.0f / (light.constant + light.linear * lightDistance + light.quadratic * pow(lightDistance, 2));

	float diffuseComponent = max(dot(surfaceNormal, lightDirection), 0.0f);
	float specularComponent = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 32);

	vec3 ambient = attenuation * light.ambient * diffuseColor;
	vec3 diffuse = attenuation * light.diffuse * diffuseComponent * diffuseColor;
	vec3 specular = attenuation * light.specular * specularComponent * specularColor;

	return ambient + diffuse + specular;
}

vec3 CalculateDirLight(DirLight light, vec3 surfaceNormal, vec3 viewDirection, vec3 diffuseColor, vec3 specularColor)
{
	vec3 lightDirection = normalize(-light.direction);
	vec3 reflectionDirection = reflect(-lightDirection, surfaceNormal);

	float diffuseComponent = max(dot(surfaceNormal, lightDirection), 0.0f);
	float specularComponent = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 32);

	vec3 ambient = light.ambient * diffuseColor;
	vec3 diffuse = light.diffuse * diffuseComponent * diffuseColor;
	vec3 specular = light.specular * specularComponent * specularColor;

	return ambient + diffuse + specular;
}

void main()
{
	vec3 norm = normalize(normal);
	vec3 fragCoord = vec3(gl_FragCoord);
	vec3 viewDirection = normalize(-fragCoord);

	vec3 diffuseColor = materialDiffuseColor;
	
	if (materialHasDiffuseMap != 0)
		diffuseColor *= texture(materialDiffuseMap, textureCoords).rgb;

	vec3 specularColor = materialSpecularColor;
	
	if (materialHasSpecularMap != 0)
		specularColor *= texture(materialSpecularMap, textureCoords).rgb;

	vec3 colorSum = vec3(0.0f);

	if (existsDirLight)
		colorSum += CalculateDirLight(dirLight, norm, viewDirection, diffuseColor, specularColor);
	
	for (int i = 0; i < lightNumber; i++)
		colorSum += CalculatePointLight(pointLights[i], norm, fragCoord, viewDirection, diffuseColor, specularColor);

	fragmentColor = vec4(colorSum, 1.0f);
}