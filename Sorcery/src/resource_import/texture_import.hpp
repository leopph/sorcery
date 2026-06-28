#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

#include "../Core.hpp"
#include "../resource_package.hpp"


namespace sorcery {
enum class TextureImportType : std::uint32_t {
  kTexture2D = 0,
  kCubemap   = 1
};


struct TextureImportSettings {
  TextureImportType type{TextureImportType::kTexture2D};
  bool allow_block_compression{true};
  bool is_srgb{true};
  bool generate_mips{true};
};


struct TextureImportSource {
  std::span<std::byte const> file_bytes;
  std::filesystem::path path;
};


struct TextureImportResult {
  std::vector<std::byte> bytes;
  rttr::type runtime_type;
  ResourcePackagePayloadKind payload_kind;
};


[[nodiscard]] SORCERYAPI
auto ImportTexture(
  TextureImportSource const& src,
  TextureImportSettings const& settings
) -> std::optional<TextureImportResult>;
}
