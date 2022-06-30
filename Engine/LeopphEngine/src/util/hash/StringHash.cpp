#include "util/hash/StringHash.hpp"


namespace leopph::internal
{
	auto StringHash::operator()(const std::string_view sv) const -> std::size_t
	{
		return m_Hash(sv);
	}

	auto StringHash::operator()(const std::string& str) const -> std::size_t
	{
		return m_Hash(str);
	}
}
