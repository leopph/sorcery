#pragma once

#include "LeopphApi.hpp"

#include <string>
#include <string_view>

namespace leopph
{
	class StringEqual
	{
		public:
			using is_transparent = void;

			auto LEOPPHAPI operator()(std::string const& left, std::string const& right) const -> bool;
			auto LEOPPHAPI operator()(std::string const& left, std::string_view right) const -> bool;
			auto LEOPPHAPI operator()(std::string const& left, char const* right) const -> bool;
				  
			auto LEOPPHAPI operator()(std::string_view left, std::string const& right) const -> bool;
			auto LEOPPHAPI operator()(std::string_view left, std::string_view right) const -> bool;
			auto LEOPPHAPI operator()(std::string_view left, char const* right) const -> bool;
				  
			auto LEOPPHAPI operator()(char const* left, std::string const& right) const -> bool;
			auto LEOPPHAPI operator()(char const* left, std::string_view right) const -> bool;
			auto LEOPPHAPI operator()(char const* left, char const* right) const -> bool;
	};
}
