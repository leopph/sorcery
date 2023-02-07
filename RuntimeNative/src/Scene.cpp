#include "Scene.hpp"


namespace leopph {
Scene::Scene(std::string mName) :
	mName{ std::move(mName) } {}


auto Scene::GetSerializationType() const -> Type {
	return SerializationType;
}

void Scene::SerializeBinary(std::vector<u8>& out) const {
	auto const nodeStr{ Dump(mYamlData) };
	std::span const nodeStrSpan{ reinterpret_cast<u8 const*>(nodeStr.data()), nodeStr.length() + 1 };
	out.insert_range(std::end(out), nodeStrSpan);
}

Object::BinaryDeserializationResult Scene::DeserializeBinary(std::span<u8 const> const bytes) {
	mYamlData = YAML::Load(reinterpret_cast<char const*>(bytes.data()));
	return { 0 };
}

auto Scene::Load(ObjectFactory const& objectFactory) -> void {
	struct ObjectWithSerializedData {
		ManagedAccessObject* obj;
		YAML::Node node;
	};

	std::vector<ObjectWithSerializedData> objectsWithSerializedData;

	for (std::size_t i{ 0 }; i < mYamlData.size(); i++) {
		auto const guid{ Guid::Parse(mYamlData[i]["guid"].as<std::string>()) };

		if (auto const obj{ dynamic_cast<ManagedAccessObject*>(objectFactory.New(static_cast<Type>(mYamlData[i]["objectType"].as<int>()))) }) {
			obj->SetGuid(guid);
			objectsWithSerializedData.emplace_back(obj, mYamlData[i]["data"]);
		}
	}

	for (auto& [obj, node] : objectsWithSerializedData) {
		obj->DeserializeTextual(node);
		obj->CreateManagedObject();
	}
}

auto Scene::Save() -> void {
	YAML::Node sceneNode;

	auto const serializeObject{
		[&sceneNode](Object const* obj) {
			YAML::Node seNode;
			seNode["objectType"] = static_cast<int>(obj->GetSerializationType());
			seNode["guid"] = obj->GetGuid().ToString();

			YAML::Node dataNode;
			obj->SerializeTextual(dataNode);
			seNode["data"] = dataNode;

			sceneNode.push_back(seNode);
		}
	};

	for (auto const& entity : mEntities) {
		serializeObject(entity.get());

		for (auto const& component : entity->mComponents) {
			serializeObject(component.get());
		}
	}

	mYamlData = sceneNode;
}

auto Scene::Clear() -> void {
	mEntities.clear();
}

auto Scene::CreateEntity() -> Entity* {
	auto const entity{ new Entity{} };
	entity->SetScene(this);
	return mEntities.emplace_back(entity).get();
}


auto Scene::DestroyEntity(Entity const* const entity) -> void {
	if (entity) {
		std::erase_if(mEntities, [entity](auto const& ownedEntity) {
			return ownedEntity->GetGuid() == entity->GetGuid();
		});
	}
}

auto Scene::GetEntities(std::vector<Entity*>& out) const -> std::vector<Entity*>& {
	out.clear();
	for (auto const& entity : mEntities) {
		out.emplace_back(entity.get());
	}
	return out;
}
}
