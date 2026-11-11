#include "identity.hpp"

#include <cassert>
#include <numeric>
#include <ranges>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include <spdlog/spdlog.h>

#include "hash.hpp"
#include "sorcery/char_encoding_helpers.hpp"


namespace sorcery::mage::model_import {
namespace {
auto HashKey(std::string_view const key) -> std::uint64_t {
  return Mix64(Fnv1a64(std::as_bytes(std::span{key})));
}


// Appends "#n" to repeated keys so that every key is unique. The first occurrence keeps its key.
auto MakeKeysUnique(std::span<std::string> const keys) -> void {
  std::unordered_set<std::string> used;
  used.reserve(keys.size());

  for (auto& key : keys) {
    if (used.insert(key).second) {
      continue;
    }

    for (auto n = 1uz;; ++n) {
      if (auto candidate = std::format("{}#{}", key, n); used.insert(candidate).second) {
        key = std::move(candidate);
        break;
      }
    }
  }
}
}


auto SubassetIds::FromKeys(SubassetKeys const& keys, std::filesystem::path const& src) -> SubassetIds {
  SubassetIds ids;
  constexpr std::uint64_t prefab_id{0};

  std::vector<std::string_view> flat_keys;
  flat_keys.append_range(keys.meshes);
  flat_keys.append_range(keys.materials);
  flat_keys.append_range(keys.textures);

  std::vector<std::uint64_t> flat_ids(flat_keys.size());

  std::vector<std::size_t> order(flat_keys.size());
  std::ranges::iota(order, 0uz);
  std::ranges::sort(order, {}, [&](auto const idx) { return flat_keys[idx]; });

  std::unordered_map<std::uint64_t, std::size_t> key_idx_by_id;

  auto const is_id_taken = [&](auto const id) {
    return id == prefab_id || key_idx_by_id.contains(id);
  };

  for (auto const i : order) {
    auto const key = flat_keys[i];
    auto const base = HashKey(key);
    auto id = base;

    for (std::uint64_t salt{1}; is_id_taken(id); ++salt) {
      id = Mix64(base ^ salt);
    }

    if (id != base) {
      auto const kept_key = base == prefab_id
                              ? std::string_view{keys.prefab}
                              : flat_keys.at(key_idx_by_id.at(base));

      spdlog::warn(
        "Subasset key hash collision in [{}]: \"{}\" collides with \"{}\". "
        "Assigned salted ID {:#018x} to \"{}\"; its ID may change if keys are added or removed.",
        ToUntypedSv(src.u8string()), key, kept_key, id, key);
    }

    key_idx_by_id.emplace(id, i);
    flat_ids[i] = id;
  }

  ids.prefab_id_ = prefab_id;
  ids.mesh_ids_.assign_range(std::span{flat_ids}.subspan(0, keys.meshes.size()));
  ids.mtl_ids_.assign_range(std::span{flat_ids}.subspan(keys.meshes.size(), keys.materials.size()));
  ids.tex_ids_.assign_range(std::span{flat_ids}.subspan(keys.meshes.size() + keys.materials.size(),
    keys.textures.size()));

  return ids;
}


auto SubassetIds::GetPrefabId() const -> std::uint64_t {
  return prefab_id_;
}


auto SubassetIds::GetMeshId(std::size_t const mesh_idx) const -> std::uint64_t {
  assert(mesh_idx < mesh_ids_.size());
  return mesh_ids_[mesh_idx];
}


auto SubassetIds::GetMaterialId(std::size_t const mtl_idx) const -> std::uint64_t {
  assert(mtl_idx < mtl_ids_.size());
  return mtl_ids_[mtl_idx];
}


auto SubassetIds::GetTextureId(std::size_t const tex_idx) const -> std::uint64_t {
  assert(tex_idx < tex_ids_.size());
  return tex_ids_[tex_idx];
}


auto SubassetIds::GetMaterialCount() const -> std::size_t {
  return mtl_ids_.size();
}


auto MakeMeshGroupLabels(
  SceneNodeList const& nodes,
  AnchorNodeByGroup const& anchor_node_by_group,
  std::filesystem::path const& src
) -> SubassetLabels {
  // The absolute path of node i
  auto node_paths = std::vector<std::string>(nodes.size());
  // The last segment of node i's path, for example "child[2]" in "/grandparent/parent/child[2]"
  auto node_segments = std::vector<std::string>(nodes.size());
  // How many children does node i have with a specific name
  auto name_counts = std::vector<std::unordered_map<std::string_view, std::size_t>>(nodes.size());
  // How many groups does node i reference
  auto group_counts = std::vector<std::size_t>(nodes.size());

  auto const stem = ToUntypedStr(src.stem().u8string());

  // Build the absolute path of each node, and count how many children of each name each node has.
  // The root path is "", every other path is "/grandparent/parent/child[n]", where n deduplicates
  // children with the same name.
  for (auto const [i, node] : std::views::enumerate(nodes)) {
    // Leave the root node's path empty as it is fairly volatile.
    // For example, some importers change it based on the input file name.
    if (!node.parent_idx) {
      continue;
    }

    auto const node_name = std::string_view{node.ptr->mName.C_Str()};
    auto const name_count = name_counts[*node.parent_idx][node_name]++;
    auto const& parent_path = node_paths[*node.parent_idx];
    node_segments[i] = name_count == 0
                         ? std::string{node_name}
                         : std::format("{}[{}]", node_name, name_count);
    node_paths[i] = std::format("{}/{}", parent_path, node_segments[i]);
  }

  auto labels = SubassetLabels{};

  for (auto const& anchor_node_opt : anchor_node_by_group) {
    if (!anchor_node_opt) {
      // Every unreferenced mesh is grouped into a single group,
      // so at most one group is unreferenced. This is for that case.
      labels.keys.emplace_back("mesh:unreferenced");
      labels.names.emplace_back(std::format("{} ({})", stem, "unreferenced"));
      continue;
    }

    auto const anchor_node_idx = *anchor_node_opt;
    auto const group_count = group_counts[anchor_node_idx]++;
    auto const anchor_node_path = std::string_view{node_paths[anchor_node_idx]};

    labels.keys.emplace_back(group_count == 0
                               ? std::format("mesh:{}", anchor_node_path)
                               : std::format("mesh:{}:{}", anchor_node_path, group_count));

    auto const& node = nodes[anchor_node_idx];

    // Groups on the root node get the file stem as their name.
    // If the root references multiple groups, they will be delimited using stem (1), stem (2), etc.
    if (!node.parent_idx) {
      labels.names.emplace_back(group_count == 0
                                  ? std::string{stem}
                                  : std::format("{} ({})", stem, group_count));
      continue;
    }

    // Groups on the non-root nodes get the last segment of the node path as their name.
    // Also deduped using (1), (2), etc.

    auto const node_segment = std::string_view{node_segments[anchor_node_idx]};

    // If the node name was empty, we append its last segment (which is only a dedup index) to the file stem for the name.
    // Otherwise the last segment is used as the name. A dedup suffix is added if the node references multiple groups.

    auto const base_name = node.ptr->mName.length == 0
                             ? std::format("{}{}", stem, node_segment)
                             : std::string{node_segment};

    labels.names.emplace_back(group_count == 0
                                ? base_name
                                : std::format("{} ({})", base_name, group_count));
  }

  MakeKeysUnique(labels.keys);
  return labels;
}


auto MakeMaterialKeys(std::span<MaterialSlotInfo const> const mtl_slots) -> std::vector<std::string> {
  auto keys = mtl_slots |
              std::views::transform([](MaterialSlotInfo const& slot) {
                return std::format("material:{}", slot.name);
              }) |
              std::ranges::to<std::vector>();
  MakeKeysUnique(keys);
  return keys;
}


auto MakeTextureLabels(
  std::span<SourceTexture const> const source_texes,
  std::filesystem::path const& src
) -> SubassetLabels {
  SubassetLabels labels;

  for (auto const [idx, src_tex] : std::views::enumerate(source_texes)) {
    auto const& tex = *src_tex.tex;

    if (!tex.mFilename.Empty()) {
      labels.keys.emplace_back(std::format("texture:{}", tex.mFilename.C_Str()));
      labels.names.emplace_back(tex.mFilename.C_Str());
      continue;
    }

    // Compressed textures store their byte count in mWidth, uncompressed ones are mWidth * mHeight texels
    auto const byte_count = tex.mHeight == 0
                              ? std::size_t{tex.mWidth}
                              : std::size_t{tex.mWidth} * tex.mHeight * sizeof(aiTexel);

    labels.keys.emplace_back(std::format("texture:{:016x}",
      Fnv1a64(std::span{reinterpret_cast<std::byte const*>(tex.pcData), byte_count})));
    labels.names.emplace_back(std::format("{} Texture {}", ToUntypedSv(src.filename().stem().u8string()), idx));
  }

  MakeKeysUnique(labels.keys);
  return labels;
}
}
