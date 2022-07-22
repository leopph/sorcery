#pragma once

#include <string>
#include <string_view>


namespace leopph::internal
{
	struct StringLess
	{
		using is_transparent = void;
		bool operator()(std::string const& left, std::string const& right) const;
		bool operator()(std::string const& left, std::string_view right) const;
		bool operator()(std::string_view left, std::string const& right) const;
	};
}
