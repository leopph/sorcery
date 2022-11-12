#include "Util.hpp"

#include <combaseapi.h>

#include <sstream>


namespace leopph {
	namespace {
		std::stringstream gStringStream;
	}


	auto Guid::Generate() -> Guid {
		GUID guid;
		while (CoCreateGuid(&guid) != S_OK);
		Guid ret;
		std::memcpy(&ret, &guid, 16);
		return ret;
	}


	auto Guid::Parse(std::string_view const str) -> Guid
	{
		gStringStream.clear();
		gStringStream.str({});
		gStringStream << std::hex << str;
		Guid guid{};
		char sep;
		gStringStream >> guid.data1 >> sep >> guid.data0;
		return guid;
	}


	auto Guid::ToString() const -> std::string {
		gStringStream.clear();
		gStringStream.str({});
		gStringStream << std::hex << data1 << "-" << std::hex << data0;
		return gStringStream.str();
	}


    auto Guid::operator==(Guid const& other) const -> bool
    {
		return data0 == other.data0 && data1 == other.data1;
    }
}