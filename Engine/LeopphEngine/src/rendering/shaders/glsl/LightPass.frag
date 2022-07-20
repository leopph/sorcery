//! #version 450 core

#include "LightingBuffer.glsl"
#include "TransformBuffer.glsl"


layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(location = 0) out vec3 out_FragColor;


layout(binding = 0) uniform usampler2D u_NormColorGlossTex;
layout(binding = 1) uniform sampler2D u_DepthTex;


void main()
{	
	// Parse gbuffer contents
	Fragment frag;

	// Reconstruct pos from depth
	vec4 fragPosNdc = vec4(gl_FragCoord.xy / textureSize(u_DepthTex, 0).xy * 2 - 1, texelFetch(u_DepthTex, ivec2(gl_FragCoord.xy), 0).r * 2 - 1, 1);
	vec4 fragPosWorld = fragPosNdc * u_ViewProjMatInv;
	fragPosWorld /= fragPosWorld.w;
	frag.pos = fragPosWorld.xyz;

	uvec3 packedNormColorGloss = texelFetch(u_NormColorGlossTex, ivec2(gl_FragCoord.xy), 0).rgb;

	// Decode normal
	vec2 comprNorm = unpackSnorm2x16(packedNormColorGloss.x);
	frag.normal = vec3(comprNorm.xy, 1 - abs(comprNorm.x) - abs(comprNorm.y));
	frag.normal.xy = frag.normal.z < 0 ? (1 - abs(frag.normal.yx)) * vec2(frag.normal.x >= 0 ? 1 : -1, frag.normal.y >= 0 ? 1 : -1) : frag.normal.xy;
	frag.normal = normalize(frag.normal);

	// Unpack color and gloss bits
	vec4 unPack = unpackUnorm4x8(packedNormColorGloss.y);
	frag.diff = unPack.xyz;
	frag.spec.r = unPack.w;
	frag.spec.gb = unpackUnorm4x8(packedNormColorGloss.z).xy;
	frag.gloss = unpackHalf2x16(packedNormColorGloss.z).y;

	vec3 color = u_AmbientLight * frag.diff;

	#if DIRLIGHT
	vec3 dirEffect = DirLightEffect(frag, u_DirLight, u_CamPos);

	#if NUM_DIR_SHADOW_CASCADES
	dirEffect *= DirShadow(frag, u_DirLight, fragPosNdc.z, u_DirShadowCascades, u_DirShadowMaps);
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

	out_FragColor = color;
}