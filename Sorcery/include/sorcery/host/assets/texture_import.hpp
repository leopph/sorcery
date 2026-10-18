#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include <sorcery/Core.hpp>
#include <sorcery/reflection.hpp>
#include <sorcery/host/assets/asset_package.hpp>


namespace sorcery {
enum class TextureImportType : std::uint32_t {
  kTexture2D = 0,
  kCubemap   = 1
};


enum class AlphaCoverageMode : std::uint32_t {
  kAuto     = 0, // Model import: derived from the clip materials using the texture. Standalone: off.
  kDisabled = 1,
  kCustom   = 2 // Uses alpha_coverage_threshold.
};


struct TextureImportSettings {
  TextureImportType type{TextureImportType::kTexture2D};
  bool allow_block_compression{true};
  bool is_srgb{true};
  bool generate_mips{true};
  AlphaCoverageMode alpha_coverage_mode{AlphaCoverageMode::kAuto};
  float alpha_coverage_threshold{0.5f};
};


struct TextureImportSource {
  std::span<std::byte const> file_bytes;
  std::u8string_view ext; // with dot, e.g., ".jpg"
};


struct TextureImportResult {
  std::vector<std::byte> bytes;
  rttr::type runtime_type;
  AssetPackagePayloadKind payload_kind;
};


[[nodiscard]] SORCERYAPI
auto ImportTexture(
  TextureImportSource const& src,
  TextureImportSettings const& settings
) -> std::optional<TextureImportResult>;
}
