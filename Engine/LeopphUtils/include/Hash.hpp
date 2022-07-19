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

			auto LEOPPHAPI operator()(std::string const& str) const -> std::size_t;
			auto LEOPPHAPI operator()(std::string_view sv) const -> std::size_t;
			auto LEOPPHAPI operator()(char const* s) const -> std::size_t;

		private:
			std::hash<std::string_view> m_Hash;
	};
}
