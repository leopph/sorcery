#include "sorcery/assets/Scene.hpp"

#include <algorithm>

#include "sorcery/asset_reference.hpp"
#include "sorcery/entity_serialization.hpp"
#include "sorcery/reflection.hpp"
#include "sorcery/world.hpp"
#include "sorcery/scene/Entity.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{"Scene"};
}


namespace sorcery {
Scene::Scene() {
  SaveVersionNumber();
}


auto Scene::Save(World const& world) -> void {
  yaml_data_.reset();
  SaveVersionNumber();

  auto const world_entities = world.GetEntities();

  std::vector<Entity const*> entities;
  entities.reserve(world_entities.size());
  std::ranges::transform(world_entities, std::back_inserter(entities), [](auto const& ptr) { return ptr.get(); });

  yaml_data_["sceneObjects"] = SerializeEntitySet(entities, EntitySerializationContext{
    .asset_ref_serialization = AssetRefSerialization::kGlobal
  });
}


auto Scene::Load(World& world) -> void {
  if (auto const version{yaml_data_["version"]}; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{
      std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName())
    };
  }

  // Add the new entities to the scene

  for (auto& entity_ptr : DeserializeEntitySet(yaml_data_["sceneObjects"], yaml_ctx_)) {
    world.AddEntity(std::move(entity_ptr));
  }
}


auto Scene::Serialize() const noexcept -> YAML::Node {
  return yaml_data_;
}


auto Scene::Deserialize(
  YAML::Node const& yaml_node,
  YamlDeserializeContext const& ctx
) noexcept -> void {
  yaml_data_ = Clone(yaml_node);
  yaml_ctx_ = ctx;
}


auto Scene::SaveVersionNumber() -> void {
  yaml_data_["version"] = 1;
}
}
