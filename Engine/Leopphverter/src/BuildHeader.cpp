#include "BuildHeader.hpp"

#include <algorithm>
#include <format>


namespace leopph::convert::driver
{
	namespace
	{
		constexpr auto VERSION_NUMBER = "1.0";
	}



	std::string build_printed_header()
	{
		std::string_view constexpr vertBorder{"###"};
		auto const title = std::format("Leopphverter v{}", VERSION_NUMBER);
		std::string_view constexpr funny{"Converting random formats to leopph3d since 2022."};
		auto const maxLineLngth = std::max<std::size_t>(title.size(), funny.size());

		auto const titlePaddingLngth = (maxLineLngth - title.size()) / 2 + 1;
		auto const funnyPaddingLngth = (maxLineLngth - funny.size()) / 2 + 1;

		std::string const titlePadding(titlePaddingLngth, ' ');
		std::string const funnyPadding(funnyPaddingLngth, ' ');

		auto const minPaddingLength = std::min(titlePaddingLngth, funnyPaddingLngth);

		std::string const horizBorder(maxLineLngth + 2 * (vertBorder.size() + minPaddingLength), '#');
		auto const horizSubBorder = [vertBorder, &horizBorder]
		{
			std::string ret;
			ret += vertBorder;
			ret.append(horizBorder.size() - 2 * vertBorder.size(), ' ');
			ret += vertBorder;
			return ret;
		}();

		std::string header;
		header.append(horizBorder).push_back('\n');
		header.append(horizSubBorder).push_back('\n');
		header.append(vertBorder).append(titlePadding).append(title).append(titlePadding).append(vertBorder).push_back('\n');
		header.append(vertBorder).append(funnyPadding).append(funny).append(funnyPadding).append(vertBorder).push_back('\n');
		header.append(horizSubBorder).push_back('\n');
		header.append(horizBorder).push_back('\n');

		return header;
	}
}
