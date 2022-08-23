//? #version 450 core
//? #extension GL_ARB_bindless_texture : require

#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

struct Material
{
	vec4 diffuseColorAlpha;
	vec4 specularColorGloss;
	sampler2D diffuseMap;
	sampler2D specularMap;
	bool useDiffuseMap;
	bool useSpecularMap;
	float alphaThreshold;
};

layout(std140, binding = 2) uniform PerMaterialData
{
	Material uMaterial;
};

#endif