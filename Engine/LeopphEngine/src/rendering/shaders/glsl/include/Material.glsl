//? #version 450 core

#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL


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

#endif