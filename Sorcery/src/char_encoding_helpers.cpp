#include "char_encoding_helpers.hpp"

#include <bit>


namespace sorcery {
auto ToU8Str(std::string_view const str) -> std::u8string {
  return std::u8string{str.begin(), str.end()};
}


auto ToU8StrView(std::string_view const str) -> std::u8string_view {
  return std::u8string_view{std::bit_cast<char8_t const*>(str.data()), str.size()};
}


auto ToUntypedStr(std::u8string_view const str) -> std::string {
  return std::string{str.begin(), str.end()};
}


auto ToUntypedStrView(std::u8string_view const str) -> std::string_view {
  return std::string_view{std::bit_cast<char const*>(str.data()), str.size()};
}
}
