#version 410 core

#define ALPHA_THRESHOLD 0.01
#define MIN_SHADOW_BIAS 0.0001
#define MAX_SHADOW_BIAS 0.001

#ifdef EXISTS_DIRLIGHT
#ifdef DIRLIGHT_SHADOW
#define MAX_DIR_LIGHT_CASCADE_COUNT 3
#endif
#endif

#ifdef EXISTS_SPOTLIGHT
#define MAX_SPOT_LIGHT_COUNT 64
#endif

#ifdef EXISTS_POINTLIGHT
#define MAX_POINT_LIGHT_COUNT 64
#endif


struct DirLight
{
	vec3 direction;
	
	vec3 diffuseColor;
	vec3 specularColor;
};


struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
	
	float innerAngleCosine;
	float outerAngleCosine;
};


struct PointLight
{
	vec3 position;

	vec3 diffuseColor;
	vec3 specularColor;

	float constant;
	float linear;
	float quadratic;
};


struct Material
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float shininess;

	sampler2D ambientMap;
	sampler2D diffuseMap;
	sampler2D specularMap;

	int hasAmbientMap;
	int hasDiffuseMap;
	int hasSpecularMap;
};


layout (location = 0) in vec3 in_FragPos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;

layout (location = 0) out vec4 out_FragColor;

uniform Material u_Material;
uniform vec3 u_AmbientLight;
uniform vec3 u_CamPos;

#ifdef EXISTS_DIRLIGHT
uniform DirLight u_DirLight;
#ifdef DIRLIGHT_SHADOW
layout (location = 3) in float in_ClipPosZ;
uniform sampler2DShadow u_DirLightShadowMaps[MAX_DIR_LIGHT_CASCADE_COUNT];
uniform mat4 u_DirLightClipMatrices[MAX_DIR_LIGHT_CASCADE_COUNT];
uniform float u_DirLightCascadeFarBounds[MAX_DIR_LIGHT_CASCADE_COUNT];
uniform uint u_DirLightCascadeCount;
#endif
#endif

#ifdef EXISTS_SPOTLIGHT
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHT_COUNT];
uniform int u_SpotLightCount;
#endif

#ifdef EXISTS_POINTLIGHT
uniform PointLight u_PointLights[MAX_POINT_LIGHT_COUNT];
uniform int u_PointLightCount;
#endif


#ifdef EXISTS_DIRLIGHT
#ifdef DIRLIGHT_SHADOW
float CalculateDirLightShadow(vec3 fragNormal)
{
	uint cascadeIndex = 0;
    for (int i = 0; i < u_DirLightCascadeCount; i++)
    {
        if (in_ClipPosZ < u_DirLightCascadeFarBounds[i])
        {
            cascadeIndex = i;
            break;
        }
    }
    vec4 fragPosDirLightSpace = vec4(in_FragPos, 1) * u_DirLightClipMatrices[cascadeIndex];
    vec3 normalizedPos = fragPosDirLightSpace.xyz * 0.5 + 0.5;
	float bias = max(MAX_SHADOW_BIAS * (1.0 - dot(fragNormal, -u_DirLight.direction)), MIN_SHADOW_BIAS);
	return texture(u_DirLightShadowMaps[cascadeIndex], vec3(normalizedPos.xy, normalizedPos.z - bias));
}
#endif
#endif


float CalculateAttenuation(float constant, float linear, float quadratic, float dist)
{
	return 1.0 / (constant + linear * dist + quadratic * pow(dist, 2));
}


vec3 CalculateLightEffect(vec3 direction, vec3 normal, vec3 matDiff, vec3 matSpec, vec3 lightDiff, vec3 lightSpec)
{
	float diffuseDot = max(dot(direction, normal), 0);
	vec3 diffuse = matDiff * diffuseDot * lightDiff;

	if (diffuseDot > 0)
	{
		// Blinn-Phong
		vec3 halfway = normalize(direction + normalize(u_CamPos - in_FragPos));
		vec3 specular = matSpec * pow(max(dot(normal, halfway), 0), 4 * u_Material.shininess) * lightSpec;
		return diffuse + specular;

		// Phong
		/*vec3 reflection = normalize(2 * diffuseDot * normal - direction);
		vec3 specular = matSpec * pow(max(dot(reflection, normalize(-inFragPos)), 0), material.shininess) * lightSpec;
		return diffuse + specular;*/
	}

	return diffuse;
}


