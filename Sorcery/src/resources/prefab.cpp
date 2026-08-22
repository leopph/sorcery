#include "prefab.hpp"

#include <queue>

#include "../scene_objects/Entity.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Prefab>{"Prefab"};
}


namespace sorcery {
auto Prefab::Serialize() const noexcept -> YAML::Node {
  return node_;
}


auto Prefab::Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void {
  node_ = yaml_node;
  ctx_ = ctx;
}


auto Prefab::CaptureEntitySet(
  std::span<Entity const* const> const entities,
  EntitySerializationContext const& ctx
) -> void {
  node_ = SerializeEntitySet(entities, ctx);
}


auto Prefab::CaptureEntityHierarchy(
  std::span<Entity const* const> roots,
  EntitySerializationContext const& ctx
) -> void {
  std::vector<Entity const*> entity_list;

  std::queue<Entity const*> entity_queue;
  entity_queue.push_range(roots);

  while (!entity_queue.empty()) {
    auto const* const entity{entity_queue.front()};
    entity_queue.pop();

    entity_list.emplace_back(entity);

    for (auto const* const child : entity->GetTransform().GetChildren()) {
      entity_queue.emplace(child->GetEntity());
    }
  }

  CaptureEntitySet(entity_list, ctx);
}


auto Prefab::Instantiate() const -> std::vector<std::unique_ptr<Entity>> {
  return DeserializeEntitySet(node_, ctx_);
}
}
