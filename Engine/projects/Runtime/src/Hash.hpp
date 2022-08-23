#pragma once

#include "LeopphApi.hpp"

#include <functional>
#include <string>
#include <string_view>


namespace leopph
{
	class StringHash
	{
		public:
			using is_transparent = void;

			LEOPPHAPI std::size_t operator()(std::string const& str) const;
			LEOPPHAPI std::size_t operator()(std::string_view sv) const;
			LEOPPHAPI std::size_t operator()(char const* s) const;

		private:
			std::hash<std::string_view> m_Hash;
	};
}
