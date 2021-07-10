#include "material.h"

namespace leopph
{
	Material::Material() :
		ambientColor{ .red = 32, .green = 32, .blue = 32 },
		diffuseColor{ .red = 255, .green = 255, .blue = 255 },
		specularColor{ .red = 127, .green = 127, .blue = 127 },
		ambientMap{ nullptr }, diffuseMap{ nullptr }, specularMap{ nullptr },
		shininess{ 32 }
	{}
}