#version 460 core

#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.0005


struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
	float range;
	
	float innerAngleCosine;
	float outerAngleCosine;
};


layout (location = 0) in vec2 in_TexCoords;

layout (location = 0) out vec4 out_FragmentColor;

layout (location = 0) uniform SpotLight u_SpotLight;
layout (location = 10) uniform sampler2D u_PositionTexture;
layout (location = 11) uniform sampler2D u_NormalTexture;
layout (location = 12) uniform sampler2D u_DiffuseTexture;
layout (location = 13) uniform sampler2D u_SpecularTexture;
layout (location = 14) uniform sampler2D u_ShineTexture;
layout (location = 15) uniform vec3 u_CameraPosition;
layout (location = 16) uniform sampler2DShadow u_ShadowMap;
layout (location = 17) uniform mat4 u_LightWorldToClipMatrix;


vec3 CalculateBlinnPhong(vec3 dirToLight, vec3 fragPos, vec3 fragNormal, vec3 fragDiffuse, vec3 fragSpecular, float fragShine)
{
	float diffuseDot = max(dot(dirToLight, fragNormal), 0);
	vec3 light = fragDiffuse * diffuseDot * u_SpotLight.diffuseColor;

	if (diffuseDot > 0)
	{
		vec3 halfway = normalize(dirToLight + normalize(u_CameraPosition - fragPos));
		light += fragSpecular * pow(max(dot(fragNormal, halfway), 0), 4 * fragShine) * u_SpotLight.specularColor;
	}

	return light;
}


float CalculateAttenuation(float constant, float linear, float quadratic, float dist)
{
	return 1.0 / (constant + linear * dist + quadratic * pow(dist, 2));
}


float CalculateShadow(vec3 dirToLight, vec3 fragPos, vec3 fragNormal)
{
	vec4 fragPosLightSpace = vec4(fragPos, 1) * u_LightWorldToClipMatrix;
	vec3 normalizedPos = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;

	vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
	float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, dirToLight)), MIN_SHADOW_BIAS);
	float shadow = 0;

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			shadow += texture(u_ShadowMap, vec3(normalizedPos.xy + vec2(i, j) * texelSize, normalizedPos.z - bias));
		}
	}

	return shadow / 9;
}


void main()
{
	vec3 fragPos = texture(u_PositionTexture, in_TexCoords).rgb;

	vec3 dirToLight = u_SpotLight.position - fragPos;
	float dist = length(dirToLight);

	if (dist > u_SpotLight.range)
	{
		out_FragmentColor = vec4(vec3(0), 1);
		return;
	}

	dirToLight = normalize(dirToLight);

	/* Let theta be the angle between
	 * the direction vector pointing from the fragment to the light
	 * and the reverse of the light's direction vector. */
	float thetaCosine = dot(dirToLight, -u_SpotLight.direction);

	/* Let epsilon be the difference between
	 * the cosines of the light's cutoff angles. */
	float epsilon = u_SpotLight.innerAngleCosine - u_SpotLight.outerAngleCosine;

	/* Determine if the frag is
	 * inside the inner angle, or
	 * between the inner and outer angles, or
	 * outside the outer angle. */
	float intensity = clamp((thetaCosine - u_SpotLight.outerAngleCosine) / epsilon, 0.0, 1.0);

	if (intensity == 0)
	{
		out_FragmentColor = vec4(vec3(0), 1);
		return;
	}

	vec3 fragNormal = texture(u_NormalTexture, in_TexCoords).rgb;
    vec3 fragDiffuse = texture(u_DiffuseTexture, in_TexCoords).rgb;
    vec3 fragSpecular = texture(u_SpecularTexture, in_TexCoords).rgb;
	float fragShine = texture(u_ShineTexture, in_TexCoords).r;

	vec3 light = CalculateBlinnPhong(dirToLight, fragPos, fragNormal, fragDiffuse, fragSpecular, fragShine);
	float attenuation = CalculateAttenuation(u_SpotLight.constant, u_SpotLight.linear, u_SpotLight.quadratic, dist);
	float shadow = CalculateShadow(dirToLight, fragPos, fragNormal);

	out_FragmentColor = vec4(light * intensity * attenuation * shadow, 1);
}