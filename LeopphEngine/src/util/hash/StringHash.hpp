#pragma once

#include <string>
#include <string_view>


namespace leopph::internal
{
	class StringHash
	{
	public:
		using is_transparent = void;
		std::size_t operator()(std::string_view sv) const;
		std::size_t operator()(const std::string& str) const;

	private:
		std::hash<std::string_view> m_Hash;
	};
}