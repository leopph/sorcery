#include "Guid.hpp"

#include <combaseapi.h>

#include <sstream>


namespace sorcery {
Guid::Guid(u64 const data0, u64 const data1) :
  mDataLo{ data0 },
  mDataHi{ data1 } { }


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
  std::stringstream strStream;
  strStream << std::hex << str;
  Guid guid;
  char sep;
  strStream >> guid.mDataHi >> sep >> guid.mDataLo;
  return guid;
}


auto Guid::ToString() const -> std::string {
  return std::string{ *this };
}


auto Guid::operator<=>(Guid const& other) const noexcept -> std::strong_ordering {
  auto const hiComp{ mDataHi <=> other.mDataHi };
  return hiComp != std::strong_ordering::equivalent
           ? hiComp
           : mDataLo <=> other.mDataLo;
}


auto Guid::IsValid() const noexcept -> bool {
  return mDataLo != 0 || mDataHi != 0;
}


Guid::operator std::string() const {
  std::stringstream strStream;
  strStream << std::hex << mDataHi << "-" << std::hex << mDataLo;
  return strStream.str();
}
}
