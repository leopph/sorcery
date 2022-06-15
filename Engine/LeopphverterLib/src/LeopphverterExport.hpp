#pragma once

#include "LeopphApi.hpp"
#include "LeopphverterCommon.hpp"

#include <vector>


namespace leopph::convert
{
	LEOPPHAPI auto Export(Object const& object) -> std::vector<char>;
}
