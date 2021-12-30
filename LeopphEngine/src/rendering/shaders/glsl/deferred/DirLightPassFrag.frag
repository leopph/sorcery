#version 440 core

#ifdef CAST_SHADOW
#define MAX_NUM_CASCADES 3
#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.01
#endif


struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;
};


layout (location = 0) in vec2 in_TexCoords;

layout (location = 0) out vec4 out_FragmentColor;

uniform sampler2D u_PositionTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_ShineTexture;

uniform vec3 u_CameraPosition;
uniform DirLight u_DirLight;

#ifdef CAST_SHADOW
uniform uint u_NumCascades;
uniform mat4 u_CascadeMatrices[MAX_NUM_CASCADES];
uniform float u_CascadeBounds[MAX_NUM_CASCADES];
uniform sampler2DShadow u_DirLightShadowMaps[MAX_NUM_CASCADES];
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
	for (int i = 0; i < u_NumCascades; ++i)
	{
		if (fragPosCameraClipZ < u_CascadeBounds[i])
		{
			vec4 fragPosLightSpace = vec4(fragPos, 1) * u_CascadeMatrices[i];
			vec3 normalizedPos = fragPosLightSpace.xyz;
			normalizedPos *= 0.5;
			normalizedPos += 0.5;
			
			float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, -u_DirLight.direction)), MIN_SHADOW_BIAS);
			return texture(u_DirLightShadowMaps[i], vec3(normalizedPos.xy, normalizedPos.z - bias));
		}
	}
	return 1.0;
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