#pragma once

#include "Core.hpp"
#include "Resources/Resource.hpp"

#include <cstdint>
#include <span>
#include <vector>


namespace sorcery {
LEOPPHAPI auto PackResourceBinary(Resource::Category categ, std::span<std::uint8_t const> resBytes, std::vector<std::uint8_t>& fileBytes) noexcept -> void;
[[nodiscard]] LEOPPHAPI auto UnpackResourceBinary(std::span<std::uint8_t const> fileBytes, Resource::Category& categ, std::vector<std::uint8_t>& resBytes) noexcept -> bool;
}
