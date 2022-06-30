#pragma once

#include "Color.hpp"
#include "GlTexture.hpp"

#include <memory>


namespace leopph
{
	struct Material
	{
		Color DiffuseColor{250, 255, 255};
		Color SpecularColor{0, 0, 0};

		std::shared_ptr<GlTexture> DiffuseMap;
		std::shared_ptr<GlTexture> SpecularMap;
		std::shared_ptr<GlTexture> OpacityMap;

		float Gloss{1};
		float Opacity{1};

		bool TwoSided{false};
	};
}
