//! #version 420 core

#pragma option name=TRANSPARENT //! #define TRANSPARENT 1

#include "LightingBuffer.glsl"

layout (location = 0) in vec3 in_FragPos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;

#if NUM_DIR_SHADOW_CASCADES
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

		vec3 color = u_AmbientLight * frag.diff;

	#if DIRLIGHT
	vec3 dirEffect = DirLightEffect(frag, u_DirLight, u_CamPos);

	#if NUM_DIR_SHADOW_CASCADES
	dirEffect *= DirShadow(frag, u_DirLight, in_FragPosNdcZ, u_DirShadowCascades, u_DirShadowMaps);
	#endif

	color += dirEffect;
	#endif


	#if NUM_SPOT_NO_SHADOW
	for (int i = 0; i < NUM_SPOT_SHADOW; i++)
	{
		color += SpotLightEffect(frag, u_NonCastingSpotLights[i], u_CamPos);
	}
	#endif

	#if NUM_SPOT_SHADOW
	for (int i = 0; i < NUM_SPOT_SHADOW; i++)
	{
		vec3 spotEffect = SpotLightEffect(frag, u_CastingSpotLights[i], u_CamPos);
		spotEffect *= SpotShadow(frag, u_CastingSpotLights[i], u_SpotShadowWorldToClipMats[i], u_SpotShadowMaps[i]);
		color += spotEffect;
	}
	#endif

	#if NUM_POINT_NO_SHADOW
	for (int i = 0; i < NUM_POINT_NO_SHADOW; i++)
	{
		color += PointLightEffect(frag, u_NonCastingPointLights[i], u_CamPos);
	}
	#endif

	#if NUM_POINT_SHADOW
	for (int i = 0; i < NUM_POINT_SHADOW; i++)
	{
		vec3 pointEffect = PointLightEffect(frag, u_CastingPointLights[i], u_CamPos);
		pointEffect *= PointShadow(frag, u_CastingPointLights[i], u_PointShadowMaps[i]);
		color += pointEffect;
	}
	#endif

	#if TRANSPARENT
	float weight = max(min(1.0, max(max(color.r, color.g), color.b) * alpha), alpha) * clamp(0.03 / (1e-5 + pow(frag.pos.z / 200, 4.0)), 1e-2, 3e3);
	out_Accum = vec4(color * alpha, alpha) * weight;
	out_Reveal = alpha;
	#else
	out_FragColor = colorSum;
	#endif
}