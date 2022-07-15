#version 420 core


//! #define DIRLIGHT
//! #define NUM_SPOT 3
//! #define NUM_POINT 3
//! #define TRANSPARENT

#include "Lighting.glsl"


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
	colorSum += DirLightEffect(frag, u_DirLight, u_CamPos);
	#endif

	#if NUM_SPOT > 0
	for (int i = 0; i < NUM_SPOT; i++)
	{
		colorSum += SpotLightEffect(frag, u_SpotLights[i], u_CamPos);
	}
	#endif

	#if NUM_POINT > 0
	for (int i = 0; i < NUM_POINT; i++)
	{
		colorSum += PointLightEffect(frag, u_PointLights[i], u_CamPos);
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