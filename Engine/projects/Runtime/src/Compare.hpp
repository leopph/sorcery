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

			LEOPPHAPI bool operator()(std::string const& left, std::string const& right) const;
			LEOPPHAPI bool operator()(std::string const& left, std::string_view right) const;
			LEOPPHAPI bool operator()(std::string const& left, char const* right) const;

			LEOPPHAPI bool operator()(std::string_view left, std::string const& right) const;
			LEOPPHAPI bool operator()(std::string_view left, std::string_view right) const;
			LEOPPHAPI bool operator()(std::string_view left, char const* right) const;

			LEOPPHAPI bool operator()(char const* left, std::string const& right) const;
			LEOPPHAPI bool operator()(char const* left, std::string_view right) const;
			LEOPPHAPI bool operator()(char const* left, char const* right) const;
	};


	struct StringLess
	{
		using is_transparent = void;

		LEOPPHAPI bool operator()(std::string const& left, std::string const& right) const;
		LEOPPHAPI bool operator()(std::string const& left, std::string_view right) const;
		LEOPPHAPI bool operator()(std::string_view left, std::string const& right) const;
	};
}
