#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "render_material.hpp"
#include "render_mesh.hpp"
#include "render_texture.hpp"
#include "../object_id.hpp"
#include "../observer_ptr.hpp"
#include "wand/wand.hpp"


namespace sorcery {
class ObjectRegistry;


namespace rendering {
class RenderResourceRegistry {
public:
  template<typename T>
  struct QueryResult {
    ObserverPtr<T> resource;
    bool is_new;
  };


  RenderResourceRegistry(wand::GraphicsDevice& device, ObjectRegistry const& obj_registry);

  [[nodiscard]]
  auto CreateOrGetMaterial(ObjectId const& id) -> QueryResult<RenderMaterial>;

  [[nodiscard]]
  auto CreateOrGetMesh(ObjectId const& id) -> QueryResult<RenderMesh>;

  [[nodiscard]]
  auto CreateOrGetTexture(ObjectId const& id) -> QueryResult<RenderTexture>;

  auto CollectGarbage(std::size_t budget) -> void;

private:
  using RenderResource = std::variant<
    std::monostate,
    std::unique_ptr<RenderMaterial>,
    std::unique_ptr<RenderMesh>,
    std::unique_ptr<RenderTexture>
  >;


  struct Association {
    RenderResource resource;
    std::uint32_t generation{};
  };


  template<typename T>
  [[nodiscard]]
  auto CreateResource() const -> std::unique_ptr<T>;


  template<typename T>
  [[nodiscard]]
  auto CreateOrGetResource(ObjectId const& id) -> QueryResult<T>;


  ObserverPtr<wand::GraphicsDevice> device_;
  ObserverPtr<ObjectRegistry const> obj_registry_;
  std::vector<Association> associations_;
  std::size_t gc_cursor_{};
};
}
}
