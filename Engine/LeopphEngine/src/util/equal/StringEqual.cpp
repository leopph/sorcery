#include "util/equal/StringEqual.hpp"


namespace leopph::internal
{
	auto StringEqual::operator()(const std::string& left, const std::string& right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(const std::string& left, std::string_view right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(std::string_view left, const std::string& right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(std::string_view left, std::string_view right) const -> bool
	{
		return left == right;
	}
}
