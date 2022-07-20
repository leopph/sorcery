//? #version 450 core


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