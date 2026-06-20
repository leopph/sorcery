#include "resource_package.hpp"

#include <algorithm>

#include "Serialization.hpp"
#include "resources/Cubemap.hpp"
#include "resources/Material.hpp"
#include "resources/Mesh.hpp"
#include "resources/Scene.hpp"
#include "resources/Texture2D.hpp"


namespace sorcery {
namespace resource_package {
std::uint32_t const kMagic{0x19991108};
}


namespace {
[[nodiscard]] auto ToRuntimeType(rttr::type const& type) noexcept -> std::optional<ResourceRuntimeType> {
  if (type == rttr::type::get<Material>()) {
    return ResourceRuntimeType::kMaterial;
  }
  if (type == rttr::type::get<Mesh>()) {
    return ResourceRuntimeType::kMesh;
  }
  if (type == rttr::type::get<Scene>()) {
    return ResourceRuntimeType::kScene;
  }
  if (type == rttr::type::get<Texture2D>()) {
    return ResourceRuntimeType::kTexture2D;
  }
  if (type == rttr::type::get<Cubemap>()) {
    return ResourceRuntimeType::kCubemap;
  }

  return std::nullopt;
}
}


auto PackBinaryResourcePackage(
  std::span<ResourceImportResult const> const imports
) noexcept -> std::optional<std::vector<std::byte>> {
  std::vector<std::byte> package_bytes;

  resource_package::Header const header{
    .magic = resource_package::kMagic,
    .version = 1,
    .resource_count = static_cast<std::uint32_t>(imports.size())
  };

  SerializeToBinary(header.magic, package_bytes);
  SerializeToBinary(header.version, package_bytes);
  SerializeToBinary(header.resource_count, package_bytes);

  std::vector<resource_package::Entry> entries;

  std::uint64_t name_table_size{0};

  for (auto const& import_data : imports) {
    auto const runtime_type = ToRuntimeType(import_data.runtime_type);

    if (!runtime_type) {
      // TODO log error
      return std::nullopt;
    }

    // Data offsets will be filled in later when we know the size of the name table
    entries.emplace_back(import_data.payload_kind, *runtime_type, name_table_size, import_data.name.size(), 0,
      import_data.bytes.size());
    name_table_size += import_data.name.size();
  }

  std::uint64_t data_table_size{0};

  for (std::size_t i{0}; i < imports.size(); ++i) {
    auto& entry = entries[i];

    constexpr auto header_size{3 * sizeof(std::uint32_t)};
    constexpr auto entry_size{
      sizeof(ResourcePackagePayloadKind) + sizeof(ResourceRuntimeType) + 4 * sizeof(std::uint64_t)
    };

    entry.data_offset = header_size + entry_size * imports.size() + name_table_size + data_table_size;
    data_table_size += entry.data_size;
  }

  for (auto const& entry : entries) {
    SerializeToBinary(entry.payload_kind, package_bytes);
    SerializeToBinary(entry.runtime_type, package_bytes);
    SerializeToBinary(entry.name_offset, package_bytes);
    SerializeToBinary(entry.name_size, package_bytes);
    SerializeToBinary(entry.data_offset, package_bytes);
    SerializeToBinary(entry.data_size, package_bytes);
  }

  for (auto const& import_data : imports) {
    std::ranges::copy_n(reinterpret_cast<std::byte const*>(import_data.name.data()), import_data.name.size(),
      std::back_inserter(package_bytes));
  }

  for (auto const& import_data : imports) {
    std::ranges::copy_n(import_data.bytes.begin(), import_data.bytes.size(), std::back_inserter(package_bytes));
  }

  return package_bytes;
}


auto UnpackBinaryResourcePackage(
  std::span<std::byte const> file_bytes
) noexcept -> std::optional<std::vector<resource_package::Entry>> {
  auto const file_byte_size{file_bytes.size()};

  resource_package::Header header;

  if (!DeserializeFromBinary(file_bytes, header.magic) || header.magic != resource_package::kMagic) {
    return std::nullopt;
  }

  file_bytes = file_bytes.subspan(sizeof(resource_package::Header::magic));

  if (!DeserializeFromBinary(file_bytes, header.version) || header.version != 1) {
    return std::nullopt;
  }

  file_bytes = file_bytes.subspan(sizeof(resource_package::Header::version));

  if (!DeserializeFromBinary(file_bytes, header.resource_count)) {
    return std::nullopt;
  }

  file_bytes = file_bytes.subspan(sizeof(resource_package::Header::resource_count));

  std::vector<resource_package::Entry> entries;
  entries.reserve(header.resource_count);

  for (std::uint32_t i{0}; i < header.resource_count; ++i) {
    resource_package::Entry entry;

    if (!DeserializeFromBinary(file_bytes, entry.payload_kind)) {
      return std::nullopt;
    }

    file_bytes = file_bytes.subspan(sizeof(entry.payload_kind));

    if (!DeserializeFromBinary(file_bytes, entry.runtime_type)) {
      return std::nullopt;
    }

    file_bytes = file_bytes.subspan(sizeof(entry.runtime_type));

    if (!DeserializeFromBinary(file_bytes, entry.name_offset)) {
      return std::nullopt;
    }

    file_bytes = file_bytes.subspan(sizeof(entry.name_offset));

    if (!DeserializeFromBinary(file_bytes, entry.name_size)) {
      return std::nullopt;
    }

    file_bytes = file_bytes.subspan(sizeof(entry.name_size));

    if (!DeserializeFromBinary(file_bytes, entry.data_offset)) {
      return std::nullopt;
    }

    file_bytes = file_bytes.subspan(sizeof(entry.data_offset));

    if (!DeserializeFromBinary(file_bytes, entry.data_size)) {
      return std::nullopt;
    }

    file_bytes = file_bytes.subspan(sizeof(entry.data_size));

    if (entry.name_offset > file_byte_size ||
        entry.name_size > file_byte_size - entry.name_offset ||
        entry.data_offset > file_byte_size ||
        entry.data_size > file_byte_size - entry.data_offset) {
      return std::nullopt;
    }

    entries.push_back(entry);
  }

  return entries;
}
}
