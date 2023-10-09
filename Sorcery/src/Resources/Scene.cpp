#include "Scene.hpp"

#include "../SceneObjects/SceneObject.hpp"
#include "../Platform.hpp"
#include "../Serialization.hpp"
#undef FindResource
#include "../ResourceManager.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{"Scene"};
}


namespace sorcery {
Scene* Scene::sActiveScene{nullptr};
std::vector<Scene*> Scene::sAllScenes;


auto Scene::GetActiveScene() noexcept -> Scene* {
  if (!sActiveScene) {
    sActiveScene = CreateAndInitialize<Scene>();
  }

  return sActiveScene;
}


Scene::Scene() {
  sAllScenes.emplace_back(this);

  if (!sActiveScene) {
    sActiveScene = this;
  }

  Save();
}


Scene::~Scene() {
  std::erase(sAllScenes, this);

  // Strange loop needed because Entity dtor calls Scene::RemoveEntity
  while (!mEntities.empty()) {
    Destroy(*mEntities.back());
  }

  if (sActiveScene == this) {
    sActiveScene = sAllScenes.empty() ? nullptr : sAllScenes.back();
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
  mYamlData["ambientLight"] = mAmbientLight;

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
      YAML::Node retNode;

      if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        auto const it{ptrFixUp.find(v.get_value<SceneObject*>())};
        retNode = it != std::end(ptrFixUp) ? it->second : 0;
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

  auto const lastActiveScene{sActiveScene};
  sActiveScene = this;

  Clear();

  if (auto const version{mYamlData["version"]}; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName())};
  }

  if (auto const node{mYamlData["ambientLight"]}) {
    mAmbientLight = node.as<Vector3>(mAmbientLight);
  }

  for (auto const& sceneObjectNode : mYamlData["sceneObjects"]) {
    auto const typeNode{sceneObjectNode["type"]};
    auto const type{rttr::type::get_by_name(typeNode.as<std::string>())};
    auto const sceneObjectVariant{type.create()};
    auto const sceneObj{sceneObjectVariant.get_value<SceneObject*>()};
    sceneObj->OnInit();
    ptrFixUp[static_cast<int>(std::ssize(ptrFixUp)) + 1] = sceneObj;
  }

  auto const extensionFunc{
    [](YAML::Node const& objNode, rttr::variant& v) -> void {
      if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        if (auto const it{ptrFixUp.find(objNode.as<int>(0))}; it != std::end(ptrFixUp)) {
          auto const type{v.get_type()};
          v = it->second;
          [[maybe_unused]] auto const success{v.convert(type)};
          assert(success);
        }
      }
    }
  };

  for (auto const& [fileId, obj] : ptrFixUp) {
    ReflectionDeserializeFromYaml(mYamlData["sceneObjects"][fileId - 1]["properties"], *obj, extensionFunc);
  }

  sActiveScene = lastActiveScene;
}


auto Scene::SetActive() -> void {
  sActiveScene = this;
}


auto Scene::Clear() -> void {
  // Entity destructor modifies this collection, hence the strange loop
  while (!mEntities.empty()) {
    Destroy(*mEntities.back());
  }
}


auto Scene::GetAmbientLightVector() const noexcept -> Vector3 const& {
  return mAmbientLight;
}


auto Scene::SetAmbientLightVector(Vector3 const& vector) noexcept -> void {
  mAmbientLight = vector;
}


auto Scene::GetAmbientLight() const noexcept -> Color {
  return Color{Vector4{mAmbientLight, 1}};
}


auto Scene::SetAmbientLight(Color const& color) noexcept -> void {
  mAmbientLight = Vector3{static_cast<Vector4>(color)};
}
}
