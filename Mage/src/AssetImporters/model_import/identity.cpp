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


auto MakeMeshGroupKeys(NodeListPreorderDfs const& nodes, NodesByMesh const& nodes_by_mesh,
                       MeshesByGroup const& meshes_by_group) -> std::vector<std::string> {
  // The absolute path of node i
  std::vector<std::string> node_paths(nodes.size());
  // How many children does node i have with a specific name
  std::vector<std::unordered_map<std::string_view, std::size_t>> name_counts(nodes.size());
  // How many groups does node i reference
  std::vector<std::size_t> group_counts(nodes.size());

  // Build the absolute path of each node, and count how many children of each name each node has.
  // The root path is "", every other path is "/grandparent/parent/child[n]", where n deduplicates
  // children with the same name.
  for (auto const [i, node] : std::views::enumerate(nodes)) {
    // Leave the root node's path empty as it is fairly volatile.
    // For example, some importers change it based on the input file name.
    if (!node.parent_idx) {
      continue;
    }

    std::string_view const node_name{node.ptr->mName.C_Str()};
    auto const name_count = name_counts[*node.parent_idx][node_name]++;
    auto const& parent_path = node_paths[*node.parent_idx];
    node_paths[i] = std::format("{}/{}{}", parent_path, node_name,
      name_count == 0 ? std::string{} : std::format("[{}]", name_count));
  }

  auto keys = meshes_by_group | std::views::transform([&](auto const& meshes) {
    auto const first_mesh_idx = meshes.front();
    auto const& mesh_nodes = nodes_by_mesh.at(first_mesh_idx);

    if (mesh_nodes.empty()) {
      // Every unreferenced mesh is grouped into a single group,
      // so at most one group is unreferenced. This is for that case.
      return std::string{"mesh:unreferenced"};
    }

    auto const first_node_idx = mesh_nodes.front();
    auto const group_count = group_counts[first_node_idx]++;

    return group_count == 0
             ? std::format("mesh:{}", node_paths[first_node_idx])
             : std::format("mesh:{}:{}", node_paths[first_node_idx], group_count);
  }) | std::ranges::to<std::vector<std::string>>();
  MakeKeysUnique(keys);
  return keys;
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


auto MakeTextureKeys(std::span<TextureLoadInfo const> const tex_infos) -> std::vector<std::string> {
  auto keys = tex_infos | std::views::transform([](TextureLoadInfo const& info) {
    auto const& tex = *info.tex;

    if (!tex.mFilename.Empty()) {
      return std::format("texture:{}", tex.mFilename.C_Str());
    }

    // Compressed textures store their byte count in mWidth, uncompressed ones are mWidth * mHeight texels
    auto const byte_count = tex.mHeight == 0
                              ? std::size_t{tex.mWidth}
                              : std::size_t{tex.mWidth} * tex.mHeight * sizeof(aiTexel);

    return std::format("texture:{:016x}",
      Fnv1a64(std::span{reinterpret_cast<std::byte const*>(tex.pcData), byte_count}));
  }) | std::ranges::to<std::vector>();

  MakeKeysUnique(keys);
  return keys;
}
}
