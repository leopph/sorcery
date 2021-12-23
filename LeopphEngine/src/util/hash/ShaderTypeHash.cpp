#include "ShaderTypeHash.hpp"


namespace leopph::internal
{
	auto ShaderTypeHash::operator()(ShaderType type) const -> std::size_t
	{
		return m_Hash(static_cast<int>(type));
	}

	auto ShaderTypeHash::operator()(const int i) const -> std::size_t
	{
		return m_Hash(i);
	}
}
