#pragma once

#include "Entity.hpp"
#include "Object.hpp"

#include "YamlInclude.hpp"

#include <string>
#include <memory>
#include <vector>
#include <span>


namespace leopph {
class Scene : public Object {
	std::vector<std::unique_ptr<Entity>> mEntities;
	YAML::Node mYamlData;

public:
	LEOPPHAPI static Type const SerializationType;
	LEOPPHAPI auto GetSerializationType() const -> Type override;

	LEOPPHAPI auto CreateEntity() -> Entity&;
	LEOPPHAPI auto DestroyEntity(Entity const& entityToRemove) -> void;

	[[nodiscard]] constexpr decltype(auto) begin(this auto&& self) noexcept {
		return self.mEntities.begin();
	}

	[[nodiscard]] constexpr decltype(auto) end(this auto&& self) noexcept {
		return self.mEntities.end();
	}

	[[nodiscard]] LEOPPHAPI auto GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const>;

	[[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> std::string;
	LEOPPHAPI auto Deserialize(std::span<u8 const> bytes) -> void;

	LEOPPHAPI auto Save() -> void;
	LEOPPHAPI auto Load(ObjectInstantiatorManager const& manager) -> void;

	LEOPPHAPI auto Clear() -> void;
};
}
