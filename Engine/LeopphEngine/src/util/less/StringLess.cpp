#include "util/less/StringLess.hpp"


bool leopph::internal::StringLess::operator()(std::string const& left, std::string const& right) const
{
	return left < right;
}


bool leopph::internal::StringLess::operator()(std::string const& left, std::string_view right) const
{
	return left < right;
}


bool leopph::internal::StringLess::operator()(std::string_view left, std::string const& right) const
{
	return left < right;
}
