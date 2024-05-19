#pragma once

#include "SceneObject.hpp"
#include "Component.hpp"
#include "TransformComponent.hpp"
#include "../Reflection.hpp"

#include <vector>
#include <concepts>


namespace sorcery {
class Scene;


class Entity final : public SceneObject {
  RTTR_ENABLE(SceneObject)
  RTTR_REGISTRATION_FRIEND

public:
  LEOPPHAPI Entity();
  LEOPPHAPI Entity(Entity const& other);
  LEOPPHAPI Entity(Entity&& other) noexcept;

  ~Entity() override = default;

  auto operator=(Entity const& other) -> void = delete;
  auto operator=(Entity&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto Clone() -> Entity* override;

  [[nodiscard]] LEOPPHAPI auto GetTransform() const -> TransformComponent&;

  [[nodiscard]] LEOPPHAPI auto GetScene() const -> Scene&;

  LEOPPHAPI auto AddComponent(Component& component) -> void;
  LEOPPHAPI auto RemoveComponent(Component& component) -> void;

  template<std::derived_from<Component> T>
  auto GetComponent() const -> T*;

  template<std::derived_from<Component> T>
  auto GetComponents(std::vector<T*>& out) const -> std::vector<T*>&;

  template<std::derived_from<Component> T>
  auto GetComponents() const -> std::vector<T*>;

  LEOPPHAPI auto OnInit() -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  [[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;

private:
  Scene* mScene{nullptr};
  mutable TransformComponent* mTransform{nullptr};
  std::vector<Component*> mComponents;
};


template<std::derived_from<Component> T>
auto Entity::GetComponent() const -> T* {
  if constexpr (std::same_as<Component, T>) {
    return mComponents.empty()
             ? nullptr
             : mComponents.front();
  } else {
    for (auto const component : mComponents) {
      if (auto const castPtr{rttr::rttr_cast<T*>(component)}) {
        return castPtr;
      }
    }

    return nullptr;
  }
}


template<std::derived_from<Component> T>
auto Entity::GetComponents(std::vector<T*>& out) const -> std::vector<T*>& {
  if constexpr (std::is_same_v<T, Component>) {
    out = mComponents;
  } else {
    out.clear();

    for (auto const component : mComponents) {
      if (auto const castPtr{rttr::rttr_cast<T*>(component)}) {
        out.emplace_back(castPtr);
      }
    }
  }

  return out;
}


template<std::derived_from<Component> T>
auto Entity::GetComponents() const -> std::vector<T*> {
  std::vector<T*> ret;
  GetComponents(ret);
  return ret;
}
}
