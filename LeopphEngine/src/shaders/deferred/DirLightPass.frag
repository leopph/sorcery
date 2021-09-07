#version 460 core

#define MAX_DIR_LIGHT_CASCADE_COUNT 5
#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.001


struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;
};


layout (location = 0) in vec2 in_TexCoords;

out vec4 out_FragmentColor;

uniform vec3 u_CameraPosition;
uniform DirLight u_DirLight;
uniform uint u_CascadeCount;
uniform float u_CascadeDepth;
uniform mat4 u_LightTransforms[MAX_DIR_LIGHT_CASCADE_COUNT];
uniform sampler2D u_PositionTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AmbientTexture;
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_ShineTexture;
uniform sampler2D u_ShadowMaps[MAX_DIR_LIGHT_CASCADE_COUNT];


void main()
{
    vec3 fragPos = texture(u_PositionTexture, in_TexCoords).rgb;
    vec3 fragNormal = texture(u_NormalTexture, in_TexCoords).rgb;
    vec3 fragAmbient = texture(u_AmbientTexture, in_TexCoords).rgb;
    vec3 fragDiffuse = texture(u_DiffuseTexture, in_TexCoords).rgb;
    vec3 fragSpecular = texture(u_SpecularTexture, in_TexCoords).rgb;
	float fragShine = texture(u_ShineTexture, in_TexCoords).r;

	vec3 directionToLight = -u_DirLight.direction;

	float diffuseDot = max(dot(directionToLight, fragNormal), 0);
	vec3 light = fragDiffuse * diffuseDot * u_DirLight.diffuseColor;

	if (diffuseDot > 0)
	{
		vec3 halfway = normalize(directionToLight + normalize(u_CameraPosition - fragPos));
		light += fragSpecular * pow(max(dot(fragNormal, halfway), 0), 4 * fragShine) * u_DirLight.specularColor;
	}

	for (int i = 0; i < u_CascadeCount; i++)
	{
		if (fragPos.z < ((i * u_CascadeDepth) / (u_CascadeCount * u_CascadeDepth)))
		{
			vec4 fragPosLightSpace = vec4(fragPos, 1) * u_LightTransforms[i];
			vec3 normalizedPos = fragPosLightSpace.xyz;
			normalizedPos *= 0.5;
			normalizedPos += 0.5;

			vec2 texelSize = 1.0 / textureSize(u_ShadowMaps[i], 0);
			float shadow = 0;
			float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, directionToLight)), MIN_SHADOW_BIAS);

			for (int i = -1; i <= 1; i++)
			{
				for (int j = -1; j <= 1; j++)
				{
					float pcfDepth = texture(u_ShadowMaps[i], normalizedPos.xy + vec2(i, j) * texelSize).r;
					shadow += normalizedPos.z - bias > pcfDepth ? 1.0 : 0.0;
				}
			}

			light *= (1 - (shadow / 9));
			break;
		}
	}

	out_FragmentColor = vec4(light, 1);
}