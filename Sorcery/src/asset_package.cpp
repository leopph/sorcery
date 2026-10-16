#include "sorcery/host/assets/asset_package.hpp"

#include <algorithm>
#include <fstream>

#include "vector_stream.hpp"
#include "sorcery/Serialization.hpp"
#include "sorcery/assets/Cubemap.hpp"
#include "sorcery/assets/Material.hpp"
#include "sorcery/assets/Mesh.hpp"
#include "sorcery/assets/prefab.hpp"
#include "sorcery/assets/Scene.hpp"
#include "sorcery/assets/Texture2D.hpp"


namespace sorcery {
namespace asset_package {
std::uint32_t const kMagic{0x19991108};
}


namespace {
[[nodiscard]] auto ToRuntimeType(rttr::type const& type) -> std::optional<AssetRuntimeType> {
  if (type == rttr::type::get<Material>()) {
    return AssetRuntimeType::kMaterial;
  }
  if (type == rttr::type::get<Mesh>()) {
    return AssetRuntimeType::kMesh;
  }
  if (type == rttr::type::get<Scene>()) {
    return AssetRuntimeType::kScene;
  }
  if (type == rttr::type::get<Texture2D>()) {
    return AssetRuntimeType::kTexture2D;
  }
  if (type == rttr::type::get<Cubemap>()) {
    return AssetRuntimeType::kCubemap;
  }
  if (type == rttr::type::get<Prefab>()) {
    return AssetRuntimeType::kPrefab;
  }

  return std::nullopt;
}


[[nodiscard]] auto ToRttrType(AssetRuntimeType const type) -> std::optional<rttr::type> {
  switch (type) {
    case AssetRuntimeType::kMaterial:
      return rttr::type::get<Material>();
    case AssetRuntimeType::kMesh:
      return rttr::type::get<Mesh>();
    case AssetRuntimeType::kScene:
      return rttr::type::get<Scene>();
    case AssetRuntimeType::kTexture2D:
      return rttr::type::get<Texture2D>();
    case AssetRuntimeType::kCubemap:
      return rttr::type::get<Cubemap>();
    case AssetRuntimeType::kPrefab:
      return rttr::type::get<Prefab>();
  }

  return std::nullopt;
}


struct HeaderSerializer {
  [[nodiscard]] static
  auto Write(asset_package::Header const& header, std::ostream& os) -> bool {
    return os.write(reinterpret_cast<char const* const>(&header.magic), sizeof(header.magic)) &&
           os.write(reinterpret_cast<char const* const>(&header.version), sizeof(header.version)) &&
           os.write(reinterpret_cast<char const* const>(&header.asset_count), sizeof(header.asset_count));
  }


  [[nodiscard]] static
  auto Read(std::istream& is) -> std::optional<asset_package::Header> {
    if (!is.seekg(0, std::ios::beg)) {
      return std::nullopt;
    }

    asset_package::Header header;

    if (!is.read(reinterpret_cast<char*>(&header.magic), sizeof(header.magic))) {
      return std::nullopt;
    }

    if (header.magic != asset_package::kMagic) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&header.version), sizeof(header.version))) {
      return std::nullopt;
    }

    if (header.version != 1) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&header.asset_count), sizeof(header.asset_count))) {
      return std::nullopt;
    }

    return header;
  }


  constexpr static auto kHeaderSize{3 * sizeof(std::uint32_t)};
};


struct EntrySerializer {
  [[nodiscard]] static
  auto Write(asset_package::Entry const& entry, std::ostream& os) -> bool {
    return os.write(reinterpret_cast<char const* const>(&entry.payload_kind), sizeof(entry.payload_kind)) &&
           os.write(reinterpret_cast<char const* const>(&entry.runtime_type), sizeof(entry.runtime_type)) &&
           os.write(reinterpret_cast<char const* const>(&entry.name_offset), sizeof(entry.name_offset)) &&
           os.write(reinterpret_cast<char const* const>(&entry.name_size), sizeof(entry.name_size)) &&
           os.write(reinterpret_cast<char const* const>(&entry.data_offset), sizeof(entry.data_offset)) &&
           os.write(reinterpret_cast<char const* const>(&entry.data_size), sizeof(entry.data_size));
  }


