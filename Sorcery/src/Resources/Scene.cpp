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


auto Scene::Serialize() const noexcept -> YAML::Node {
  return mYamlData;
}


auto Scene::Deserialize(YAML::Node const& yamlNode) noexcept -> void {
  mYamlData = yamlNode;
}


auto Scene::CreateEntity() -> Entity& {
  auto* const entity = new Entity{};
  entity->SetScene(*this);

  mSceneObjects.emplace_back(entity);
  mEntities.emplace_back(entity);

  return *entity;
}


auto Scene::DestroyEntity(Entity const& entity) -> void {
  if (std::addressof(entity.GetScene()) != this) {
    return;
  }

  static std::vector<ObserverPtr<Component>> entityComponents;
  entityComponents.clear();

  for (auto const component : entity.GetComponents(entityComponents)) {
    std::erase(mComponents, component);
    std::erase_if(mSceneObjects, [component](auto const& sceneObject) {
      return sceneObject.get() == component;
    });
  }

  std::erase(mEntities, std::addressof(entity));
  std::erase_if(mSceneObjects, [&entity](auto const& sceneObject) {
    return sceneObject.get() == std::addressof(entity);
  });
}


auto Scene::DestroyComponent(Component const& component) -> void {
  if (std::addressof(component.GetEntity().GetScene()) != this) {
    return;
  }

  std::erase(mComponents, std::addressof(component));
  std::erase_if(mSceneObjects, [&component](auto const& sceneObject) {
    return sceneObject.get() == std::addressof(component);
  });
}


auto Scene::GetEntities() const noexcept -> std::span<ObserverPtr<Entity> const> {
  return mEntities;
}


auto Scene::Save() -> void {
  mYamlData.reset();
  mYamlData["version"] = 1;

  static std::unordered_map<void const*, int> ptrFixUp;

  for (auto const& sceneObject : mSceneObjects) {
    ptrFixUp[sceneObject.get()] = static_cast<int>(std::ssize(ptrFixUp) + 1);
  }

  std::function<YAML::Node(rttr::variant const&)> extensionFunc;
  extensionFunc = [&extensionFunc](rttr::variant const& variant) {
    auto const objType{ variant.get_type() };
    auto const isWrapper{ objType.is_wrapper() };
    auto const wrappedType{ objType.get_wrapped_type() };
    auto const rawWrappedType{
      isWrapper
        ? wrappedType.get_raw_type()
        : objType.get_raw_type()
    };

    YAML::Node ret;

    if (rawWrappedType.is_sequential_container()) {
      for (auto const& elem : variant.create_sequential_view()) {
        ret.push_back(ReflectionSerializeToYAML(elem, extensionFunc));
      }
    } else if (objType.is_pointer() && rawWrappedType.is_derived_from(rttr::type::get<SceneObject>())) {
      auto const ptr{
        isWrapper
          ? variant.get_wrapped_value<void const*>()
          : variant.get_value<void const*>()
      };

      if (auto const it{ ptrFixUp.find(ptr) }; it != std::end(ptrFixUp)) {
        ret = it->second;
      } else {
        ret = 0;
      }
    } else if (isWrapper && wrappedType.is_pointer() && rawWrappedType.is_derived_from(rttr::type::get<Resource>())) {
      if (auto const res{ variant.extract_wrapped_value().get_value<Resource*>() }) {
        ret = res->GetGuid().ToString();
      } else {
        ret = Guid{}.ToString();
      }
    } else if (rawWrappedType.is_class() && !isWrapper) {
      for (auto const& prop : objType.get_properties()) {
        ret[prop.get_name().to_string()] = ReflectionSerializeToYAML(prop.get_value(variant), extensionFunc);
      }
    }

    return ret;
  };

  for (auto const& sceneObject : mSceneObjects) {
    mYamlData["sceneObjects"].push_back(ReflectionSerializeToYAML(*sceneObject, extensionFunc));
  }

  ptrFixUp.clear();
}


auto Scene::Load(ObjectInstantiatorManager const& manager) -> void {
  sActiveScene = this;
  mSceneObjects.clear();
  mEntities.clear();
  mComponents.clear();

  if (auto const version{ mYamlData["version"] }; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{ std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName()) };
  }

  static std::unordered_map<int, SceneObject*> ptrFixUp;

  for (auto const& sceneObjectNode : mYamlData["sceneObjects"]) {
    auto const typeNode{ sceneObjectNode["type"] };
    auto const type{ rttr::type::get(typeNode.as<std::string>()) };
    auto const sceneObjectVariant{ type.create() };
    mSceneObjects.emplace_back(sceneObjectVariant.get_value<SceneObject*>());

    if (type == rttr::type::get<Entity>()) {
      mEntities.push_back(sceneObjectVariant.get_value<Entity*>());
    } else if (type.is_derived_from(rttr::type::get<Component>())) {
      mComponents.push_back(sceneObjectVariant.get_value<Component*>());
    } else {
      throw std::runtime_error{ "Instantiated SceneObject is neither an Entity nor a Component!" };
    }

    ptrFixUp[static_cast<int>(std::ssize(ptrFixUp)) + 1] = sceneObjectVariant.get_value<SceneObject*>();
  }

  std::function<void(YAML::Node const&, rttr::variant& variant)> extensionFunc;
  extensionFunc = [&extensionFunc](YAML::Node const& objNode, rttr::variant& variant) {
    auto const objType{ variant.get_type() };
    auto const isWrapper{ objType.is_wrapper() };
    auto const wrappedType{ objType.get_wrapped_type() };
    auto const rawWrappedType{
      isWrapper
        ? wrappedType.get_raw_type()
        : objType.get_raw_type()
    };

    if (rawWrappedType.is_sequential_container()) {
      auto sequence{ variant.create_sequential_view() };

      for (auto const& elemNode : objNode) {
        auto elem{ sequence.get_value_type().create() };
        ReflectionDeserializeFromYAML(elemNode, elem, extensionFunc);
        sequence.insert(sequence.end(), elem);
      }
    } else if (objType.is_pointer() && rawWrappedType.is_derived_from(rttr::type::get<SceneObject>())) {
      if (auto const ptrId{ objNode.as<int>() }; ptrId != 0) {
        if (auto const it{ ptrFixUp.find(ptrId) }; it != std::end(ptrFixUp)) {
          variant = it->second;
        } else {
          variant = nullptr;
        }
      } else {
        variant = nullptr;
      }
    } else if (isWrapper && wrappedType.is_pointer() && rawWrappedType.is_derived_from(rttr::type::get<Resource>())) {
      variant = gResourceManager.FindResource(Guid::Parse(objNode.as<std::string>()));
    } else if (rawWrappedType.is_class() && !isWrapper) {
      for (auto const& prop : objType.get_properties()) {
        auto propValue{ prop.get_value(variant) };
        ReflectionDeserializeFromYAML(objNode[prop.get_name().to_string()], propValue, extensionFunc);
        std::ignore = prop.set_value(variant, propValue);
      }
    }
  };

  for (auto const& [fileId, obj] : ptrFixUp) {
    ReflectionDeserializeFromYAML(mYamlData["objects"][fileId - 1], *obj, extensionFunc);
  }

  ptrFixUp.clear();
}


auto Scene::Clear() -> void {
  mSceneObjects.clear();
}
}
