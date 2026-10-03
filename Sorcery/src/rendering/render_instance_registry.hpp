#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "skinned_render_mesh_instance.hpp"
#include "static_render_mesh_instance.hpp"
#include "../object_id.hpp"
#include "../observer_ptr.hpp"


namespace sorcery {
class ObjectRegistry;


namespace rendering {
class RenderInstanceRegistry {
public:
  template<typename T>
  struct QueryResult {
    ObserverPtr<T> resource;
    bool is_new;
  };


  explicit RenderInstanceRegistry(ObjectRegistry const& obj_registry);

  [[nodiscard]]
  auto CreateOrGetStaticInstance(ObjectId const& id) -> QueryResult<StaticRenderMeshInstance>;

  [[nodiscard]]
  auto CreateOrGetSkinnedInstance(ObjectId const& id) -> QueryResult<SkinnedRenderMeshInstance>;

  auto CollectGarbage(std::size_t budget) -> void;

private:
  using RenderInstance = std::variant<
    std::monostate,
    std::unique_ptr<StaticRenderMeshInstance>,
    std::unique_ptr<SkinnedRenderMeshInstance>
  >;


  struct Association {
    RenderInstance instance;
    std::uint32_t generation{};
  };


  template<typename T>
  [[nodiscard]]
  auto CreateInstance() const -> std::unique_ptr<T>;

  template<typename T>
  [[nodiscard]]
  auto CreateOrGetInstance(ObjectId const& id) -> QueryResult<T>;

  ObserverPtr<ObjectRegistry const> object_registry_;
  std::vector<Association> associations_;
  std::size_t gc_cursor_{};
};
}
}
