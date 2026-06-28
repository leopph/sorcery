#include "resource_package.hpp"

#include <algorithm>
#include <fstream>

#include "Serialization.hpp"
#include "vector_stream.hpp"
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


[[nodiscard]] auto ToRttrType(ResourceRuntimeType const type) noexcept -> std::optional<rttr::type> {
  switch (type) {
    case ResourceRuntimeType::kMaterial:
      return rttr::type::get<Material>();
    case ResourceRuntimeType::kMesh:
      return rttr::type::get<Mesh>();
    case ResourceRuntimeType::kScene:
      return rttr::type::get<Scene>();
    case ResourceRuntimeType::kTexture2D:
      return rttr::type::get<Texture2D>();
    case ResourceRuntimeType::kCubemap:
      return rttr::type::get<Cubemap>();
    default:
      return std::nullopt;
  }
}


struct HeaderSerializer {
  static
  auto Write(resource_package::Header const& header, std::ostream& os) -> bool {
    return os.write(reinterpret_cast<char const* const>(&header.magic), sizeof(header.magic)) &&
           os.write(reinterpret_cast<char const* const>(&header.version), sizeof(header.version)) &&
           os.write(reinterpret_cast<char const* const>(&header.resource_count), sizeof(header.resource_count));
  }


  [[nodiscard]] static
  auto Read(std::istream& is) -> std::optional<resource_package::Header> {
    if (!is.seekg(0, std::ios::beg)) {
      return std::nullopt;
    }

    resource_package::Header header;

    if (!is.read(reinterpret_cast<char*>(&header.magic), sizeof(header.magic))) {
      return std::nullopt;
    }

    if (header.magic != resource_package::kMagic) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&header.version), sizeof(header.version))) {
      return std::nullopt;
    }

    if (header.version != 1) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&header.resource_count), sizeof(header.resource_count))) {
      return std::nullopt;
    }

    return header;
  }


  constexpr static auto kHeaderSize{3 * sizeof(std::uint32_t)};
};


struct EntrySerializer {
  static
  auto Write(resource_package::Entry const& entry, std::ostream& os) -> bool {
    return os.write(reinterpret_cast<char const* const>(&entry.payload_kind), sizeof(entry.payload_kind)) &&
           os.write(reinterpret_cast<char const* const>(&entry.runtime_type), sizeof(entry.runtime_type)) &&
           os.write(reinterpret_cast<char const* const>(&entry.name_offset), sizeof(entry.name_offset)) &&
           os.write(reinterpret_cast<char const* const>(&entry.name_size), sizeof(entry.name_size)) &&
           os.write(reinterpret_cast<char const* const>(&entry.data_offset), sizeof(entry.data_offset)) &&
           os.write(reinterpret_cast<char const* const>(&entry.data_size), sizeof(entry.data_size));
  }


  [[nodiscard]] static
  auto Read(std::istream& is, std::size_t const idx) -> std::optional<resource_package::Entry> {
    if (!is.seekg(HeaderSerializer::kHeaderSize + idx * kEntrySize, std::ios::beg)) {
      return std::nullopt;
    }

    resource_package::Entry entry;

    if (!is.read(reinterpret_cast<char*>(&entry.payload_kind), sizeof(entry.payload_kind))) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&entry.runtime_type), sizeof(entry.runtime_type))) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&entry.name_offset), sizeof(entry.name_offset))) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&entry.name_size), sizeof(entry.name_size))) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&entry.data_offset), sizeof(entry.data_offset))) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&entry.data_size), sizeof(entry.data_size))) {
      return std::nullopt;
    }

    return entry;
  }


  constexpr static auto kEntrySize{
    sizeof(ResourcePackagePayloadKind) + sizeof(ResourceRuntimeType) + 4 * sizeof(std::uint64_t)
  };
};


[[nodiscard]] auto ComputeNameTableOffset(std::size_t const subresource_count) noexcept -> std::size_t {
  return HeaderSerializer::kHeaderSize + EntrySerializer::kEntrySize * subresource_count;
}


[[nodiscard]] auto IsEntryValid(resource_package::Entry const& entry, std::size_t const package_size) -> bool {
  return !(entry.name_offset > package_size ||
           entry.name_size > package_size - entry.name_offset ||
           entry.data_offset > package_size ||
           entry.data_size > package_size - entry.data_offset);
}
}


