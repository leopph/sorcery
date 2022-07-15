#version 420 core

#include "Lighting.glsl"

//! #define DIRLIGHT
//! #define SPOTLIGHT
//! #define POINTLIGHT
//! #define SHADOW
//! #define NUM_CASCADES 3



layout(pixel_center_integer) in vec4 gl_FragCoord;
layout(location = 0) out vec3 out_FragColor;


layout(binding = 0) uniform usampler2D u_NormColorGlossTex;
layout(binding = 1) uniform sampler2D u_DepthTex;


layout(std140, binding = 0) uniform Commons
{
	layout(row_major) mat4 u_CamViewProjInv;
	vec3 u_CamPos;
};


#ifdef AMBIENTLIGHT
uniform vec3 u_AmbLight;
#endif

#ifdef DIRLIGHT
uniform DirLight u_DirLight;
#endif

#ifdef SPOTLIGHT
uniform SpotLight u_SpotLight;
#endif

#ifdef POINTLIGHT
uniform PointLight u_PointLight;
#endif


#ifdef SHADOW

#ifdef DIRLIGHT
uniform DirShadowCascade u_DirShadowCascades[NUM_CASCADES];
layout(binding = 2) uniform sampler2DShadow u_DirShadowMaps[NUM_CASCADES];
#endif

#ifdef SPOTLIGHT
uniform mat4 u_SpotShadowWorldToClipMat;
layout(binding = 2) uniform sampler2DShadow u_SpotShadowMap;
#endif

#ifdef POINTLIGHT
layout(binding = 2) uniform samplerCube u_PointShadowMap;
#endif

#endif




void main()
{	
	// Parse gbuffer contents
	Fragment frag;

	// Reconstruct pos from depth
	vec4 fragPosNdc = vec4(gl_FragCoord.xy / textureSize(u_DepthTex, 0).xy * 2 - 1, texelFetch(u_DepthTex, ivec2(gl_FragCoord.xy), 0).r * 2 - 1, 1);
	vec4 fragPosWorld = fragPosNdc * u_CamViewProjInv;
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

	vec3 color;

	#ifdef AMBIENTLIGHT
	color = u_Light * frag.diff;
	#endif

	#ifdef DIRLIGHT
	color = DirLightEffect(frag, u_DirLight, u_CamPos);

	#ifdef SHADOW
	color *= DirShadow(frag, u_DirLight, fragPosNdc.z, u_DirShadowCascades, u_DirShadowMaps);
	#endif

	#endif

	#ifdef SPOTLIGHT
	color = SpotLightEffect(frag, u_SpotLight, u_CamPos);

	#ifdef SHADOW
	color *= SpotShadow(frag, u_SpotLight, u_SpotShadowWorldToClipMat, u_SpotShadowMap);
	#endif

	#endif

	#ifdef POINTLIGHT
	color = PointLightEffect(frag, u_PointLight, u_CamPos);

	#ifdef SHADOW
	color *= PointShadow(frag, u_PointLight, u_PointShadowMap);
	#endif

	#endif

	out_FragColor = color;
}