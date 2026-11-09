#include "prefab.hpp"

#include <ranges>

#include "sorcery/assets/prefab.hpp"
#include "scene.hpp"
#include "sorcery/entity_serialization.hpp"
#include "sorcery/assets/Material.hpp"
#include "sorcery/assets/Mesh.hpp"
#include "sorcery/scene/Entity.hpp"
#include "sorcery/scene/StaticMeshComponent.hpp"


namespace sorcery::mage::model_import {
namespace {
[[nodiscard]]
auto SerializePrefab(std::span<std::unique_ptr<Entity> const> const entities) -> std::vector<std::byte> {
  auto entity_ptrs = entities |
                     std::views::transform([](auto const& ptr) { return ptr.get(); }) |
                     std::ranges::to<std::vector<Entity const*>>();

  auto const prefab_node{
    SerializeEntitySet(entity_ptrs, EntitySerializationContext{
      .asset_ref_serialization = AssetRefSerialization::kLocal
    })
  };

  auto const yaml_str{YAML::Dump(prefab_node)};

  std::vector<std::byte> prefab_bytes;
  prefab_bytes.resize(yaml_str.size());
  std::ranges::copy(yaml_str | std::views::transform([](char c) { return static_cast<std::byte>(c); }),
    prefab_bytes.begin());

  return prefab_bytes;
}
}


auto CreateHierarchyPrefab(std::string_view const name, std::span<MeshData const> const mesh_groups,
                           MaterialSlotsByMesh const& mtl_slots_by_mesh,
                           HierarchyLayout const& hierarchy, SubassetIds const& ids) -> AssetImportResult {
  // Create dummies for all mesh groups

  std::vector<std::unique_ptr<Mesh>> dummy_meshes(mesh_groups.size());
  for (auto const [group_idx, group] : std::views::enumerate(mesh_groups)) {
    // The only thing we care about in a dummy mesh is its material slot count
    MeshData const mesh_data{
      .positions = {},
      .normals = {},
      .tangents = {},
      .uvs = {},
      .bone_weights = {},
      .bone_indices = {},
      .meshlets = {},
      .vertex_indices = {},
      .triangle_indices = {},
      .cull_data = {},
      .material_slots = mesh_groups[group_idx].material_slots,
      .submeshes = {},
      .animations = {},
      .skeleton = {},
      .bones = {},
      .bounds = {},
      .idx32 = {}
    };

    auto dummy_mesh = std::make_unique<Mesh>(mesh_data, CpuResidencyPolicy::kKeepResident);
    dummy_mesh->SetAssetId(AssetId{Guid::Invalid(), ids.GetMeshId(group_idx)});
    dummy_meshes[group_idx] = std::move(dummy_mesh);
  }

  // Create dummies for all materials

  std::vector<std::unique_ptr<Material>> dummy_materials(ids.GetMaterialCount());
  for (auto i = 0uz; i < ids.GetMaterialCount(); ++i) {
    auto dummy_mtl = std::make_unique<Material>();
    dummy_mtl->SetAssetId(AssetId{Guid::Invalid(), ids.GetMaterialId(i)});
    dummy_materials[i] = std::move(dummy_mtl);
  }

  // Create the entity hierarchy

  std::vector<std::unique_ptr<Entity>> entities(hierarchy.nodes.size());

  for (auto const [node_idx, node] : std::views::enumerate(hierarchy.nodes)) {
    auto entity = std::make_unique<Entity>();

    auto const entity_name = node.ptr->mName.Empty()
                               ? std::format("Node {}", node_idx)
                               : std::string{node.ptr->mName.C_Str()};
    entity->SetName(entity_name);

    aiVector3f position;
    aiQuaternion rotation;
    aiVector3f scaling;
    node.ptr->mTransformation.Decompose(scaling, rotation, position);

    auto& transform = entity->GetTransform();
    transform.SetLocalPosition(Convert(position));
    transform.SetLocalRotation(Convert(rotation));
    transform.SetLocalScale(Convert(scaling));

    if (node.parent_idx) {
      transform.SetParent(&entities[*node.parent_idx]->GetTransform());
    }

    for (auto const& group_idx : hierarchy.groups_by_node[node_idx]) {
      auto mesh_component{std::make_unique<StaticMeshComponent>()};

      mesh_component->SetMesh(MakeAssetRef(MakeObserver(dummy_meshes[group_idx].get())));

      if (!dummy_materials.empty()) {
        for (auto i = 0uz; i < mesh_groups[group_idx].material_slots.size(); ++i) {
          mesh_component->SetMaterial(static_cast<int>(i),
            MakeAssetRef(MakeObserver(dummy_materials.at(mtl_slots_by_mesh.at(group_idx).at(i)).get())));
        }
      }

      entity->AddComponent(std::move(mesh_component));
    }

    entities[node_idx] = std::move(entity);
  }

  if (!entities.empty()) {
    // Rename the root of the prefab to the prefab asset name
    entities[0]->SetName(name);
  }

  return AssetImportResult{
    .id = ids.GetPrefabId(),
    .version = GetCurrentPayloadVersion(AssetPackagePayloadKind::kPrefab),
    .payload_kind = AssetPackagePayloadKind::kPrefab,
    .runtime_type = rttr::type::get<Prefab>(),
    .name = std::string{name},
    .bytes = SerializePrefab(entities)
  };
}


auto CreateSingleEntityPrefab(std::string_view const name, MeshData const& mesh,
                              std::span<SceneMaterialIndex const> const mesh_mtl_slot_remap,
                              SubassetIds const& ids) -> AssetImportResult {
  auto const entity{std::make_unique<Entity>()};
  entity->SetName(name);

  auto mesh_component{std::make_unique<StaticMeshComponent>()};

  auto const dummy_mesh = std::make_unique<Mesh>(mesh, CpuResidencyPolicy::kKeepResident);
  dummy_mesh->SetAssetId(AssetId{Guid::Invalid(), ids.GetMeshId(0)});

  mesh_component->SetMesh(MakeAssetRef(MakeObserver(dummy_mesh.get())));

  std::vector<std::unique_ptr<Material>> dummy_materials;
  dummy_materials.reserve(ids.GetMaterialCount());

  for (auto i = 0uz; i < ids.GetMaterialCount(); i++) {
    auto dummy_mtl{std::make_unique<Material>()};
    dummy_mtl->SetAssetId(AssetId{Guid::Invalid(), ids.GetMaterialId(i)});
    dummy_materials.emplace_back(std::move(dummy_mtl));
  }

  if (!dummy_materials.empty()) {
    for (auto i = 0uz; i < mesh.material_slots.size(); ++i) {
      mesh_component->SetMaterial(static_cast<int>(i),
        MakeAssetRef(MakeObserver(dummy_materials.at(mesh_mtl_slot_remap[i]).get())));
    }
  }

  entity->AddComponent(std::move(mesh_component));

  return AssetImportResult{
    .id = ids.GetPrefabId(),
    .version = GetCurrentPayloadVersion(AssetPackagePayloadKind::kPrefab),
    .payload_kind = AssetPackagePayloadKind::kPrefab,
    .runtime_type = rttr::type::get<Prefab>(),
    .name = std::string{entity->GetName()},
    .bytes = SerializePrefab(std::span{&entity, 1})
  };
}
}
