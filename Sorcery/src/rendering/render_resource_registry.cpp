#include "render_resource_registry.hpp"

#include <algorithm>
#include <cassert>

#include "../object_registry.hpp"


namespace sorcery::rendering {
// ReSharper disable CppDefinitionsOrder
template<>
auto RenderResourceRegistry::CreateResource<RenderMaterial>() const -> std::unique_ptr<RenderMaterial> {
  return std::make_unique<RenderMaterial>(*device_);
}


template<>
auto RenderResourceRegistry::CreateResource<RenderMesh>() const -> std::unique_ptr<RenderMesh> {
  return std::make_unique<RenderMesh>();
}


template<>
auto RenderResourceRegistry::CreateResource<RenderTexture>() const -> std::unique_ptr<RenderTexture> {
  return std::make_unique<RenderTexture>();
}


template<typename T>
auto RenderResourceRegistry::CreateOrGetResource(ObjectId const& id) -> QueryResult<T> {
  assert(id.IsValid());

  if (associations_.size() <= id.idx) {
    associations_.resize(id.idx + 1);
  }

  auto& assoc = associations_[id.idx];

  if (std::holds_alternative<std::monostate>(assoc.resource) || assoc.generation != id.gen) {
    auto res = CreateResource<T>();
    auto const obs = MakeObserver(res.get());
    assoc.generation = id.gen;
    assoc.resource = std::move(res);
    return {obs, true};
  }

  assert(std::holds_alternative<std::unique_ptr<T>>(assoc.resource));
  return {MakeObserver(std::get<std::unique_ptr<T>>(assoc.resource).get()), false};
}


// ReSharper restore CppDefinitionsOrder


RenderResourceRegistry::RenderResourceRegistry(wand::GraphicsDevice& device, ObjectRegistry const& obj_registry) :
  device_{&device},
  obj_registry_{&obj_registry} {}


auto RenderResourceRegistry::CreateOrGetMaterial(ObjectId const& id) -> QueryResult<RenderMaterial> {
  return CreateOrGetResource<RenderMaterial>(id);
}


auto RenderResourceRegistry::CreateOrGetMesh(ObjectId const& id) -> QueryResult<RenderMesh> {
  return CreateOrGetResource<RenderMesh>(id);
}


auto RenderResourceRegistry::CreateOrGetTexture(ObjectId const& id) -> QueryResult<RenderTexture> {
  return CreateOrGetResource<RenderTexture>(id);
}


auto RenderResourceRegistry::CollectGarbage(std::size_t const budget) -> void {
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

    if (std::holds_alternative<std::monostate>(assoc.resource)) {
      continue;
    }

    ObjectId const src_id{
      .idx = static_cast<std::uint32_t>(idx),
      .gen = assoc.generation
    };

    if (!obj_registry_->Resolve(src_id)) {
      assoc.resource = std::monostate{};
    }
  }
}
}
