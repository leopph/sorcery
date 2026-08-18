#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

#include "Core.hpp"


namespace sorcery {
[[nodiscard]] SORCERYAPI
auto ReadBinaryFile(
  std::filesystem::path const& src,
  std::vector<std::byte>& out
) -> bool;

[[nodiscard]] SORCERYAPI
auto ReadBinaryFile(
  std::filesystem::path const& src,
  std::vector<unsigned char>& out
) -> bool;
}
