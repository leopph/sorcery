#include "sorcery/host/assets/asset_package.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <numeric>
#include <ranges>
#include <utility>

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


namespace {
std::uint32_t constexpr kCurrentPackageVersion{2};
}
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
           os.write(reinterpret_cast<char const* const>(&header.package_version), sizeof(header.package_version)) &&
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

    if (!is.read(reinterpret_cast<char*>(&header.package_version), sizeof(header.package_version))) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&header.asset_count), sizeof(header.asset_count))) {
      return std::nullopt;
    }

    return header;
  }


  constexpr static auto kHeaderSize =
    sizeof(decltype(asset_package::Header::magic)) +
    sizeof(decltype(asset_package::Header::package_version)) +
    sizeof(decltype(asset_package::Header::asset_count));
};


struct EntrySerializer {
  [[nodiscard]] static
  auto Write(asset_package::Entry const& entry, std::ostream& os) -> bool {
    return os.write(reinterpret_cast<char const* const>(&entry.id), sizeof(entry.id)) &&
           os.write(reinterpret_cast<char const* const>(&entry.version), sizeof(entry.version)) &&
           os.write(reinterpret_cast<char const* const>(&entry.payload_kind), sizeof(entry.payload_kind)) &&
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

    if (!is.read(reinterpret_cast<char*>(&entry.id), sizeof(entry.id))) {
      return std::nullopt;
    }

    if (!is.read(reinterpret_cast<char*>(&entry.version), sizeof(entry.version))) {
      return std::nullopt;
    }

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


  constexpr static auto kEntrySize =
    sizeof(decltype(asset_package::Entry::id)) +
    sizeof(decltype(asset_package::Entry::version)) +
    sizeof(asset_package::Entry::payload_kind) +
    sizeof(asset_package::Entry::runtime_type) +
    sizeof(asset_package::Entry::name_offset) +
    sizeof(asset_package::Entry::name_size) +
    sizeof(asset_package::Entry::data_offset) +
    sizeof(asset_package::Entry::data_size);
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


[[nodiscard]] auto ReadHeader(std::istream& is) -> std::expected<asset_package::Header, AssetPackageReadError> {
  auto const header = HeaderSerializer::Read(is);

  if (!header) {
    return std::unexpected{AssetPackageReadError::kFileError};
  }

  if (header->magic != asset_package::kMagic) {
    return std::unexpected{AssetPackageReadError::kInvalidData};
  }

  if (header->package_version != asset_package::kCurrentPackageVersion) {
    return std::unexpected{AssetPackageReadError::kPackageVersionMismatch};
  }

  return *header;
}


[[nodiscard]] auto ReadEntries(std::istream& is, asset_package::Header const& header,
                               std::size_t const package_size) -> std::expected<
  std::vector<asset_package::Entry>, AssetPackageReadError> {
  if (std::cmp_greater(HeaderSerializer::kHeaderSize + header.asset_count * EntrySerializer::kEntrySize,
    package_size)) {
    return std::unexpected{AssetPackageReadError::kInvalidData};
  }

  std::vector<asset_package::Entry> entries;
  entries.reserve(header.asset_count);

  for (auto i = 0uz; i < header.asset_count; ++i) {
    auto const entry = EntrySerializer::Read(is, i);

    if (!entry) {
      return std::unexpected{AssetPackageReadError::kFileError};
    }

    if (!IsEntryValid(*entry, package_size)) {
      return std::unexpected{AssetPackageReadError::kInvalidData};
    }

    entries.emplace_back(*entry);
  }

  for (auto i = 1uz; i < entries.size(); ++i) {
    if (entries[i - 1].id >= entries[i].id) {
      return std::unexpected{AssetPackageReadError::kInvalidData};
    }
  }

  return entries;
}
}


auto GetCurrentPayloadVersion(AssetPackagePayloadKind const payload_kind) -> std::uint32_t {
  switch (payload_kind) {
    case AssetPackagePayloadKind::kInvalid: [[fallthrough]];
    case AssetPackagePayloadKind::kTexture: [[fallthrough]];
    case AssetPackagePayloadKind::kMesh: [[fallthrough]];
    case AssetPackagePayloadKind::kMaterial: [[fallthrough]];
    case AssetPackagePayloadKind::kPrefab:
      return 1;
  }

  assert(false && "Unhandled AssetPackagePayloadKind in GetCurrentPayloadVersion");
  return 0;
}


auto PackBinaryAssetPackage(
  std::span<AssetImportResult const> const imports
) -> std::expected<std::vector<std::byte>, AssetPackageWriteError> {
  // Sort imports by id
  std::vector<std::size_t> import_indices(imports.size());
  std::ranges::iota(import_indices, 0uz);
  std::ranges::sort(import_indices, {}, [imports](auto const idx) { return imports[idx].id; });
  auto const sorted_imports = import_indices | std::views::transform([imports](auto const idx) -> auto& {
    return imports[idx];
  });

  // Make sure there are no duplicate ids
  if (std::ranges::adjacent_find(import_indices, {}, [imports](auto const idx) { return imports[idx].id; }) !=
      import_indices.end()) {
    return std::unexpected{AssetPackageWriteError::kDuplicateEntryIds};
  }

  std::vector<std::byte> package_bytes;
  ByteVectorOstream os{package_bytes};

  // Generate header

  asset_package::Header const header{
    .magic = asset_package::kMagic,
    .package_version = asset_package::kCurrentPackageVersion,
    .asset_count = static_cast<std::uint32_t>(sorted_imports.size())
  };

  // Write header

  if (!HeaderSerializer::Write(header, os)) {
    return std::unexpected{AssetPackageWriteError::kIoError};
  }

  // Generate entires

  std::vector<asset_package::Entry> entries;

  auto const name_table_offset = ComputeNameTableOffset(sorted_imports.size());
  std::uint64_t name_table_size{0};

  for (auto const& import_data : sorted_imports) {
    auto const runtime_type = ToRuntimeType(import_data.runtime_type);

    if (!runtime_type) {
      return std::unexpected{AssetPackageWriteError::kUnknownRuntimeType};
    }

    // Data offsets will be filled in later when we know the size of the name table
    entries.emplace_back(import_data.id, import_data.version, import_data.payload_kind, *runtime_type,
      name_table_offset + name_table_size, import_data.name.size(), 0, import_data.bytes.size());
    name_table_size += import_data.name.size();
  }

  // Compute data offsets

  auto const data_table_offset{name_table_offset + name_table_size};
  std::uint64_t data_table_size{0};

  for (auto& entry : entries) {
    entry.data_offset = data_table_offset + data_table_size;
    data_table_size += entry.data_size;
  }

  for (auto const& entry : entries) {
    if (!EntrySerializer::Write(entry, os)) {
      return std::unexpected{AssetPackageWriteError::kIoError};
    }
  }

  for (auto const& import_data : sorted_imports) {
    std::ranges::copy_n(reinterpret_cast<std::byte const*>(import_data.name.data()), import_data.name.size(),
      std::back_inserter(package_bytes));
  }

  for (auto const& import_data : sorted_imports) {
    std::ranges::copy_n(import_data.bytes.begin(), import_data.bytes.size(), std::back_inserter(package_bytes));
  }

  return package_bytes;
}


auto UnpackBinaryAssetPackageEntries(
  std::span<std::byte const> const file_bytes
) -> std::expected<std::vector<asset_package::Entry>, AssetPackageReadError> {
  auto const package_size = file_bytes.size();
  ByteSpanIstream is{file_bytes};

  auto const header = ReadHeader(is);

  if (!header) {
    return std::unexpected{header.error()};
  }

  return ReadEntries(is, *header, package_size);
}


auto PeekBinaryAssetPackage(
  std::filesystem::path const& file_path_abs
) -> std::expected<AssetPackageInfo, AssetPackageReadError> {
  try {
    auto const package_size = std::filesystem::file_size(file_path_abs);
    std::ifstream is{file_path_abs, std::ios::binary};

    if (!is) {
      return std::unexpected{AssetPackageReadError::kFileError};
    }

    auto const header = ReadHeader(is);

    if (!header) {
      return std::unexpected{header.error()};
    }

    auto const entries = ReadEntries(is, *header, package_size);

    if (!entries) {
      return std::unexpected{entries.error()};
    }

    AssetPackageInfo package_info;
    package_info.entries.reserve(entries->size());

    for (auto const& entry : *entries) {
      if (entry.version != GetCurrentPayloadVersion(entry.payload_kind)) {
        return std::unexpected{AssetPackageReadError::kPayloadVersionMismatch};
      }

      auto const type = ToRttrType(entry.runtime_type);

      if (!type) {
        return std::unexpected{AssetPackageReadError::kInvalidData};
      }

      if (!is.seekg(entry.name_offset, std::ios::beg)) {
        return std::unexpected{AssetPackageReadError::kFileError};
      }

      std::string name(entry.name_size, '\0');

      if (!is.read(name.data(), entry.name_size)) {
        return std::unexpected{AssetPackageReadError::kFileError};
      }

      package_info.entries.emplace_back(*type, std::move(name), entry.id);
    }

    return package_info;
  } catch (...) {
    return std::unexpected{AssetPackageReadError::kException};
  }
}


auto LoadBinaryAssetPackageSubasset(
  std::filesystem::path const& file_path_abs,
  std::uint64_t const subasset_id
) -> std::expected<AssetPackageSubasset, AssetPackageReadError> {
  std::ifstream is{file_path_abs, std::ios::binary};

  if (!is || !is.seekg(0, std::ios::end)) {
    return std::unexpected{AssetPackageReadError::kFileError};
  }

  auto const package_size = is.tellg();

  auto const header = ReadHeader(is);

  if (!header) {
    return std::unexpected{header.error()};
  }

  auto const entries = ReadEntries(is, *header, package_size);

  if (!entries) {
    return std::unexpected{entries.error()};
  }

  auto const entry = std::ranges::lower_bound(*entries, subasset_id, {}, &asset_package::Entry::id);

  if (entry == entries->end() || entry->id != subasset_id) {
    return std::unexpected{AssetPackageReadError::kSubassetNotFound};
  }

  if (entry->version != GetCurrentPayloadVersion(entry->payload_kind)) {
    return std::unexpected{AssetPackageReadError::kPayloadVersionMismatch};
  }

  AssetPackageSubasset subasset;
  subasset.payload_kind = entry->payload_kind;

  if (!is.seekg(entry->name_offset, std::ios::beg)) {
    return std::unexpected{AssetPackageReadError::kFileError};
  }

  subasset.name.resize(entry->name_size);
  if (!is.read(subasset.name.data(), static_cast<std::streamsize>(subasset.name.size()))) {
    return std::unexpected{AssetPackageReadError::kFileError};
  }


  if (!is.seekg(entry->data_offset, std::ios::beg)) {
    return std::unexpected{AssetPackageReadError::kFileError};
  }

  subasset.bytes.resize(entry->data_size);
  if (!is.read(reinterpret_cast<char*>(subasset.bytes.data()),
    static_cast<std::streamsize>(subasset.bytes.size()))) {
    return std::unexpected{AssetPackageReadError::kFileError};
  }

  return subasset;
}
}
