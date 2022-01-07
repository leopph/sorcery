#version 430 core

struct Material
{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float shininess;

	sampler2D ambientMap;
	sampler2D diffuseMap;
	sampler2D specularMap;

	int hasAmbientMap;
	int hasDiffuseMap;
	int hasSpecularMap;
};


layout (location = 0) in vec3 in_Normal;
layout (location = 1) in vec2 in_TexCoords;

layout (location = 0) out vec4 out_NormalShine;
layout (location = 1) out vec3 out_AmbientColor;
layout (location = 2) out vec3 out_DiffuseColor;
layout (location = 3) out vec3 out_SpecularColor;

uniform Material u_Material;


void main()
{
    out_NormalShine = vec4(normalize(in_Normal), u_Material.shininess);
    out_AmbientColor = u_Material.ambientColor;
    out_DiffuseColor = u_Material.diffuseColor;
    out_SpecularColor = u_Material.specularColor;

    if (u_Material.hasAmbientMap != 0)
    {
        out_AmbientColor *= texture(u_Material.ambientMap, in_TexCoords).rgb;
    }

    if (u_Material.hasDiffuseMap != 0)
    {
        out_DiffuseColor *= texture(u_Material.diffuseMap, in_TexCoords).rgb;
    }

    if (u_Material.hasSpecularMap != 0)
    {
        out_SpecularColor *= texture(u_Material.specularMap, in_TexCoords).rgb;
    }
}