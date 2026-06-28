#pragma once

#include "Core.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>


namespace sorcery {
[[nodiscard]] LEOPPHAPI
auto ReadFileBinary(
  std::filesystem::path const& src,
  std::vector<unsigned char>& out
) -> bool;

[[nodiscard]] LEOPPHAPI
auto ReadFileBinary(
  std::filesystem::path const& src,
  std::vector<std::byte>& out
) -> bool;
}
