#version 430 core

struct Material
{
	vec3 diffuseColor;
	vec3 specularColor;

	float gloss;

	sampler2D diffuseMap;
	sampler2D specularMap;

	int hasDiffuseMap;
	int hasSpecularMap;
};


layout (location = 0) in vec3 in_Normal;
layout (location = 1) in vec2 in_TexCoords;

layout (location = 0) out vec3 out_NormalGloss;
layout (location = 1) out vec3 out_DiffuseColor;
layout (location = 2) out vec3 out_SpecularColor;

uniform Material u_Material;


void main()
{
    // Encode normal
    vec3 normal = normalize(in_Normal);
    vec2 compressedNormal = normalize(normal.xy) * sqrt(normal.z * 0.5 + 0.5);
    out_NormalGloss = vec3(compressedNormal, u_Material.gloss);

    out_DiffuseColor = u_Material.diffuseColor;
    out_SpecularColor = u_Material.specularColor;

    if (u_Material.hasDiffuseMap != 0)
    {
        out_DiffuseColor *= texture(u_Material.diffuseMap, in_TexCoords).rgb;
    }

    if (u_Material.hasSpecularMap != 0)
    {
        out_SpecularColor *= texture(u_Material.specularMap, in_TexCoords).rgb;
    }
}