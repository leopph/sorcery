#version 410 core

struct Material
{
	vec3 diffuseColor;
	vec3 specularColor;

	float gloss;

	sampler2D diffuseMap;
	sampler2D specularMap;

	bool hasDiffuseMap;
	bool hasSpecularMap;
};


layout (location = 0) in vec3 in_Normal;
layout (location = 1) in vec2 in_TexCoords;

layout (location = 0) out vec3 out_NormGloss;
layout (location = 1) out vec3 out_Diff;
layout (location = 2) out vec3 out_Spec;

uniform Material u_Material;


void main()
{
    // Encode normal
    vec3 normal = normalize(in_Normal);
    vec2 compressedNormal = normalize(normal.xy) * sqrt(normal.z * 0.5 + 0.5);
    out_NormGloss = vec3(compressedNormal, u_Material.gloss);

    vec3 diffCol = u_Material.diffuseColor;
    vec3 specCol = u_Material.specularColor;

    if (u_Material.hasDiffuseMap)
    {
        diffCol *= texture(u_Material.diffuseMap, in_TexCoords).rgb;
    }

    if (u_Material.hasSpecularMap)
    {
        specCol *= texture(u_Material.specularMap, in_TexCoords).rgb;
    }

    out_Diff = diffCol;
    out_Spec = specCol;
}