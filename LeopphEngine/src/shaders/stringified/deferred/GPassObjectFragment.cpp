#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/Shader.hpp"
#include <string>
std::string leopph::impl::Shader::s_GPassObjectFragmentSource{ R"#fileContents#(#version 460 core

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


layout (location = 0) in vec3 inFragPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outAmbientColor;
layout (location = 3) out vec3 outDiffuseColor;
layout (location = 4) out vec4 outSpecularColor;

uniform Material material;


void main()
{
    outPosition = inFragPos;
    outNormal = normalize(inNormal);
    outAmbientColor = material.ambientColor;
    outDiffuseColor = material.diffuseColor;
    outSpecularColor = vec4(material.specularColor, material.shininess);

    if (material.hasAmbientMap != 0)
    {
        outAmbientColor *= texture(material.ambientMap, inTexCoords).rgb;
    }

    if (material.hasDiffuseMap != 0)
    {
        outDiffuseColor *= texture(material.diffuseMap, inTexCoords).rgb;
    }

    if (material.hasSpecularMap != 0)
    {
        outSpecularColor.rgb *= texture(material.specularMap, inTexCoords).rgb;
    }
})#fileContents#" };