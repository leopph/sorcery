#pragma once

#include "NativeResource.hpp"
#include "../Entity.hpp"
#include "../Component.hpp"

#include <memory>
#include <vector>
#include <span>
#include <concepts>


namespace sorcery {
class Scene final : public NativeResource {
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
  Scene(Scene const& other) = delete;
  Scene(Scene&& other) = delete;

  LEOPPHAPI ~Scene() override;

  auto operator=(Scene const& other) -> void = delete;
  auto operator=(Scene&& other) -> void = delete;

  LEOPPHAPI auto CreateEntity() -> Entity&;
  LEOPPHAPI auto DestroyEntity(Entity const& entity) -> void;

  template<std::derived_from<Component> T>
  LEOPPHAPI auto CreateComponent(Entity& targetEntity) -> ObserverPtr<T>;
  LEOPPHAPI auto DestroyComponent(Component const& component) -> void;

  [[nodiscard]] LEOPPHAPI auto GetEntities() const noexcept -> std::span<ObserverPtr<Entity> const>;

  LEOPPHAPI auto Save() -> void;
  LEOPPHAPI auto Load(ObjectInstantiatorManager const& manager) -> void;

  LEOPPHAPI auto Clear() -> void;

  LEOPPHAPI static Type const SerializationType;
  LEOPPHAPI auto GetSerializationType() const -> Type override;

  [[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> YAML::Node override;
  LEOPPHAPI auto Deserialize(YAML::Node const& yamlNode) noexcept -> void override;
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
