//? #version 450 core
//? #extension GL_ARB_bindless_texture : require

#ifndef FORWARD_FRAG_GLSL
#define FORWARD_FRAG_GLSL

//! #define TRANSPARENT 1

#include "BlinnPhong.glsl"
#include "Lighting.glsl"
#include "CameraData.glsl"
#include "Material.glsl"

layout (location = 0) in vec3 in_FragPos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;

/*#if NUM_DIRLIGHT_SHADOW_CASCADE
layout (location = 3) in float in_FragPosNdcZ;
#endif*/

/*#if TRANSPARENT
layout (location = 0) out vec4 out_Accum;
layout (location = 1) out float out_Reveal;
#else*/
layout (location = 0) out vec3 out_FragColor;
//#endif


void main()
{
	Fragment frag;
	frag.diff = uMaterial.diffuseColorAlpha.rgb;
	frag.alpha = uMaterial.diffuseColorAlpha.a;

	if (uMaterial.useDiffuseMap)
	{
		vec4 diffMapColor = texture(uMaterial.diffuseMap, in_TexCoords);
		frag.diff *= diffMapColor.rgb;
		frag.alpha *= diffMapColor.a;
	}

	if (frag.alpha < uMaterial.alphaThreshold)
	{
		discard;
	}

	frag.pos = in_FragPos;
	frag.normal = normalize(in_Normal);
	frag.spec = uMaterial.specularColorGloss.rgb;
	frag.gloss = uMaterial.specularColorGloss.a;

	if (uMaterial.useSpecularMap)
	{
		frag.spec *= texture(uMaterial.specularMap, in_TexCoords).rgb;
	}

	vec3 camPos = uCamPosition;

	vec3 color = uAmbientIntensity * frag.diff;

	for (int i = 0; i < uNumLights; i++)
	{
		if (uLights[i].type == LIGHT_TYPE_DIR)
		{
			color += DirLightBlinnPhongEffect(frag, uLights[i], camPos);
		}
		else if (uLights[i].type == LIGHT_TYPE_SPOT)
		{
			color += SpotLightBlinnPhongEffect(frag, uLights[i], camPos);
		}
		else if (uLights[i].type == LIGHT_TYPE_POINT)
		{
			color += PointLightBlinnPhongEffect(frag, uLights[i], camPos);
		}
		else
		{
			// There was an error with a light, produce magenta
			out_FragColor = vec3(1, 0, 1);
			return;
		}
	}

	//#if TRANSPARENT
	//float weight = max(min(1.0, max(max(color.r, color.g), color.b) * alpha), alpha) * clamp(0.03 / (1e-5 + pow(frag.pos.z / 200, 4.0)), 1e-2, 3e3);
	//out_Accum = vec4(color * alpha, alpha) * weight;
	//out_Reveal = alpha;
	//#else
	out_FragColor = color;
	//#endif
}

#endif