#include "Serialization.hpp"

#include <Entity.hpp>
#include <Component.hpp>

#include <format>
#include <vector>

namespace leopph::editor {
	auto SerializeScene() -> YAML::Node {
		YAML::Node sceneNode;

		auto const serializeObject{ [&sceneNode](Object const* obj) {
			YAML::Node seNode;
			seNode["objectType"] = static_cast<int>(obj->GetSerializationType());
			seNode["guid"] = obj->GetGuid().ToString();

			YAML::Node dataNode;
			obj->SerializeTextual(dataNode);
			seNode["data"] = dataNode;

			sceneNode.push_back(seNode);
		} };

		static std::vector<Object*> objects;
		Object::FindObjectsOfType<Entity>(objects);
		std::ranges::for_each(objects, serializeObject);
		Object::FindObjectsOfType<Component>(objects);
		std::ranges::for_each(objects, serializeObject);
		return sceneNode;
	}


	auto DeserializeScene(ObjectFactory const& factory, YAML::Node const& sceneNode) -> void {
		struct ObjectWithSerializedData {
			ManagedAccessObject* obj;
			YAML::Node node;
		};

		std::vector<ObjectWithSerializedData> objectsWithSerializedData;

		for (std::size_t i{ 0 }; i < sceneNode.size(); i++) {
			auto const guid{ Guid::Parse(sceneNode[i]["guid"].as<std::string>())};
			auto obj{ dynamic_cast<ManagedAccessObject*>(factory.New(static_cast<Object::Type>(sceneNode[i]["objectType"].as<int>()))) };

			if (obj) {
				obj->SetGuid(guid);
				objectsWithSerializedData.emplace_back(obj, sceneNode[i]["data"]);
			}
		}

		for (auto& [obj, node] : objectsWithSerializedData) {
			obj->DeserializeTextual(node);
			obj->CreateManagedObject();
		}
	}
}
