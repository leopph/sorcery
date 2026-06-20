#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "Core.hpp"
#include "Reflection.hpp"


namespace sorcery {
enum class ResourcePackagePayloadKind: std::uint8_t {
  kTexture  = 0,
  kMesh     = 1,
  kMaterial = 2,
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


[[nodiscard]] SORCERYAPI
auto PackBinaryResourcePackage(
  std::span<ResourceImportResult const> imports
) noexcept -> std::optional<std::vector<std::byte>>;

[[nodiscard]] SORCERYAPI
auto UnpackBinaryResourcePackage(
  std::span<std::byte const> file_bytes
) noexcept -> std::optional<std::vector<resource_package::Entry>>;
}
