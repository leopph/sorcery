#include "Guid.hpp"

#include <combaseapi.h>

#include <sstream>


namespace leopph {
namespace {
std::stringstream gStringStream;
}


Guid::Guid(u64 const data0, u64 const data1) :
  mData0{ data0 },
  mData1{ data1 } { }


auto Guid::Generate() -> Guid {
  Guid ret;

  while (!ret.IsValid()) {
    GUID guid;
    while (CoCreateGuid(&guid) != S_OK) { }
    std::memcpy(&ret, &guid, 16);
  }

  return ret;
}


auto Guid::Parse(std::string_view const str) -> Guid {
  gStringStream.clear();
  gStringStream.str({});
  gStringStream << std::hex << str;
  Guid guid{};
  char sep;
  gStringStream >> guid.mData1 >> sep >> guid.mData0;
  return guid;
}


auto Guid::ToString() const -> std::string {
  gStringStream.clear();
  gStringStream.str({});
  gStringStream << std::hex << mData1 << "-" << std::hex << mData0;
  return gStringStream.str();
}


auto Guid::operator==(Guid const& other) const -> bool {
  return mData0 == other.mData0 && mData1 == other.mData1;
}


auto Guid::IsValid() const noexcept -> bool {
  return mData0 != 0 || mData1 != 0;
}
}
