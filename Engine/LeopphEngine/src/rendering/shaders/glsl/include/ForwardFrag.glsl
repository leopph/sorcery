//? #version 450 core

#ifndef FORWARD_FRAG_GLSL
#define FORWARD_FRAG_GLSL

//! #define TRANSPARENT 1

#include "BlinnPhong.glsl"
#include "Material.glsl"
#include "CameraData.glsl"

layout (location = 0) in vec3 in_FragPos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;

#if NUM_DIRLIGHT_SHADOW_CASCADE
layout (location = 3) in float in_FragPosNdcZ;
#endif

#if TRANSPARENT
layout (location = 0) out vec4 out_Accum;
layout (location = 1) out float out_Reveal;
#else
layout (location = 0) out vec3 out_FragColor;
#endif

uniform Material u_Material;


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

	#if TRANSPARENT
	if (alpha >= 1)
	#else
	if (alpha < 1)
	#endif
	{
		discard;
	}

	vec3 camPos = GetCameraPosition();

	vec3 color = GetAmbientIntensity() * frag.diff;

	if (ExistsDirLight())
	{
		DirLight dirLight = GetDirLight();
		vec3 dirEffect = DirLightBlinnPhongEffect(frag, dirLight, camPos);
		color += dirEffect;
	}

	uint numSpot = GetNumSpotLights();
	for (uint i = 0; i < numSpot; i++)
	{
		vec3 spotEffect = SpotLightBlinnPhongEffect(frag, GetSpotLight(i), camPos);
		color += spotEffect;
	}

	uint numPoint = GetNumPointLights();
	for (uint i = 0; i < numPoint; i++)
	{
		vec3 pointEffect = PointLightBlinnPhongEffect(frag, GetPointLight(i), camPos);
		color += pointEffect;
	}

	#if TRANSPARENT
	float weight = max(min(1.0, max(max(color.r, color.g), color.b) * alpha), alpha) * clamp(0.03 / (1e-5 + pow(frag.pos.z / 200, 4.0)), 1e-2, 3e3);
	out_Accum = vec4(color * alpha, alpha) * weight;
	out_Reveal = alpha;
	#else
	out_FragColor = color;
	#endif
}

#endif