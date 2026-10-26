#include "sorcery/assets/Scene.hpp"

#include <queue>

#include "sorcery/asset_reference.hpp"
#include "sorcery/entity_serialization.hpp"
#include "sorcery/reflection.hpp"
#include "sorcery/scene/Entity.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{"Scene"};
}


namespace sorcery {
auto Scene::Serialize() const noexcept -> YAML::Node {
  return yaml_data_;
}


auto Scene::Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void {
  yaml_data_ = Clone(yaml_node);
  yaml_ctx_ = ctx;
}


Scene::Scene() {
  SaveVersionNumber();
}


auto Scene::SetEntities(std::span<Entity const* const> const entities) -> void {
  yaml_data_.reset();
  SaveVersionNumber();

  yaml_data_["sceneObjects"] = SerializeEntitySet(entities, EntitySerializationContext{
    .asset_ref_serialization = AssetRefSerialization::kGlobal
  });
}


auto Scene::SetEntityHierarchy(std::span<Entity const* const> const root_entities) -> void {
  std::vector<Entity const*> all_entities;

  std::queue<Entity const*> entity_queue;
  entity_queue.push_range(root_entities);

  while (!entity_queue.empty()) {
    auto const* const entity = entity_queue.front();
    entity_queue.pop();

    all_entities.emplace_back(entity);

    for (auto const* const child : entity->GetTransform().GetChildren()) {
      entity_queue.emplace(child->GetEntity());
    }
  }

  SetEntities(all_entities);
}


auto Scene::Instantiate() const -> std::vector<std::unique_ptr<Entity>> {
  if (auto const version{yaml_data_["version"]}; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{
      std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName())
    };
  }

  return DeserializeEntitySet(yaml_data_["sceneObjects"], yaml_ctx_);
}


auto Scene::SaveVersionNumber() -> void {
  yaml_data_["version"] = 1;
}
}
