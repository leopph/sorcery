#pragma once

#include <string>
#include <string_view>

#include "Core.hpp"


namespace sorcery {
// Returns an UTF8-typed string with the same content as the provided string view.
// MAKE SURE THE PASSED STRING IS VALID UTF-8! This function does NO validation!
[[nodiscard]] LEOPPHAPI auto ToU8Str(std::string_view str) -> std::u8string;

// Returns an UTF8-typed string view that refers the same content as the provided string view.
// MAKE SURE THE PASSED STRING IS VALID UTF-8! This function does NO validation!
[[nodiscard]] LEOPPHAPI auto ToU8StrView(std::string_view str) -> std::u8string_view;

// Returns an UTF8-encoded generic std::string with the same content as the provided UTF-8 string view.
[[nodiscard]] LEOPPHAPI auto ToUntypedStr(std::u8string_view str) -> std::string;

// Returns an untyped string view that refers to the same content as the provided UTF-8 string view.
// The content thus is still assumed to be valid UTF-8, but the type is not enforced.
[[nodiscard]] LEOPPHAPI auto ToUntypedStrView(std::u8string_view str) -> std::string_view;
}
