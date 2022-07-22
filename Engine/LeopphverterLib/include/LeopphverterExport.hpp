#pragma once

#include "LeopphApi.hpp"
#include "LeopphverterTypes.hpp"

#include <bit>
#include <vector>


namespace leopph::convert
{
	LEOPPHAPI std::vector<unsigned char> Export(Object const& object, std::endian endianness = std::endian::native);
}
