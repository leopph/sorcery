#pragma once

#include "../Texture.hpp"
#include "../../misc/Color.hpp"

#include <memory>


namespace leopph
{
	class Material final
	{
		public:
			Material();

			Color AmbientColor;
			Color DiffuseColor;
			Color SpecularColor;

			std::shared_ptr<Texture> AmbientMap;
			std::shared_ptr<Texture> DiffuseMap;
			std::shared_ptr<Texture> SpecularMap;

			float Gloss;
			bool TwoSided;
	};
}
