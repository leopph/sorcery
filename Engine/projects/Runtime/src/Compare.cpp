#include "Compare.hpp"

#include <cstring>


namespace leopph
{
	bool StringEqual::operator()(std::string const& left, std::string const& right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(std::string const& left, std::string_view const right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(std::string const& left, char const* right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(std::string_view const left, std::string const& right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(std::string_view const left, std::string_view const right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(std::string_view const left, char const* right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(char const* left, std::string const& right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(char const* left, std::string_view const right) const
	{
		return left == right;
	}



	bool StringEqual::operator()(char const* left, char const* right) const
	{
		return std::strcmp(left, right);
	}



	bool StringLess::operator()(std::string const& left, std::string const& right) const
	{
		return left < right;
	}



	bool StringLess::operator()(std::string const& left, std::string_view const right) const
	{
		return left < right;
	}



	bool StringLess::operator()(std::string_view const left, std::string const& right) const
	{
		return left < right;
	}
}
