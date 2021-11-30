#version 440 core


#ifdef CAST_SHADOW
#define MAX_DIR_LIGHT_CASCADE_COUNT 3
#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.0005
#endif


struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;
};


layout (location = 0) in vec2 in_TexCoords;

layout (location = 0) out vec4 out_FragmentColor;

layout (location = 0) uniform sampler2D u_PositionTexture;
layout (location = 1) uniform sampler2D u_NormalTexture;
layout (location = 2) uniform sampler2D u_DiffuseTexture;
layout (location = 3) uniform sampler2D u_SpecularTexture;
layout (location = 4) uniform sampler2D u_ShineTexture;
layout (location = 5) uniform vec3 u_CameraPosition;
layout (location = 6) uniform DirLight u_DirLight;
#ifdef CAST_SHADOW
layout (location = 9) uniform uint u_CascadeCount;
layout (location = 10) uniform sampler2DShadow u_DirLightShadowMaps[MAX_DIR_LIGHT_CASCADE_COUNT];
layout (location = 10 + MAX_DIR_LIGHT_CASCADE_COUNT) uniform mat4 u_LightClipMatrices[MAX_DIR_LIGHT_CASCADE_COUNT];
layout (location = 10 + 8 * MAX_DIR_LIGHT_CASCADE_COUNT) uniform float u_CascadeFarBounds[MAX_DIR_LIGHT_CASCADE_COUNT];
#endif


vec3 CalculateBlinnPhong(vec3 dirToLight, vec3 fragPos, vec3 fragNormal, vec3 fragDiffuse, vec3 fragSpecular, float fragShine)
{
	float diffuseDot = max(dot(dirToLight, fragNormal), 0);
	vec3 light = fragDiffuse * diffuseDot * u_DirLight.diffuseColor;

	if (diffuseDot > 0)
	{
		vec3 halfway = normalize(dirToLight + normalize(u_CameraPosition - fragPos));
		light += fragSpecular * pow(max(dot(fragNormal, halfway), 0), 4 * fragShine) * u_DirLight.specularColor;
	}

	return light;
}


#ifdef CAST_SHADOW
float CalculateShadow(vec3 fragPos, float fragPosCameraClipZ, vec3 fragNormal)
{
	int cascadeIndex = -1;
	for (int i = 0; i < u_CascadeCount; ++i)
	{
		if (fragPosCameraClipZ < u_CascadeFarBounds[i])
		{
			cascadeIndex = i;
			break;
		}
	}

	vec4 fragPosLightSpace = vec4(fragPos, 1) * u_LightClipMatrices[cascadeIndex];
	vec3 normalizedPos = fragPosLightSpace.xyz;
	normalizedPos *= 0.5;
	normalizedPos += 0.5;

	float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, -u_DirLight.direction)), MIN_SHADOW_BIAS);
	return texture(u_DirLightShadowMaps[cascadeIndex], vec3(normalizedPos.xy, normalizedPos.z - bias));
}
#endif


void main()
{
    vec4 fragPos = texture(u_PositionTexture, in_TexCoords);
    vec3 fragNormal = texture(u_NormalTexture, in_TexCoords).rgb;
    vec3 fragDiffuse = texture(u_DiffuseTexture, in_TexCoords).rgb;
    vec3 fragSpecular = texture(u_SpecularTexture, in_TexCoords).rgb;
	float fragShine = texture(u_ShineTexture, in_TexCoords).r;

	vec3 dirToLight = -u_DirLight.direction;

	vec3 light = CalculateBlinnPhong(dirToLight, fragPos.xyz, fragNormal, fragDiffuse, fragSpecular, fragShine);

	#ifdef CAST_SHADOW
	light *= CalculateShadow(fragPos.xyz, fragPos.w, fragNormal);
	#endif

	out_FragmentColor = vec4(light, 1);
}