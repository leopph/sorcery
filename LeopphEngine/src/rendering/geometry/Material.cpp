#include "Material.hpp"


namespace leopph
{
	Material::Material() :
		AmbientColor{.red = 32, .green = 32, .blue = 32},
		DiffuseColor{.red = 255, .green = 255, .blue = 255},
		SpecularColor{.red = 127, .green = 127, .blue = 127},
		Shininess{32},
		TwoSided{false}
	{}
}
