#pragma once

#include <string>
#include <string_view>


namespace leopph::internal
{
	class StringEqual
	{
		public:
			using is_transparent = void;
			auto operator()(const std::string& left, const std::string& right) const -> bool;
			auto operator()(const std::string& left, std::string_view right) const -> bool;
			auto operator()(std::string_view left, const std::string& right) const -> bool;
			auto operator()(std::string_view left, std::string_view right) const -> bool;
	};
}
