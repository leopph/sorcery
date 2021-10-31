#pragma once

#include "Texture.hpp"
#include "../misc/Color.hpp"

#include <optional>



namespace leopph
{
	class Material
	{
		public:
			Color AmbientColor;
			Color DiffuseColor;
			Color SpecularColor;

			float Shininess;

			std::optional<impl::Texture> AmbientMap;
			std::optional<impl::Texture> DiffuseMap;
			std::optional<impl::Texture> SpecularMap;

			bool TwoSided;

			Material();
	};
}
