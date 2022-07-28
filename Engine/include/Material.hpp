#pragma once

#include "Color.hpp"
#include "Texture.hpp"
#include "Types.hpp"

#include <memory>


namespace leopph
{
	struct Material
	{
		Color diffuseColor{250, 255, 255, 255};
		Color specularColor{0, 0, 0, 255};

		std::shared_ptr<Texture> diffuseMap;
		std::shared_ptr<Texture> specularMap;
		std::shared_ptr<Texture> opacityMap;

		f32 gloss{0};
		f32 opacity{1};

		bool twoSided{false};
	};
}