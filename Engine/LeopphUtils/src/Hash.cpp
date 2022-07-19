#include "Hash.hpp"

namespace leopph
{
	auto StringHash::operator()(std::string const& str) const -> std::size_t
	{
		return m_Hash(str);
	}

	auto StringHash::operator()(std::string_view const sv) const -> std::size_t
	{
		return m_Hash(sv);
	}

	auto StringHash::operator()(char const* s) const -> std::size_t
	{
		return m_Hash(s);
	}
}
