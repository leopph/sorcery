#version 420 core


//! #define DIRLIGHT
//! #define NUM_SPOT 3
//! #define NUM_POINT 3
//! #define TRANSPARENT


struct Fragment
{
	vec3 pos;
	vec3 normal;
	vec3 diff;
	vec3 spec;
	float gloss;
};


struct Material
{
	vec3 diffuseColor;
	vec3 specularColor;
	float gloss;
	float opacity;
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D opacityMap;
	bool hasDiffuseMap;
	bool hasSpecularMap;
	bool hasOpacityMap;
};


#ifdef DIRLIGHT
struct DirLight
{
	vec3 direction;
	vec3 diffuseColor;
	vec3 specularColor;
};
#endif


#if NUM_SPOT > 0
struct SpotLight
{
	vec3 position;
	vec3 direction;
	vec3 diffuseColor;
	vec3 specularColor;
	float range;
	float innerCos;
	float outerCos;
};
#endif


#if NUM_POINT > 0
struct PointLight
{
	vec3 position;
	vec3 diffuseColor;
	vec3 specularColor;
	float range;
};
#endif



layout (location = 0) in vec3 in_FragPos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;


#ifdef TRANSPARENT
layout (location = 0) out vec4 out_Accum;
layout (location = 1) out float out_Reveal;
#else
layout (location = 0) out vec3 out_FragColor;
#endif


uniform Material u_Material;


layout(std140, binding = 0) uniform PerFrameConstants
{
	layout(row_major) mat4 u_ViewProjMat;
	vec3 u_AmbLight;
	vec3 u_CamPos;

	#ifdef DIRLIGHT
	DirLight u_DirLight;
	#endif

	#if NUM_SPOT > 0
	SpotLight u_SpotLights[NUM_SPOT];
	#endif

	#if NUM_POINT > 0
	PointLight u_PointLights[NUM_POINT];
	#endif
};



vec3 CalcBlinnPhong(Fragment frag, vec3 dirToLight, vec3 lightDiff, vec3 lightSpec)
{
	float diffuse = max(dot(dirToLight, frag.normal), 0);
	vec3 diffEffect = frag.diff * lightDiff * diffuse;

	if (diffuse <= 0)
	{
		return diffEffect;
	}
		
	vec3 dirToCam =  normalize(u_CamPos - frag.pos);
	vec3 halfway = normalize(dirToLight + dirToCam);
	float specAngle = max(dot(frag.normal, halfway), 0);

	// 0^0 is UB, thus it may produce nan or 0 but we have to make sure
	// transparent objects stay lit from the back too
	#ifdef TRANSPARENT
	if (specAngle <= 0 && frag.gloss == 0)
	{
		return diffEffect;
	}
	#endif

	float specular = pow(specAngle, 4 * frag.gloss);
	vec3 specEffect = frag.spec * lightSpec * specular;
	return diffEffect + specEffect;
}



float CalcAtten(float dist, float range)
{
	return clamp(1 - pow(dist, 2) / pow(range, 2), 0, 1);
}



#if NUM_SPOT > 0
vec3 CalcSpotLightEffect(Fragment frag, SpotLight spotLight)
{
	vec3 dirToLight = spotLight.position - frag.pos;
	float lightFragDist = length(dirToLight);

	if (lightFragDist > spotLight.range)
	{
		return vec3(0);
	}

	dirToLight = normalize(dirToLight);
	float lightFragAngleCos = dot(dirToLight, -spotLight.direction);
	float cutOffCosDiff = spotLight.innerCos - spotLight.outerCos;
	float intensity = clamp((lightFragAngleCos - spotLight.outerCos) / cutOffCosDiff, 0, 1);

	if (intensity == 0)
	{
		return vec3(0);
	}

	vec3 spotLightEffect = CalcBlinnPhong(frag, dirToLight, spotLight.diffuseColor, spotLight.specularColor);
	spotLightEffect *= intensity;
	spotLightEffect *= CalcAtten(lightFragDist, spotLight.range);
	return spotLightEffect;
}
#endif



#if NUM_POINT > 0
vec3 CalcPointLightEffect(Fragment frag, PointLight pointLight)
{
	vec3 dirToLight = pointLight.position - frag.pos;
	float dist = length(dirToLight);

	if (dist > pointLight.range)
	{
		return vec3(0);
	}

	dirToLight = normalize(dirToLight);

	vec3 pointLightEffect = CalcBlinnPhong(frag, dirToLight, pointLight.diffuseColor, pointLight.specularColor);
	pointLightEffect *= CalcAtten(dist, pointLight.range);
	return pointLightEffect;
}
#endif



void main()
{
	Fragment frag;
	frag.pos = in_FragPos;
	frag.normal = normalize(in_Normal);
	frag.diff = u_Material.diffuseColor;
	frag.spec = u_Material.specularColor;
	frag.gloss = u_Material.gloss;
	float alpha = u_Material.opacity;

	if (u_Material.hasDiffuseMap)
	{
		frag.diff = texture(u_Material.diffuseMap, in_TexCoords).rgb;
	}

	if (u_Material.hasSpecularMap)
	{
		frag.spec *= texture(u_Material.specularMap, in_TexCoords).rgb;
	}
	
	if (u_Material.hasOpacityMap)
	{
		alpha *= texture(u_Material.opacityMap, in_TexCoords).r;
	}

	#ifdef TRANSPARENT
	if (alpha >= 1)
	#else
	if (alpha < 1)
	#endif
	{
		discard;
	}

	vec3 colorSum = frag.diff * u_AmbLight;

	#ifdef DIRLIGHT
	colorSum += CalcBlinnPhong(frag, -u_DirLight.direction, u_DirLight.diffuseColor, u_DirLight.specularColor);
	#endif

	#if NUM_SPOT > 0
	for (int i = 0; i < NUM_SPOT; i++)
	{
		colorSum += CalcSpotLightEffect(frag, u_SpotLights[i]);
	}
	#endif

	#if NUM_POINT > 0
	for (int i = 0; i < NUM_POINT; i++)
	{
		colorSum += CalcPointLightEffect(frag, u_PointLights[i]);
	}
	#endif

	#ifdef TRANSPARENT
	float weight = max(min(1.0, max(max(colorSum.r, colorSum.g), colorSum.b) * alpha), alpha) * clamp(0.03 / (1e-5 + pow(frag.pos.z / 200, 4.0)), 1e-2, 3e3);
	out_Accum = vec4(colorSum * alpha, alpha) * weight;
	out_Reveal = alpha;
	#else
	out_FragColor = colorSum;
	#endif
}