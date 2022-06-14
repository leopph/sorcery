#pragma once

#include "Color.hpp"
#include "Texture.hpp"

#include <memory>


namespace leopph
{
	struct Material
	{
		Color DiffuseColor{250, 255, 255};
		Color SpecularColor{0, 0, 0};

		std::shared_ptr<Texture> DiffuseMap;
		std::shared_ptr<Texture> SpecularMap;
		std::shared_ptr<Texture> OpacityMap;

		float Gloss{0};
		float Opacity{1};

		bool TwoSided{false};
	};
}