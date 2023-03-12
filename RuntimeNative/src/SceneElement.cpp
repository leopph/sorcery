#include "SceneElement.hpp"

namespace leopph {
auto SceneElement::Serialize(YAML::Node& node) const -> void {
	node["guid"] = GetGuid().ToString();
	node["type"] = static_cast<int>(GetSerializationType());
}

auto SceneElement::Deserialize(YAML::Node const& node) -> void {
	if (!node["guid"]) {
		throw std::runtime_error{ "Failed to deserialize object from text, because guid data is missing." };
	}

	if (!node["type"]) {
		throw std::runtime_error{ "Failed to deserialize object from text, because type data is missing." };
	}

	if (static_cast<Type>(node["type"].as<int>()) != GetSerializationType()) {
		throw std::runtime_error{ "Failed to deserialize object from text, because type data does not match the object type." };
	}

	SetGuid(Guid::Parse(node["guid"].as<std::string>()));
}
}
