#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "Core.hpp"
#include "Reflection.hpp"


namespace sorcery {
enum class ResourcePackagePayloadKind: std::uint8_t {
  kInvalid  = 0,
  kTexture  = 1,
  kMesh     = 2,
  kMaterial = 3,
};


enum class ResourceRuntimeType : std::uint8_t {
  kCubemap   = 0,
  kMaterial  = 1,
  kMesh      = 2,
  kScene     = 3,
  kTexture2D = 4,
};


namespace resource_package {
extern SORCERYAPI std::uint32_t const kMagic;


struct Header {
  std::uint32_t magic;
  std::uint32_t version;
  std::uint32_t resource_count;
};


struct Entry {
  ResourcePackagePayloadKind payload_kind;
  ResourceRuntimeType runtime_type;

  std::uint64_t name_offset;
  std::uint64_t name_size;

  std::uint64_t data_offset;
  std::uint64_t data_size;
};
}


struct ResourceImportResult {
  ResourcePackagePayloadKind payload_kind;
  rttr::type runtime_type;
  std::string name;
  std::vector<std::byte> bytes;
};


struct ResourceEntryInfo {
  rttr::type runtime_type;
  std::string name;
};


struct ResourcePackageInfo {
  std::vector<ResourceEntryInfo> entries;
};


struct ResourcePackageSubresource {
  std::string name;
  std::vector<std::byte> bytes;
  ResourcePackagePayloadKind payload_kind;
};


[[nodiscard]] SORCERYAPI
auto PackBinaryResourcePackage(
  std::span<ResourceImportResult const> imports
) noexcept -> std::optional<std::vector<std::byte>>;

[[nodiscard]] SORCERYAPI
auto UnpackBinaryResourcePackageEntries(
  std::span<std::byte const> file_bytes
) noexcept -> std::optional<std::vector<resource_package::Entry>>;

[[nodiscard]] SORCERYAPI
auto PeekBinaryResourcePackage(
  std::filesystem::path const& file_path_abs
) noexcept -> std::optional<ResourcePackageInfo>;

[[nodiscard]] SORCERYAPI
auto LoadBinaryResourcePackageSubresource(
  std::filesystem::path const& file_path_abs,
  std::size_t entry_idx
) -> std::optional<ResourcePackageSubresource>;
}
