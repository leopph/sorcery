#include "Util.hpp"

#include <combaseapi.h>


namespace leopph {
	auto GenerateGUID() -> GUID {
		::GUID guid;
		while (CoCreateGuid(&guid) != S_OK);
		leopph::GUID ret;
		std::memcpy(&ret, &guid, 16);
		return ret;
	}
}