#pragma once

#include <string>
#include <string_view>


namespace leopph::impl
{
	class StringEqual
	{
	public:
		using is_transparent = void;
		bool operator()(const std::string& left, const std::string& right) const;
		bool operator()(const std::string& left, std::string_view right) const; 
		bool operator()(std::string_view left, const std::string& right) const;
		bool operator()(std::string_view left, std::string_view right) const; 
	};
}