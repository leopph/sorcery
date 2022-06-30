#include "util/less/StringLess.hpp"


bool leopph::internal::StringLess::operator()(const std::string& left, const std::string& right) const
{
	return left < right;
}


bool leopph::internal::StringLess::operator()(const std::string& left, std::string_view right) const
{
	return left < right;
}


bool leopph::internal::StringLess::operator()(std::string_view left, const std::string& right) const
{
	return left < right;
}
