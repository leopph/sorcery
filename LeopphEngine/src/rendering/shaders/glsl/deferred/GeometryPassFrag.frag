#version 420 core

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

layout (location = 0) out vec2 out_Norm;
layout (location = 1) out uvec2 out_ColorGloss;

uniform Material u_Material;


void main()
{
    // Encode normal
    vec3 normal = normalize(in_Normal);
    vec2 compressedNormal = normalize(normal.xy) * sqrt(normal.z * 0.5 + 0.5);
    out_Norm = compressedNormal;

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

    // Pack color and gloss into 2x32 bits
    uint packedDiffSpecR = packUnorm4x8(vec4(diffCol, specCol.r));
    uint packedSpecGbGloss = packUnorm4x8(vec4(specCol.gb, 0, 0)) | packHalf2x16(vec2(0, u_Material.gloss));
    out_ColorGloss = uvec2(packedDiffSpecR, packedSpecGbGloss);
}