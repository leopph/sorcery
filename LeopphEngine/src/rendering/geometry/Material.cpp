#include "Material.hpp"


namespace leopph
{
	Material::Material() :
		AmbientColor{32, 32, 32},
		DiffuseColor{255, 255, 255},
		SpecularColor{127, 127, 127},
		Gloss{32},
		TwoSided{false}
	{}
}
