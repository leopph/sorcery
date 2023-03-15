#pragma once

#include "Component.hpp"
#include "Cubemap.hpp"

namespace leopph {
class SkyboxComponent : public Component {
	Cubemap* mCubemap{ nullptr };

public:
	Type constexpr static SerializationType{ Type::Skybox };
	[[nodiscard]] auto LEOPPHAPI GetSerializationType() const -> Type override;

	auto LEOPPHAPI CreateManagedObject() -> void override;
	auto LEOPPHAPI Serialize(YAML::Node& node) const -> void override;
	auto LEOPPHAPI Deserialize(YAML::Node const& node) -> void override;

	[[nodiscard]] auto LEOPPHAPI GetCubemap() const noexcept -> Cubemap*;
	auto LEOPPHAPI SetCubemap(Cubemap* cubemap) noexcept -> void;
};
}
