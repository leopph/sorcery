#pragma once

#include "../misc/color.h"
#include "texture.h"

#include <memory>

namespace leopph
{
	struct Material
	{
	public:
		Color m_DiffuseColor;
		std::unique_ptr<impl::Texture> m_DiffuseTexture;

		Color m_SpecularColor;
		std::unique_ptr<impl::Texture> m_SpecularTexture;

		Material();
	};
}