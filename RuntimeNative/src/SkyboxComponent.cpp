#include "SkyboxComponent.hpp"

namespace leopph {
auto SkyboxComponent::GetSerializationType() const -> Type {
	return SerializationType;
}

auto SkyboxComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "Skybox");
}

auto SkyboxComponent::Serialize(YAML::Node& node) const -> void { }
auto SkyboxComponent::Deserialize(YAML::Node const& node) -> void {}

auto SkyboxComponent::GetCubemap() const noexcept -> Cubemap* {
	return mCubemap;
}

auto SkyboxComponent::SetCubemap(Cubemap* cubemap) noexcept -> void {
	mCubemap = cubemap;
}
}
