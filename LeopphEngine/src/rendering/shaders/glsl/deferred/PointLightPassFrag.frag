#version 430 core

#define MIN_SHADOW_BIAS 0.005
#define MAX_SHADOW_BIAS 0.05


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


layout (location = 0) in vec2 in_TexCoords;

layout (location = 0) out vec4 out_FragColor;

layout (location = 0) uniform PointLight u_PointLight;
layout (location = 7) uniform vec3 u_CamPos;
layout (location = 8) uniform sampler2D u_PositionTexture;
layout (location = 9) uniform sampler2D u_NormalTexture;
layout (location = 10) uniform sampler2D u_DiffuseTexture;
layout (location = 11) uniform sampler2D u_SpecularTexture;
layout (location = 12) uniform sampler2D u_ShineTexture;

#if CAST_SHADOW
layout(location = 13) uniform samplerCubeShadow u_ShadowMap;
#endif


vec3 CalculateBlinnPhong(vec3 dirToLight, vec3 fragPos, vec3 fragNormal, vec3 fragDiffuse, vec3 fragSpecular, float fragShine)
{
	float diffuseDot = max(dot(dirToLight, fragNormal), 0);
	vec3 light = fragDiffuse * diffuseDot * u_PointLight.diffuseColor;

	if (diffuseDot > 0)
	{
		vec3 halfway = normalize(dirToLight + normalize(u_CamPos - fragPos));
		light += fragSpecular * pow(max(dot(fragNormal, halfway), 0), 4 * fragShine) * u_PointLight.specularColor;
	}

	return light;
}


float CalculateAttenuation(float constant, float linear, float quadratic, float dist)
{
	return 1.0 / (constant + linear * dist + quadratic * pow(dist, 2));
}


#if CAST_SHADOW
float CalculateShadow(vec3 fragPos, vec3 fragNormal)
{
	vec3 dirToFrag = fragPos - u_PointLight.position;
	float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, normalize(-dirToFrag))), MIN_SHADOW_BIAS);
	return texture(u_ShadowMap, vec4(dirToFrag, (length(dirToFrag) / u_PointLight.range) - bias));
}
#endif


void main()
{
	vec3 fragPos = texture(u_PositionTexture, in_TexCoords).rgb;

	vec3 dirToLight = u_PointLight.position - fragPos;
	float dist = length(dirToLight);

	if (dist > u_PointLight.range)
	{
		out_FragColor = vec4(vec3(0), 1);
		return;
	}

	dirToLight = normalize(dirToLight);

	vec3 fragNormal = texture(u_NormalTexture, in_TexCoords).rgb;
	vec3 fragDiffuse = texture(u_DiffuseTexture, in_TexCoords).rgb;
	vec3 fragSpecular = texture(u_SpecularTexture, in_TexCoords).rgb;
	float fragShine = texture(u_ShineTexture, in_TexCoords).r;

	vec3 light = CalculateBlinnPhong(dirToLight, fragPos, fragNormal, fragDiffuse, fragSpecular, fragShine);
	light *= CalculateAttenuation(u_PointLight.constant, u_PointLight.linear, u_PointLight.quadratic, dist);

	#if CAST_SHADOW
	light *= CalculateShadow(fragPos, fragNormal);
	#endif

	out_FragColor = vec4(light, 1);
}