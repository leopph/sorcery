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



in vec3 normal;
in vec3 fragmentPosition;
in vec2 textureCoords;


uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform vec3 viewPosition;
uniform PointLight pointLights[4];
uniform int lightNumber;

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



void main()
{
	vec3 norm = normalize(normal);
	vec3 viewDirection = normalize(viewPosition - fragmentPosition);
	vec3 diffuseColor = texture(texture_diffuse0, textureCoords).rgb;
	vec3 specularColor = texture(texture_specular0, textureCoords).rgb;

	vec3 colorSum = vec3(0.0f);
	
	for (int i = 0; i < lightNumber; i++)
		colorSum += CalculatePointLight(pointLights[i], norm, fragmentPosition, viewDirection, diffuseColor, specularColor);

    fragmentColor = vec4(colorSum, 1.0f);
}