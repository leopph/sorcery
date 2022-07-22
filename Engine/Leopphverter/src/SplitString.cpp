#include "Leopphverter.hpp"
#include "Types.hpp"


namespace leopph::convert::driver
{
	std::vector<std::string_view> SplitString(std::string_view const str, char const sep)
	{
		std::vector<std::string_view> ret;
		u64 base = 0;
		for (u64 i = 0; i < str.size(); i++)
		{
			if (str[i] == sep)
			{
				ret.push_back(str.substr(base, i - base));
				base = i + 1;
			}
		}
		ret.push_back(str.substr(base, str.size()));
		return ret;
	}
}
