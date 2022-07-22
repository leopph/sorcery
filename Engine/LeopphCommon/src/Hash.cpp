#include "Hash.hpp"


namespace leopph
{
	std::size_t StringHash::operator()(std::string const& str) const
	{
		return m_Hash(str);
	}


	std::size_t StringHash::operator()(std::string_view const sv) const
	{
		return m_Hash(sv);
	}


	std::size_t StringHash::operator()(char const* s) const
	{
		return m_Hash(s);
	}
}
