#pragma once

#include "../Texture.hpp"
#include "../../misc/Color.hpp"

#include <optional>


namespace leopph
{
	class Material
	{
		public:
			Material();

			Color AmbientColor;
			Color DiffuseColor;
			Color SpecularColor;

			std::optional<impl::Texture> AmbientMap;
			std::optional<impl::Texture> DiffuseMap;
			std::optional<impl::Texture> SpecularMap;

			float Shininess;
			bool TwoSided;
	};
}
