#include "Scene.hpp"

#include "SceneElement.hpp"
#include "Platform.hpp"

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
  } else if (objType.is_pointer() && rawWrappedType.is_derived_from(rttr::type::get<SceneElement>())) {
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
  } else if (rawWrappedType.is_class()) {
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


auto ReflectioNDeserializeFromYAML(YAML::Node const& objNode, std::unordered_map<int, std::shared_ptr<SceneElement>> const& ptrFixUp, rttr::variant& variant) -> void {
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
      rttr::variant elem;
      ReflectioNDeserializeFromYAML(elemNode, ptrFixUp, elem);
      sequence.insert(sequence.end(), elem);
    }
  } else if (objType.is_pointer() && rawWrappedType.is_derived_from(rttr::type::get<SceneElement>())) {
    if (auto const ptrId{ objNode.as<int>() }; ptrId != 0) {
      if (auto const it{ ptrFixUp.find(ptrId) }; it != std::end(ptrFixUp)) {
        variant = it->second.get();
      } else {
        variant = nullptr;
      }
    } else {
      variant = nullptr;
    }
  } else if (rawWrappedType.is_class()) {
    for (auto const& prop : objType.get_properties()) {
      rttr::variant propValue;
      ReflectioNDeserializeFromYAML(objNode[prop.get_name().to_string()], ptrFixUp, propValue);
      std::ignore = prop.set_value(variant, propValue);
    }
  }
}


auto ReflectionDeserializeFromYAML(YAML::Node const& objNode, std::unordered_map<int, std::shared_ptr<SceneElement>> const& ptrFixUp, Object& obj) -> void {
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
  auto const entity = new Entity{};
  entity->SetScene(this);
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

  static std::unordered_map<void const*, int> ptrFixUp;

  for (auto const& entity : mEntities) {
    ptrFixUp[entity.get()] = static_cast<int>(std::ssize(ptrFixUp) + 1);

    for (auto const& component : entity->mComponents) {
      ptrFixUp[component.get()] = static_cast<int>(std::ssize(ptrFixUp) + 1);
    }
  }

  for (auto const& entity : mEntities) {
    mYamlData["objects"].push_back(ReflectionSerializeToYAML(*entity, ptrFixUp));

    for (auto const& component : entity->mComponents) {
      mYamlData["objects"].push_back(ReflectionSerializeToYAML(*component, ptrFixUp));
    }
  }

  ptrFixUp.clear();
}


auto Scene::Load(ObjectInstantiatorManager const& manager) -> void {
  sActiveScene = this;
  mEntities.clear();

  if (auto const version{ mYamlData["version"] }; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{ std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName()) };
  }

  static std::unordered_map<int, std::shared_ptr<SceneElement>> ptrFixUp;

  for (auto const objectNode : mYamlData["objects"]) {
    auto const typeNode{ objectNode["type"] };
    auto const type{ rttr::type::get(typeNode.as<std::string>()) };
    ptrFixUp[static_cast<int>(std::ssize(ptrFixUp)) + 1] = type.create().get_value<std::shared_ptr<SceneElement>>();
  }

  for (auto const& [fileId, obj] : ptrFixUp) {
    ReflectionDeserializeFromYAML(mYamlData["objects"][fileId - 1], ptrFixUp, *obj);
  }

  ptrFixUp.clear();
}


auto Scene::Clear() -> void {
  mEntities.clear();
}
}
