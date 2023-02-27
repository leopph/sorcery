#include "Scene.hpp"

namespace leopph {
Object::Type const Scene::SerializationType{ Type::Scene };

auto Scene::GetSerializationType() const -> Type {
	return SerializationType;
}

auto Scene::CreateEntity() -> Entity& {
	return *mEntities.emplace_back(new Entity{});
}

auto Scene::DestroyEntity(Entity const& entityToRemove) -> void {
	std::erase_if(mEntities, [&entityToRemove](auto const& entity) {
		return entity.get() == &entityToRemove;
	});
}

auto Scene::GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const> {
	return mEntities;
}

auto Scene::Serialize() const noexcept -> std::string {
	return Dump(mYamlData);
}

auto Scene::Deserialize(std::span<u8 const> const bytes) -> void {
	mYamlData = YAML::Load(reinterpret_cast<char const*>(bytes.data()));
}

auto Scene::Save() -> void {
	auto constexpr serializeObject{
		[](Object const* obj) {
			YAML::Node objectNode;

			objectNode["objectType"] = static_cast<int>(obj->GetSerializationType());
			objectNode["guid"] = obj->GetGuid().ToString();

			YAML::Node dataNode;
			obj->SerializeTextual(dataNode);
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

auto Scene::Load(ObjectFactory const& factory) -> void {
	mEntities.clear();

	struct ObjectWithSerializedData {
		ManagedAccessObject* obj;
		YAML::Node node;
	};

	std::vector<ObjectWithSerializedData> objectsWithSerializedData;

	for (std::size_t i{ 0 }; i < mYamlData.size(); i++) {
		auto const guid{ Guid::Parse(mYamlData[i]["guid"].as<std::string>()) };

		if (auto const obj{ dynamic_cast<ManagedAccessObject*>(factory.New(static_cast<Object::Type>(mYamlData[i]["objectType"].as<int>()))) }) {
			obj->SetGuid(guid);
			objectsWithSerializedData.emplace_back(obj, mYamlData[i]["data"]);
		}
	}

	for (auto& [obj, node] : objectsWithSerializedData) {
		obj->DeserializeTextual(node);
		obj->CreateManagedObject();
	}
}
}
