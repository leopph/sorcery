#include "imgui_texture_references.hpp"

#include "resources/Cubemap.hpp"
#include "resources/Texture2D.hpp"


namespace sorcery::mage {
auto ImGuiTextureReferences::Reference(Texture2D& tex) -> ImTextureID {
  refs_.emplace_back(MakeObjectPtr(MakeObserver(&tex)));
  return refs_.size() - 1;
}


auto ImGuiTextureReferences::Reference(Cubemap& cubemap) -> ImTextureID {
  refs_.emplace_back(MakeObjectPtr(MakeObserver(&cubemap)));
  return refs_.size() - 1;
}


auto ImGuiTextureReferences::Reference(std::shared_ptr<rendering::RenderTarget> const& rt) -> ImTextureID {
  refs_.emplace_back(rt);
  return refs_.size() - 1;
}


auto ImGuiTextureReferences::GetReferences() const -> std::span<TextureReference const> {
  return refs_;
}


auto ImGuiTextureReferences::Reset() -> void {
  refs_.clear();
}
}
