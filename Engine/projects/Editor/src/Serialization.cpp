#include "Serialization.hpp"

#include <Entity.hpp>
#include <Component.hpp>
#include "TransformComponent.hpp"
#include "CameraComponent.hpp"
#include "CubeModelComponent.hpp"
#include "LightComponents.hpp"
#include <ManagedRuntime.hpp>
#include <SceneManager.hpp>

#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <format>
#include <functional>
#include <iostream>
#include <variant>
#include <vector>

#include "BehaviorComponent.hpp"

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


	auto DeserializeScene(YAML::Node const& sceneNode) -> void {
		struct ObjectWithSerializedData {
			Object* obj;
			YAML::Node node;
		};

		std::vector<ObjectWithSerializedData> objectsWithSerializedData;

		for (std::size_t i{ 0 }; i < sceneNode.size(); i++) {
			auto const guid{ Guid::Parse(sceneNode[i]["guid"].as<std::string>())};
			ManagedAccessObject* obj{ nullptr };

			switch (static_cast<Object::Type>(sceneNode[i]["objectType"].as<int>())) {
				case Object::Type::Entity: {
					obj = leopph::SceneManager::GetActiveScene()->CreateEntity();
					obj->CreateManagedObject("leopph", "Entity");
					break;
				}
				case Object::Type::Transform: {
					obj = new TransformComponent{};
					obj->CreateManagedObject("leopph", "Transform");
					break;
				}
				case Object::Type::Camera: {
					obj = new CameraComponent{};
					obj->CreateManagedObject("leopph", "Camera");
					break;
				}
				case Object::Type::Behavior: {
					obj = new BehaviorComponent{};
					break;
				}
				case Object::Type::CubeModel: {
					obj = new CubeModelComponent{};
					obj->CreateManagedObject("leopph", "CubeModel");
					break;
				}
				case Object::Type::DirectionalLight: {
					obj = new DirectionalLightComponent{};
					obj->CreateManagedObject("leopph", "DirectionalLight");
					break;
				}
				default: {

				}
			}

			if (obj) {
				obj->SetGuid(guid);
				objectsWithSerializedData.emplace_back(obj, sceneNode[i]["data"]);
			}
		}

		for (auto& [se, node] : objectsWithSerializedData) {
			se->DeserializeTextual(node);
		}
	}
}
