#include "render_instance_registry.hpp"

#include <algorithm>
#include <cassert>

#include "../object_registry.hpp"


namespace sorcery::rendering {
// ReSharper disable CppDefinitionsOrder
template<>
auto RenderInstanceRegistry::CreateInstance<
  StaticRenderMeshInstance>() const -> std::unique_ptr<StaticRenderMeshInstance> {
  return std::make_unique<StaticRenderMeshInstance>();
}


template<>
auto RenderInstanceRegistry::CreateInstance<
  SkinnedRenderMeshInstance>() const -> std::unique_ptr<SkinnedRenderMeshInstance> {
  return std::make_unique<SkinnedRenderMeshInstance>();
}


template<typename T>
auto RenderInstanceRegistry::CreateOrGetInstance(ObjectId const& id) -> QueryResult<T> {
  assert(id.IsValid());

  if (associations_.size() <= id.idx) {
    associations_.resize(id.idx + 1);
  }

  auto& assoc = associations_[id.idx];

  if (std::holds_alternative<std::monostate>(assoc.instance) || assoc.generation != id.gen) {
    auto inst = CreateInstance<T>();
    auto const obs = MakeObserver(inst.get());
    assoc.generation = id.gen;
    assoc.instance = std::move(inst);
    return {obs, true};
  }

  assert(std::holds_alternative<std::unique_ptr<T>>(assoc.instance));
  return {MakeObserver(std::get<std::unique_ptr<T>>(assoc.instance).get()), false};
}


// ReSharper restore CppDefinitionsOrder

RenderInstanceRegistry::RenderInstanceRegistry(ObjectRegistry const& obj_registry) :
  object_registry_{&obj_registry} {}


auto RenderInstanceRegistry::CreateOrGetStaticInstance(
  ObjectId const& id) -> QueryResult<StaticRenderMeshInstance> {
  return CreateOrGetInstance<StaticRenderMeshInstance>(id);
}


auto RenderInstanceRegistry::CreateOrGetSkinnedInstance(
  ObjectId const& id) -> QueryResult<SkinnedRenderMeshInstance> {
  return CreateOrGetInstance<SkinnedRenderMeshInstance>(id);
}


auto RenderInstanceRegistry::CollectGarbage(std::size_t const budget) -> void {
  if (associations_.empty()) {
    return;
  }

  auto const check_count = std::min(budget, associations_.size());

  for (auto checked = 0uz; checked < check_count; ++checked) {
    auto const idx = gc_cursor_;

    if (++gc_cursor_ >= associations_.size()) {
      gc_cursor_ = 0;
    }

    auto& assoc = associations_[idx];

    if (std::holds_alternative<std::monostate>(assoc.instance)) {
      continue;
    }

    ObjectId const src_id{
      .idx = static_cast<std::uint32_t>(idx),
      .gen = assoc.generation
    };

    if (!object_registry_->Resolve(src_id)) {
      assoc.instance = std::monostate{};
    }
  }
}
}