#ifdef EXISTS_DIRLIGHT
vec3 CalculateDirLight(DirLight dirLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 directionToLight = -dirLight.direction;
	vec3 light = CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, dirLight.diffuseColor, dirLight.specularColor);
	return light;
}
#endif


#ifdef EXISTS_SPOTLIGHT
vec3 CalculateSpotLight(SpotLight spotLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 posDiff = spotLight.position - in_FragPos;
	float dist = length(posDiff);
	vec3 directionToLight = normalize(posDiff);
	float thetaCosine = dot(directionToLight, -spotLight.direction);
	float epsilon = spotLight.innerAngleCosine - spotLight.outerAngleCosine;

	float intensity = clamp((thetaCosine - spotLight.outerAngleCosine) / epsilon, 0.0, 1.0);

	if (intensity > 0)
	{
		float attenuation = CalculateAttenuation(spotLight.constant, spotLight.linear, spotLight.quadratic, dist);
		vec3 light = CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, spotLight.diffuseColor, spotLight.specularColor);
		return intensity * attenuation * light;
	}

	return vec3(0);
}
#endif


#ifdef EXISTS_POINTLIGHT
vec3 CalculatePointLight(PointLight pointLight, vec3 surfaceNormal, vec3 materialDiffuseColor, vec3 materialSpecularColor)
{
	vec3 posDiff = pointLight.position - in_FragPos;
	float dist = length(posDiff);
	vec3 directionToLight = normalize(posDiff);

	float attenuation = CalculateAttenuation(pointLight.constant, pointLight.linear, pointLight.quadratic, dist);
	vec3 light = CalculateLightEffect(directionToLight, surfaceNormal, materialDiffuseColor, materialSpecularColor, pointLight.diffuseColor, pointLight.specularColor);
	return attenuation * light;
}
#endif


void main()
{
	/* Calculate ambient RGB */
	vec3 ambientColor = u_Material.ambientColor;
	vec4 ambientMapColor = vec4(0, 0, 0, 1);
	if (u_Material.hasAmbientMap != 0)
	{
		ambientMapColor = texture(u_Material.ambientMap, in_TexCoords);

		if (ambientMapColor.a < ALPHA_THRESHOLD)
			discard;

		ambientColor *= ambientMapColor.rgb;
	}

	/* Calculate diffuse RGB */
	vec3 diffuseColor = u_Material.diffuseColor;
	vec4 diffuseMapColor = vec4(0, 0, 0, 1);
	if (u_Material.hasDiffuseMap != 0)
	{
		diffuseMapColor = texture(u_Material.diffuseMap, in_TexCoords);

		if (diffuseMapColor.a < ALPHA_THRESHOLD)
			discard;

		diffuseColor *= diffuseMapColor.rgb;
	}

	/* Calculate specular RGB */
	vec3 specularColor = u_Material.specularColor;
	vec4 specularMapColor = vec4(0, 0, 0, 1);
	if (u_Material.hasSpecularMap != 0)
	{
		specularMapColor = texture(u_Material.specularMap, in_TexCoords);

		if (specularMapColor.a < ALPHA_THRESHOLD)
			discard;

		specularColor *= specularMapColor.rgb;
	}

	vec3 normal = normalize(in_Normal);

	/* Base color is ambient */
	vec3 colorSum = ambientColor;

	/* Process and add diffuse and specular colors */
	#ifdef EXISTS_DIRLIGHT
	{
		vec3 light = CalculateDirLight(u_DirLight, normal, diffuseColor, specularColor);
		#ifdef DIRLIGHT_SHADOW
		light *= CalculateDirLightShadow(normal);
		#endif
		colorSum += light;
	}
	#endif

	#ifdef EXISTS_SPOTLIGHT
	for (int i = 0; i < u_SpotLightCount; i++)
	{
		colorSum += CalculateSpotLight(u_SpotLights[i], normal, diffuseColor, specularColor);
	}
	#endif

	#ifdef EXISTS_POINTLIGHT
	for (int i = 0; i < u_PointLightCount; i++)
	{
		colorSum += CalculatePointLight(u_PointLights[i], normal, diffuseColor, specularColor);
	}
	#endif

	/* Combie fragment RGB color with the smallest of the maps' alpha values for transparency */
	out_FragColor = vec4(colorSum, min(min(ambientMapColor.a, diffuseMapColor.a), specularMapColor.a));
}