#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <sorcery/Core.hpp>
#include <sorcery/reflection.hpp>


namespace sorcery {
enum class AssetPackagePayloadKind: std::uint8_t {
  kInvalid  = 0,
  kTexture  = 1,
  kMesh     = 2,
  kMaterial = 3,
  kPrefab   = 4,
};


enum class AssetRuntimeType : std::uint8_t {
  kCubemap   = 0,
  kMaterial  = 1,
  kMesh      = 2,
  kScene     = 3,
  kTexture2D = 4,
  kPrefab    = 5,
};


namespace asset_package {
extern SORCERYAPI std::uint32_t const kMagic;


struct Header {
  std::uint32_t magic;
  std::uint32_t version;
  std::uint32_t asset_count;
};


struct Entry {
  AssetPackagePayloadKind payload_kind;
  AssetRuntimeType runtime_type;

  std::uint64_t name_offset;
  std::uint64_t name_size;

  std::uint64_t data_offset;
  std::uint64_t data_size;
};
}


struct AssetImportResult {
  AssetPackagePayloadKind payload_kind;
  rttr::type runtime_type;
  std::string name;
  std::vector<std::byte> bytes;
};


struct AssetEntryInfo {
  rttr::type runtime_type;
  std::string name;
};


struct AssetPackageInfo {
  std::vector<AssetEntryInfo> entries;
};


struct AssetPackageSubasset {
  std::string name;
  std::vector<std::byte> bytes;
  AssetPackagePayloadKind payload_kind;
};


[[nodiscard]] SORCERYAPI
auto PackBinaryAssetPackage(
  std::span<AssetImportResult const> imports
) -> std::optional<std::vector<std::byte>>;

[[nodiscard]] SORCERYAPI
auto UnpackBinaryAssetPackageEntries(
  std::span<std::byte const> file_bytes
) -> std::optional<std::vector<asset_package::Entry>>;

[[nodiscard]] SORCERYAPI
auto PeekBinaryAssetPackage(
  std::filesystem::path const& file_path_abs
) -> std::optional<AssetPackageInfo>;

[[nodiscard]] SORCERYAPI
auto LoadBinaryAssetPackageSubasset(
  std::filesystem::path const& file_path_abs,
  std::size_t entry_idx
) -> std::optional<AssetPackageSubasset>;
}
