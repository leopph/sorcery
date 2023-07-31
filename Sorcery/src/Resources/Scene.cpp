#include "Scene.hpp"

#include "../SceneObject.hpp"
#include "../Platform.hpp"
#include "../Serialization.hpp"
#undef FindResource
#include "../ResourceManager.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{ "Scene" };
}


namespace sorcery {
Scene* Scene::sActiveScene{ nullptr };
std::vector<Scene*> Scene::sAllScenes;


auto Scene::GetActiveScene() noexcept -> Scene* {
  if (!sActiveScene) {
    sActiveScene = new Scene{};
  }

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

  // Strange loop needed because Entity dtor calls Scene::RemoveEntity
  while (!mEntities.empty()) {
    delete mEntities.back();
  }

  if (sActiveScene == this) {
    sActiveScene = sAllScenes.empty()
                     ? nullptr
                     : sAllScenes.back();
  }
}


auto Scene::Serialize() const noexcept -> YAML::Node {
  return mYamlData;
}


auto Scene::Deserialize(YAML::Node const& yamlNode) noexcept -> void {
  mYamlData = yamlNode;
}


auto Scene::AddEntity(Entity& entity) -> void {
  mEntities.push_back(std::addressof(entity));
}


auto Scene::RemoveEntity(Entity const& entity) -> void {
  std::erase(mEntities, std::addressof(entity));
}


auto Scene::GetEntities() const noexcept -> std::vector<ObserverPtr<Entity>> const& {
  return mEntities;
}


auto Scene::Save() -> void {
  static std::vector<ObserverPtr<SceneObject>> tmpThisSceneObjects;
  tmpThisSceneObjects.clear();

  static std::vector<ObserverPtr<Component>> tmpComponents;
  tmpComponents.clear();

  static std::unordered_map<void const*, int> ptrFixUp;
  ptrFixUp.clear();

  mYamlData.reset();
  mYamlData["version"] = 1;

  for (auto const entity : mEntities) {
    tmpThisSceneObjects.emplace_back(entity);

    for (auto const component : entity->GetComponents(tmpComponents)) {
      tmpThisSceneObjects.emplace_back(component);
    }
  }

  for (auto const sceneObj : tmpThisSceneObjects) {
    ptrFixUp[sceneObj] = static_cast<int>(std::ssize(ptrFixUp) + 1);
  }

  auto const extensionFunc{
    [](rttr::variant const& v) -> YAML::Node {
      auto const type{ v.get_type() };
      YAML::Node retNode;

      if (type.is_pointer() && type.get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        auto const it{ ptrFixUp.find(v.get_value<SceneObject*>()) };
        retNode = it != std::end(ptrFixUp)
                    ? it->second
                    : 0;
      } else if (type.is_wrapper() && type.get_wrapped_type().is_pointer() && type.get_wrapped_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        auto const it{ ptrFixUp.find(v.get_wrapped_value<SceneObject*>()) };
        retNode = it != std::end(ptrFixUp)
                    ? it->second
                    : 0;
      } else if (type.is_sequential_container()) {
        if (auto const seqView{ v.create_sequential_view() }; seqView.get_value_type().is_pointer() && seqView.get_value_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
          for (auto const elem : seqView) {
            auto const it{ ptrFixUp.find(elem.get_value<SceneObject*>()) };
            retNode.push_back(it != std::end(ptrFixUp)
                                ? it->second
                                : 0);
          }
        }
      }

      return retNode;
    }
  };

  for (auto const sceneObj : tmpThisSceneObjects) {
    YAML::Node sceneObjNode;
    sceneObjNode["type"] = rttr::type::get(*sceneObj).get_name().to_string();
    sceneObjNode["properties"] = ReflectionSerializeToYaml(*sceneObj, extensionFunc);
    mYamlData["sceneObjects"].push_back(sceneObjNode);
  }
}


auto Scene::Load() -> void {
  static std::unordered_map<int, SceneObject*> ptrFixUp;
  ptrFixUp.clear();

  sActiveScene = this;
  mEntities.clear();

  if (auto const version{ mYamlData["version"] }; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{ std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName()) };
  }

  for (auto const& sceneObjectNode : mYamlData["sceneObjects"]) {
    auto const typeNode{ sceneObjectNode["type"] };
    auto const type{ rttr::type::get(typeNode.as<std::string>()) };
    auto const sceneObjectVariant{ type.create() };
    ptrFixUp[static_cast<int>(std::ssize(ptrFixUp)) + 1] = sceneObjectVariant.get_value<SceneObject*>();
  }

  auto const extensionFunc{
    [](YAML::Node const& objNode, rttr::variant& v) -> void {
      if (auto const type{ v.get_type() }; (type.is_pointer() && type.get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) || (type.is_wrapper() && type.get_wrapped_type().is_pointer() && type.get_wrapped_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>()))) {
        auto const it{ ptrFixUp.find(objNode.as<int>(0)) };
        v = it != std::end(ptrFixUp)
              ? it->second
              : nullptr;
      } else if (type.is_sequential_container()) {
        if (objNode.IsSequence()) {
          auto seqView{ v.create_sequential_view() };
          seqView.set_size(objNode.size());

          for (auto i{ 0 }; i < std::ssize(objNode); i++) {
            auto const it{ ptrFixUp.find(objNode.as<int>(0)) };
            seqView.set_value(i, it != std::end(ptrFixUp)
                                   ? it->second
                                   : nullptr);
          }
        }
      }
    }
  };

  for (auto const& [fileId, obj] : ptrFixUp) {
    ReflectionDeserializeFromYaml(mYamlData["sceneObjects"][fileId - 1]["properties"], *obj, extensionFunc);

    if (auto const entity{ rttr::rttr_cast<ObserverPtr<Entity>>(obj) }) {
      mEntities.emplace_back(entity);
    }
  }
}


auto Scene::SetActive() -> void {
  sActiveScene = this;
}


auto Scene::Clear() -> void {
  mEntities.clear();
}
}
