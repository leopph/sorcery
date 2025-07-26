#pragma once

#include "Core.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <optional>
#include <vector>


namespace sorcery {
enum class ExternalResourceCategory : std::int32_t {
  Texture = 0,
  Mesh    = 1
};


struct UnpackedExternalResource {
  ExternalResourceCategory category;
  std::span<std::byte const> bytes;
};


LEOPPHAPI auto PackExternalResource(ExternalResourceCategory categ, std::span<std::byte const> resBytes,
                                    std::vector<std::byte>& fileBytes) noexcept -> void;

[[nodiscard]] LEOPPHAPI auto UnpackExternalResource(
  std::span<std::byte const> file_bytes) noexcept -> std::optional<UnpackedExternalResource>;
}
