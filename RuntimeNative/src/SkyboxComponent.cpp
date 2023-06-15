#include "SkyboxComponent.hpp"

#include "Renderer.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<leopph::SkyboxComponent>{ "SkyboxComponent" }
    .constructor<>();
}


namespace leopph {
auto SkyboxComponent::GetSerializationType() const -> Type {
  return SerializationType;
}


auto SkyboxComponent::Serialize(YAML::Node& node) const -> void {
  Component::Serialize(node);

  if (mCubemap) {
    node["cubemap"] = mCubemap->GetGuid().ToString();
  }
}


auto SkyboxComponent::Deserialize(YAML::Node const& node) -> void {
  Component::Deserialize(node);

  if (auto const dataNode{ node["cubemap"] }) {
    if (auto const guidStr{ dataNode.as<std::string>("") }; !guidStr.empty()) {
      SetCubemap(dynamic_cast<Cubemap*>(FindObjectByGuid(Guid::Parse(guidStr))));
    }
  }
}


auto SkyboxComponent::GetCubemap() const noexcept -> Cubemap* {
  return mCubemap;
}


auto SkyboxComponent::SetCubemap(Cubemap* cubemap) noexcept -> void {
  renderer::UnregisterSkybox(this);

  mCubemap = cubemap;

  if (mCubemap) {
    renderer::RegisterSkybox(this);
  }
}


SkyboxComponent::~SkyboxComponent() {
  renderer::UnregisterSkybox(this);
}
}
