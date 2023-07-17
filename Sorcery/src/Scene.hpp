#pragma once

#include "NativeResource.hpp"
#include "Entity.hpp"
#include "Component.hpp"

#include "YamlInclude.hpp"

#include <memory>
#include <vector>
#include <span>
#include <concepts>


namespace sorcery {
class Scene : public NativeResource {
  RTTR_ENABLE(NativeResource)
  static Scene* sActiveScene;
  static std::vector<Scene*> sAllScenes;


  std::vector<std::unique_ptr<SceneObject>> mSceneObjects;
  std::vector<ObserverPtr<Entity>> mEntities;
  std::vector<ObserverPtr<Component>> mComponents;

  YAML::Node mYamlData;

public:
  [[nodiscard]] LEOPPHAPI static auto GetActiveScene() noexcept -> Scene*;

  LEOPPHAPI Scene();
  LEOPPHAPI ~Scene() override;

  LEOPPHAPI static Type const SerializationType;
  LEOPPHAPI auto GetSerializationType() const -> Type override;

  LEOPPHAPI auto CreateEntity() -> Entity&;
  LEOPPHAPI auto DestroyEntity(Entity const& entity) -> void;

  template<std::derived_from<Component> T>
  LEOPPHAPI auto CreateComponent(Entity& targetEntity) -> ObserverPtr<T>;
  LEOPPHAPI auto DestroyComponent(Component const& component) -> void;

  [[nodiscard]] LEOPPHAPI auto GetEntities() const noexcept -> std::span<ObserverPtr<Entity> const>;

  LEOPPHAPI auto Serialize(std::vector<std::uint8_t>& out) const noexcept -> void override;
  LEOPPHAPI auto Deserialize(std::span<std::uint8_t const> bytes) -> void;

  LEOPPHAPI auto Save() -> void;
  LEOPPHAPI auto Load(ObjectInstantiatorManager const& manager) -> void;

  LEOPPHAPI auto Clear() -> void;
};


template<std::derived_from<Component> T>
auto Scene::CreateComponent(Entity& targetEntity) -> ObserverPtr<T> {
  for (auto& sceneObject : mSceneObjects) {
    if (sceneObject.get() == std::addressof(targetEntity)) {
      auto const component{ mSceneObjects.emplace_back(new T{}).get() };
      mComponents.emplace_back(component);
      component->SetEntity(targetEntity);
      targetEntity.AddComponent(*component);
      return component;
    }
  }

  return nullptr;
}
}