auto PackBinaryResourcePackage(
  std::span<ResourceImportResult const> const imports
) noexcept -> std::optional<std::vector<std::byte>> {
  std::vector<std::byte> package_bytes;
  ByteVectorOstream os{package_bytes};

  resource_package::Header const header{
    .magic = resource_package::kMagic,
    .version = 1,
    .resource_count = static_cast<std::uint32_t>(imports.size())
  };

  if (!HeaderSerializer::Write(header, os)) {
    return std::nullopt;
  }

  std::vector<resource_package::Entry> entries;

  auto const name_table_offset{ComputeNameTableOffset(imports.size())};
  std::uint64_t name_table_size{0};

  for (auto const& import_data : imports) {
    auto const runtime_type = ToRuntimeType(import_data.runtime_type);

    if (!runtime_type) {
      // TODO log error
      return std::nullopt;
    }

    // Data offsets will be filled in later when we know the size of the name table
    entries.emplace_back(import_data.payload_kind, *runtime_type, name_table_offset + name_table_size,
      import_data.name.size(), 0, import_data.bytes.size());
    name_table_size += import_data.name.size();
  }

  auto const data_table_offset{name_table_offset + name_table_size};
  std::uint64_t data_table_size{0};

  for (std::size_t i{0}; i < imports.size(); ++i) {
    entries[i].data_offset = data_table_offset + data_table_size;
    data_table_size += entries[i].data_size;
  }

  for (auto const& entry : entries) {
    EntrySerializer::Write(entry, os);
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


auto UnpackBinaryResourcePackageEntries(
  std::span<std::byte const> file_bytes
) noexcept -> std::optional<std::vector<resource_package::Entry>> {
  auto const package_size{file_bytes.size()};
  ByteSpanIstream is{file_bytes};

  auto const header{HeaderSerializer::Read(is)};

  if (!header) {
    return std::nullopt;
  }

  std::vector<resource_package::Entry> entries;
  entries.reserve(header->resource_count);

  for (std::size_t i{0}; i < header->resource_count; ++i) {
    auto const entry{EntrySerializer::Read(is, i)};

    if (!entry || !IsEntryValid(*entry, package_size)) {
      return std::nullopt;
    }

    entries.push_back(*entry);
  }

  return entries;
}


auto PeekBinaryResourcePackage(
  std::filesystem::path const& file_path_abs
) noexcept -> std::optional<ResourcePackageInfo> {
  try {
    auto const package_size{std::filesystem::file_size(file_path_abs)};
    std::ifstream is{file_path_abs, std::ios::binary};

    if (!is) {
      return std::nullopt;
    }

    auto const header{HeaderSerializer::Read(is)};

    if (!header) {
      return std::nullopt;
    }

    ResourcePackageInfo package_info;
    package_info.entries.reserve(header->resource_count);

    for (std::size_t i{0}; i < header->resource_count; i++) {
      auto const entry{EntrySerializer::Read(is, i)};

      if (!entry || !IsEntryValid(*entry, package_size)) {
        return std::nullopt;
      }

      auto const type{ToRttrType(entry->runtime_type)};

      if (!type) {
        return std::nullopt;
      }

      auto const last_pos = is.tellg();
      if (!is.seekg(entry->name_offset, std::ios::beg)) {
        return std::nullopt;
      }

      std::string name(entry->name_size, '\0');

      if (!is.read(name.data(), entry->name_size)) {
        return std::nullopt;
      }

      if (!is.seekg(last_pos, std::ios::beg)) {
        return std::nullopt;
      }

      package_info.entries.emplace_back(*type, std::move(name));
    }

    return package_info;
  } catch (...) {
    return std::nullopt;
  }
}


auto LoadBinaryResourcePackageSubresource(
  std::filesystem::path const& file_path_abs,
  std::size_t const entry_idx
) -> std::optional<ResourcePackageSubresource> {
  std::ifstream is{file_path_abs, std::ios::binary};

  if (!is || !is.seekg(0, std::ios::end)) {
    return std::nullopt;
  }

  auto const package_size{is.tellg()};

  auto const header{HeaderSerializer::Read(is)};

  if (!header || header->resource_count <= entry_idx) {
    return std::nullopt;
  }

  auto const entry{EntrySerializer::Read(is, entry_idx)};

  if (!entry || !IsEntryValid(*entry, package_size)) {
    return std::nullopt;
  }

  ResourcePackageSubresource subresource;
  subresource.payload_kind = entry->payload_kind;

  if (!is.seekg(entry->name_offset, std::ios::beg)) {
    return std::nullopt;
  }

  subresource.name.resize(entry->name_size);
  is.read(subresource.name.data(), static_cast<std::streamsize>(subresource.name.size()));


  if (!is.seekg(entry->data_offset, std::ios::beg)) {
    return std::nullopt;
  }

  subresource.bytes.resize(entry->data_size);
  is.read(reinterpret_cast<char*>(subresource.bytes.data()), static_cast<std::streamsize>(subresource.bytes.size()));

  return subresource;
}
}
