#pragma once

#include "../misc/color.h"
#include "texture.h"

#include <memory>

namespace leopph
{
	struct Material
	{
	public:
		Color ambientColor;
		Color diffuseColor;
		Color specularColor;

		float shininess;

		std::unique_ptr<impl::Texture> diffuseMap;
		std::unique_ptr<impl::Texture> specularMap;


		Material();
	};
}