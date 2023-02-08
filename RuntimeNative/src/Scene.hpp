#pragma once

#include "Entity.hpp"
#include "Resource.hpp"

#include "YamlInclude.hpp"

#include <string>
#include <memory>
#include <vector>


namespace leopph {
class Scene final : public Resource {
	friend class SceneManager;

	std::string mName;
	std::vector<std::unique_ptr<Entity>> mEntities;
	YAML::Node mYamlData;

	explicit Scene(std::string mName);

public:
	LEOPPHAPI Type constexpr static SerializationType{ Type::Scene };
	[[nodiscard]] auto GetSerializationType() const -> Type override;

	LEOPPHAPI auto SerializeBinary(std::vector<u8>& out) const -> void override;
	LEOPPHAPI auto DeserializeBinary(std::span<u8 const> bytes) -> BinaryDeserializationResult override;

	LEOPPHAPI auto Load(ObjectFactory const& objectFactory) -> void;
	LEOPPHAPI auto Save() -> void;
	LEOPPHAPI auto Clear() -> void;

	LEOPPHAPI Entity* CreateEntity();
	LEOPPHAPI void DestroyEntity(Entity const* entity);

	LEOPPHAPI auto GetEntities(std::vector<Entity*>& out) const -> std::vector<Entity*>&;
};

template<>
class ObjectInstantiatorFor<Scene> : public ObjectInstantiator {
	LEOPPHAPI [[nodiscard]] auto Instantiate() -> Object* override;
};
}