  [[nodiscard]] static
  auto Read(std::istream& is, std::size_t const idx) -> std::optional<asset_package::Entry> {
    if (!is.seekg(HeaderSerializer::kHeaderSize + idx * kEntrySize, std::ios::beg)) {
      return std::nullopt;
    }

    asset_package::Entry entry;

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
    sizeof(AssetPackagePayloadKind) + sizeof(AssetRuntimeType) + 4 * sizeof(std::uint64_t)
  };
};


[[nodiscard]] auto ComputeNameTableOffset(std::size_t const subasset_count) -> std::size_t {
  return HeaderSerializer::kHeaderSize + EntrySerializer::kEntrySize * subasset_count;
}


[[nodiscard]] auto IsEntryValid(asset_package::Entry const& entry, std::size_t const package_size) -> bool {
  return !(entry.name_offset > package_size ||
           entry.name_size > package_size - entry.name_offset ||
           entry.data_offset > package_size ||
           entry.data_size > package_size - entry.data_offset);
}
}


auto PackBinaryAssetPackage(
  std::span<AssetImportResult const> const imports
) -> std::optional<std::vector<std::byte>> {
  std::vector<std::byte> package_bytes;
  ByteVectorOstream os{package_bytes};

  asset_package::Header const header{
    .magic = asset_package::kMagic,
    .version = 1,
    .asset_count = static_cast<std::uint32_t>(imports.size())
  };

  if (!HeaderSerializer::Write(header, os)) {
    return std::nullopt;
  }

  std::vector<asset_package::Entry> entries;

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
    if (!EntrySerializer::Write(entry, os)) {
      return std::nullopt;
    }
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


auto UnpackBinaryAssetPackageEntries(
  std::span<std::byte const> file_bytes
) -> std::optional<std::vector<asset_package::Entry>> {
  auto const package_size{file_bytes.size()};
  ByteSpanIstream is{file_bytes};

  auto const header{HeaderSerializer::Read(is)};

  if (!header) {
    return std::nullopt;
  }

  std::vector<asset_package::Entry> entries;
  entries.reserve(header->asset_count);

  for (std::size_t i{0}; i < header->asset_count; ++i) {
    auto const entry{EntrySerializer::Read(is, i)};

    if (!entry || !IsEntryValid(*entry, package_size)) {
      return std::nullopt;
    }

    entries.push_back(*entry);
  }

  return entries;
}


auto PeekBinaryAssetPackage(
  std::filesystem::path const& file_path_abs
) -> std::optional<AssetPackageInfo> {
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

    AssetPackageInfo package_info;
    package_info.entries.reserve(header->asset_count);

    for (std::size_t i{0}; i < header->asset_count; i++) {
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


auto LoadBinaryAssetPackageSubasset(
  std::filesystem::path const& file_path_abs,
  std::size_t const entry_idx
) -> std::optional<AssetPackageSubasset> {
  std::ifstream is{file_path_abs, std::ios::binary};

  if (!is || !is.seekg(0, std::ios::end)) {
    return std::nullopt;
  }

  auto const package_size{is.tellg()};

  auto const header{HeaderSerializer::Read(is)};

  if (!header || header->asset_count <= entry_idx) {
    return std::nullopt;
  }

  auto const entry{EntrySerializer::Read(is, entry_idx)};

  if (!entry || !IsEntryValid(*entry, package_size)) {
    return std::nullopt;
  }

  AssetPackageSubasset subasset;
  subasset.payload_kind = entry->payload_kind;

  if (!is.seekg(entry->name_offset, std::ios::beg)) {
    return std::nullopt;
  }

  subasset.name.resize(entry->name_size);
  if (!is.read(subasset.name.data(), static_cast<std::streamsize>(subasset.name.size()))) {
    return std::nullopt;
  }


  if (!is.seekg(entry->data_offset, std::ios::beg)) {
    return std::nullopt;
  }

  subasset.bytes.resize(entry->data_size);
  if (!is.read(reinterpret_cast<char*>(subasset.bytes.data()),
    static_cast<std::streamsize>(subasset.bytes.size()))) {
    return std::nullopt;
  }

  return subasset;
}
}
