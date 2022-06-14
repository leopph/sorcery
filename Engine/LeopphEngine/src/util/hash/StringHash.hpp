#pragma once

#include <string>
#include <string_view>


namespace leopph::internal
{
	class StringHash
	{
		public:
			using is_transparent = void;
			auto operator()(std::string_view sv) const -> std::size_t;
			auto operator()(const std::string& str) const -> std::size_t;

		private:
			std::hash<std::string_view> m_Hash;
	};
}
