#include "Scene.hpp"

#include "SceneElement.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{ "Scene" };
}


namespace sorcery {
Object::Type const Scene::SerializationType{ Type::Scene };


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
  auto constexpr serializeObject{
    [](SceneElement const* obj) {
      YAML::Node objectNode;

      objectNode["objectType"] = static_cast<int>(obj->GetSerializationType());
      objectNode["guid"] = obj->GetGuid().ToString();

      YAML::Node dataNode;
      obj->Serialize(dataNode);
      objectNode["data"] = dataNode;

      return objectNode;
    }
  };

  mYamlData.reset();

  for (auto const& entity : mEntities) {
    mYamlData.push_back(serializeObject(entity.get()));

    for (auto const& component : entity->mComponents) {
      mYamlData.push_back(serializeObject(component.get()));
    }
  }
}


auto Scene::Load(ObjectInstantiatorManager const& manager) -> void {
  mEntities.clear();

  struct ObjectWithSerializedData {
    SceneElement* obj;
    YAML::Node node;
  };

  std::vector<ObjectWithSerializedData> objectsWithSerializedData;

  for (std::size_t i{ 0 }; i < mYamlData.size(); i++) {
    auto const guid{ Guid::Parse(mYamlData[i]["guid"].as<std::string>()) };

    if (auto const obj{ dynamic_cast<SceneElement*>(manager.GetFor(static_cast<Type>(mYamlData[i]["objectType"].as<int>())).Instantiate()) }) {
      obj->SetGuid(guid);
      objectsWithSerializedData.emplace_back(obj, mYamlData[i]["data"]);
    }
  }

  for (auto& [obj, node] : objectsWithSerializedData) {
    obj->Deserialize(node);

    if (auto const entity{ dynamic_cast<Entity*>(obj) }) {
      entity->SetScene(this);
      mEntities.emplace_back(entity);
    }
  }
}


auto Scene::Clear() -> void {
  mEntities.clear();
}
}
