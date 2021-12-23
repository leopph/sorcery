#include "StringHash.hpp"


namespace leopph::internal
{
	std::size_t StringHash::operator()(const std::string_view sv) const
	{
		return m_Hash(sv);
	}

	std::size_t StringHash::operator()(const std::string& str) const
	{
		return m_Hash(str);
	}
}