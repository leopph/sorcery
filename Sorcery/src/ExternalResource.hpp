#pragma once

#include "Core.hpp"
#include "Resources/Resource.hpp"

#include <cstddef>
#include <span>
#include <vector>


namespace sorcery {
enum class ExternalResourceCategory {
  Texture = 0,
  Mesh    = 1
};


LEOPPHAPI auto PackExternalResource(ExternalResourceCategory categ, std::span<std::byte const> resBytes, std::vector<std::byte>& fileBytes) noexcept -> void;
[[nodiscard]] LEOPPHAPI auto UnpackExternalResource(std::span<std::byte const> fileBytes, ExternalResourceCategory& categ, std::vector<std::byte>& resBytes) noexcept -> bool;
}
