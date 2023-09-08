#include "Guid.hpp"

#include <combaseapi.h>

#include <sstream>


namespace sorcery {
auto Guid::Invalid() noexcept -> Guid {
  return Guid{};
}


Guid::Guid(std::uint64_t const lowBits, std::uint64_t const highBits) :
  mLowBits{lowBits},
  mHighBits{highBits} { }


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
  strStream >> guid.mHighBits >> sep >> guid.mLowBits;
  return guid;
}


auto Guid::ToString() const -> std::string {
  return std::string{*this};
}


auto Guid::operator<=>(Guid const& other) const noexcept -> std::strong_ordering {
  auto const hiComp{mHighBits <=> other.mHighBits};
  return hiComp != std::strong_ordering::equivalent
           ? hiComp
           : mLowBits <=> other.mLowBits;
}


auto Guid::IsValid() const noexcept -> bool {
  return mLowBits != 0 || mHighBits != 0;
}


Guid::operator std::string() const {
  std::stringstream strStream;
  strStream << std::hex << mHighBits << "-" << std::hex << mLowBits;
  return strStream.str();
}
}
