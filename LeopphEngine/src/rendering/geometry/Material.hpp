#pragma once

#include "../Texture.hpp"
#include "../../misc/Color.hpp"

#include <memory>


namespace leopph
{
	struct Material
	{
		Color DiffuseColor{250, 255, 255};
		Color SpecularColor{0, 0, 0};

		std::shared_ptr<Texture> DiffuseMap;
		std::shared_ptr<Texture> SpecularMap;

		float Gloss{32};

		bool TwoSided{false};
	};
}
