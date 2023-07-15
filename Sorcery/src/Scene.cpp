#include "Scene.hpp"

#include "SceneElement.hpp"
#include "Platform.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{ "Scene" };
}


namespace sorcery {
Scene* Scene::sActiveScene{ nullptr };
std::vector<Scene*> Scene::sAllScenes;
Object::Type const Scene::SerializationType{ Type::Scene };


auto Scene::GetActiveScene() noexcept -> Scene* {
  return sActiveScene;
}


Scene::Scene() {
  sAllScenes.emplace_back(this);

  if (!sActiveScene) {
    sActiveScene = this;
  }
}


Scene::~Scene() {
  std::erase(sAllScenes, this);

  if (sActiveScene == this) {
    sActiveScene = sAllScenes.empty()
                     ? nullptr
                     : sAllScenes.back();
  }
}


auto Scene::GetSerializationType() const -> Type {
  return SerializationType;
}


auto Scene::CreateEntity() -> Entity& {
  auto const entity = new Entity{};
  entity->SetScene(this);
  entity->SetGuid(Guid::Generate());
  return *mEntities.emplace_back(entity);
}


auto Scene::DestroyEntity(Entity const& entityToRemove) -> void {
  std::erase_if(mEntities, [&entityToRemove](auto const& entity) {
    return entity.get() == &entityToRemove;
  });
}


auto Scene::GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const> {
  return mEntities;
}


auto Scene::Serialize(std::vector<std::uint8_t>& out) const noexcept -> void {
  auto const dump{ Dump(mYamlData) };
  out.assign(std::begin(dump), std::end(dump));
}


auto Scene::Deserialize(std::span<std::uint8_t const> const bytes) -> void {
  std::string const input{ std::begin(bytes), std::end(bytes) };
  mYamlData = YAML::Load(input);
}


auto Scene::Save() -> void {
  mYamlData.reset();
  mYamlData["version"] = 1;

  for (auto const& entity : mEntities) {
    YAML::Node entityDataNode;
    entity->Serialize(entityDataNode);

    YAML::Node entityNode;
    entityNode["data"] = entityDataNode;

    for (auto const& component : entity->mComponents) {
      YAML::Node componentDataNode;
      component->Serialize(componentDataNode);

      YAML::Node componentNode;
      componentNode["type"] = static_cast<int>(component->GetSerializationType());
      componentNode["data"] = componentDataNode;

      entityNode["components"].push_back(componentNode);
    }

    mYamlData["entities"].push_back(entityNode);
  }
}


auto Scene::Load(ObjectInstantiatorManager const& manager) -> void {
  sActiveScene = this;
  mEntities.clear();

  if (auto const version{ mYamlData["version"] }; version && version.IsScalar() && version.as<int>(1) == 1) {
    for (auto const entityNode : mYamlData["entities"]) {
      auto const& entity{ mEntities.emplace_back(new Entity{}) };
      entity->Deserialize(entityNode["data"]);

      for (auto const componentNode : entityNode["components"]) {
        if (auto const component{ dynamic_cast<Component*>(manager.GetFor(static_cast<Type>(componentNode["type"].as<int>())).Instantiate()) }) {
          entity->AddComponent(std::shared_ptr<Component>{ component });
          component->Deserialize(componentNode["data"]);
        }
      }
    }
  } else {
    throw std::runtime_error{ std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName()) };
  }
}


auto Scene::Clear() -> void {
  mEntities.clear();
}
}
