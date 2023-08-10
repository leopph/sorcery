#pragma once

#include "Core.hpp"

#include <filesystem>
#include <vector>


namespace sorcery {
[[nodiscard]] LEOPPHAPI auto ReadFileBinary(std::filesystem::path const& src, std::vector<unsigned char>& out) -> bool;
}
