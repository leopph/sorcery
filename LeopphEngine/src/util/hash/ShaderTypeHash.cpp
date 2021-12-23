#include "ShaderTypeHash.hpp"


namespace leopph::internal
{
	std::size_t ShaderTypeHash::operator()(ShaderType type) const
	{
		return m_Hash(static_cast<int>(type));
	}


	std::size_t ShaderTypeHash::operator()(const int i) const
	{
		return m_Hash(i);
	}

}