#include "Serialization.hpp"

#include <Entity.hpp>
#include <Components.hpp>
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

namespace leopph::editor {
	auto SerializeScene() -> YAML::Node {
		YAML::Node sceneNode;
		for (auto const* const se : GetAllSceneElements()) {
			YAML::Node seNode;
			seNode["objectType"] = static_cast<int>(se->GetSerializationType());
			seNode["guid"] = se->GetGuid().ToString();

			YAML::Node dataNode;
			se->Serialize(dataNode);
			seNode["data"] = dataNode;

			sceneNode.push_back(seNode);
		}
		return sceneNode;
	}


	auto DeserializeScene(YAML::Node const& sceneNode) -> void {
		struct SceneElementWithDataNode {
			SceneElement* se;
			YAML::Node node;
		};

		std::vector<SceneElementWithDataNode> sceneElementsWithDataNode;
		for (std::size_t i{ 0 }; i < sceneNode.size(); i++) {
			auto const guid{ Guid::Parse(sceneNode[i]["guid"].as<std::string>())};
			SceneElement* obj{ nullptr };
			switch (static_cast<SceneElement::Type>(sceneNode[i]["objectType"].as<int>())) {
				case SceneElement::Type::Entity: {
					obj = leopph::SceneManager::GetActiveScene()->CreateEntity();
					obj->CreateManagedObject("leopph", "Entity");
					break;
				}
				case SceneElement::Type::Transform: {
					obj = new Transform{};
					obj->CreateManagedObject("leopph", "Transform");
					break;
				}
				case SceneElement::Type::Camera: {
					obj = new Camera{};
					obj->CreateManagedObject("leopph", "Camera");
					break;
				}
				case SceneElement::Type::Behavior: {
					obj = new Behavior{};
					break;
				}
				case SceneElement::Type::CubeModel: {
					obj = new CubeModel{};
					obj->CreateManagedObject("leopph", "CubeModel");
					break;
				}
				case SceneElement::Type::DirectionalLight: {
					obj = new DirectionalLight{};
					obj->CreateManagedObject("leopph", "DirectionalLight");
					break;
				}
				default: {

				}
			}

			if (obj) {
				obj->SetGuid(guid);
				sceneElementsWithDataNode.emplace_back(obj, sceneNode[i]["data"]);
			}
		}

		for (auto& [se, node] : sceneElementsWithDataNode) {
			se->Deserialize(node);
		}
	}
}