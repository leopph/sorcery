#include "Scene.hpp"

#include "../SceneObject.hpp"
#include "../Platform.hpp"
#undef FindResource

#include "../ResourceManager.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{ "Scene" };
}


namespace sorcery {
namespace {
auto ReflectionSerializeToYAML(rttr::variant const& variant, std::unordered_map<void const*, int> const& ptrFixUp) -> YAML::Node {
  auto const objType{ variant.get_type() };
  auto const isWrapper{ objType.is_wrapper() };
  auto const wrappedType{ objType.get_wrapped_type() };
  auto const rawWrappedType{
    isWrapper
      ? wrappedType.get_raw_type()
      : objType.get_raw_type()
  };

  YAML::Node ret;

  if (rawWrappedType.is_arithmetic()) {
    if (rawWrappedType == rttr::type::get<char>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<char>()
              : variant.get_value<char>();
    } else if (rawWrappedType == rttr::type::get<signed char>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<signed char>()
              : variant.get_value<signed char>();
    } else if (rawWrappedType == rttr::type::get<unsigned char>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned char>()
              : variant.get_value<unsigned char>();
    } else if (rawWrappedType == rttr::type::get<short>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<short>()
              : variant.get_value<short>();
    } else if (rawWrappedType == rttr::type::get<unsigned short>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned short>()
              : variant.get_value<unsigned short>();
    } else if (rawWrappedType == rttr::type::get<int>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<int>()
              : variant.get_value<int>();
    } else if (rawWrappedType == rttr::type::get<unsigned>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned>()
              : variant.get_value<unsigned>();
    } else if (rawWrappedType == rttr::type::get<long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<long>()
              : variant.get_value<long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned long>()
              : variant.get_value<unsigned long>();
    } else if (rawWrappedType == rttr::type::get<long long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<long long>()
              : variant.get_value<long long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long long>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<unsigned long long>()
              : variant.get_value<unsigned long long>();
    } else if (rawWrappedType == rttr::type::get<float>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<float>()
              : variant.get_value<float>();
    } else if (rawWrappedType == rttr::type::get<double>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<double>()
              : variant.get_value<double>();
    } else if (rawWrappedType == rttr::type::get<long double>()) {
      ret = isWrapper
              ? variant.get_wrapped_value<long double>()
              : variant.get_value<long double>();
    }
  } else if (rawWrappedType.is_sequential_container()) {
    for (auto const& elem : variant.create_sequential_view()) {
      ret.push_back(ReflectionSerializeToYAML(elem, ptrFixUp));
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
      ret[prop.get_name().to_string()] = ReflectionSerializeToYAML(prop.get_value(variant), ptrFixUp);
    }
  }

  return ret;
}


template<typename T>
auto ReflectionSerializeToYAML(T const& obj, std::unordered_map<void const*, int> const& ptrFixUp) -> YAML::Node {
  auto const objType{ rttr::type::get<T>() };

  YAML::Node ret;
  ret["type"] = objType.get_name().to_string();

  for (auto const& prop : objType.get_properties()) {
    ret["properties"][prop.get_name().to_string()] = ReflectionSerializeToYAML(prop.get_value(obj), ptrFixUp);
  }

  return ret;
}


auto ReflectioNDeserializeFromYAML(YAML::Node const& objNode, std::unordered_map<int, ObserverPtr<SceneObject>> const& ptrFixUp, rttr::variant& variant) -> void {
  auto const objType{ variant.get_type() };
  auto const isWrapper{ objType.is_wrapper() };
  auto const wrappedType{ objType.get_wrapped_type() };
  auto const rawWrappedType{
    isWrapper
      ? wrappedType.get_raw_type()
      : objType.get_raw_type()
  };

  if (rawWrappedType.is_arithmetic()) {
    if (rawWrappedType == rttr::type::get<char>()) {
      variant = objNode.as<char>();
    } else if (rawWrappedType == rttr::type::get<signed char>()) {
      variant = objNode.as<signed char>();
    } else if (rawWrappedType == rttr::type::get<unsigned char>()) {
      variant = objNode.as<unsigned char>();
    } else if (rawWrappedType == rttr::type::get<short>()) {
      variant = objNode.as<short>();
    } else if (rawWrappedType == rttr::type::get<unsigned short>()) {
      variant = objNode.as<unsigned short>();
    } else if (rawWrappedType == rttr::type::get<int>()) {
      variant = objNode.as<int>();
    } else if (rawWrappedType == rttr::type::get<unsigned>()) {
      variant = objNode.as<unsigned>();
    } else if (rawWrappedType == rttr::type::get<long>()) {
      variant = objNode.as<long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long>()) {
      variant = objNode.as<unsigned long>();
    } else if (rawWrappedType == rttr::type::get<long long>()) {
      variant = objNode.as<long long>();
    } else if (rawWrappedType == rttr::type::get<unsigned long long>()) {
      variant = objNode.as<unsigned long long>();
    } else if (rawWrappedType == rttr::type::get<float>()) {
      variant = objNode.as<float>();
    } else if (rawWrappedType == rttr::type::get<double>()) {
      variant = objNode.as<double>();
    } else if (rawWrappedType == rttr::type::get<long double>()) {
      variant = objNode.as<long double>();
    }
  } else if (rawWrappedType.is_sequential_container()) {
    auto sequence{ variant.create_sequential_view() };

    for (auto const& elemNode : objNode) {
      auto elem{ sequence.get_value_type().create() };
      ReflectioNDeserializeFromYAML(elemNode, ptrFixUp, elem);
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
      ReflectioNDeserializeFromYAML(objNode[prop.get_name().to_string()], ptrFixUp, propValue);
      std::ignore = prop.set_value(variant, propValue);
    }
  }
}


auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, std::unordered_map<int, ObserverPtr<SceneObject>> const& ptrFixUp, Object& obj) -> void {
  auto const objType{ rttr::type::get(obj) };

  auto const& propertiesNode{ objNode["properties"] };

  for (auto it{ propertiesNode.begin() }; it != propertiesNode.end(); ++it) {
    auto const prop{ objType.get_property(it->first.as<std::string>()) };
    auto propValue{ prop.get_value(obj) };
    ReflectioNDeserializeFromYAML(it->second, ptrFixUp, propValue);
    std::ignore = prop.set_value(obj, propValue);
  }
}
}


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

  for (auto const& sceneObject : mSceneObjects) {
    mYamlData["sceneObjects"].push_back(ReflectionSerializeToYAML(*sceneObject, ptrFixUp));
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

  for (auto const& [fileId, obj] : ptrFixUp) {
    ReflectionDeserializeFromYAML(mYamlData["objects"][fileId - 1], ptrFixUp, *obj);
  }

  ptrFixUp.clear();
}


auto Scene::Clear() -> void {
  mSceneObjects.clear();
}
}
