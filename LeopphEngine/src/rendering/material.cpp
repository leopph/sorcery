#include "material.h"

namespace leopph
{
	Material::Material() :
		m_DiffuseColor{ .red = 255, .green = 255, .blue = 255 },
		m_SpecularColor{ .red = 255, .green = 255, .blue = 255 },
		m_DiffuseTexture{ nullptr }, m_SpecularTexture{ nullptr }
	{}
}