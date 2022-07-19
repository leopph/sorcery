#include "Equal.hpp"

#include <cstring>

namespace leopph
{
	auto StringEqual::operator()(std::string const& left, std::string const& right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(std::string const& left, std::string_view const right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(std::string const& left, char const* right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(std::string_view const left, std::string const& right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(std::string_view const left, std::string_view const right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(std::string_view const left, char const* right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(char const* left, std::string const& right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(char const* left, std::string_view const right) const -> bool
	{
		return left == right;
	}

	auto StringEqual::operator()(char const* left, char const* right) const -> bool
	{
		return std::strcmp(left, right);
	}
}
