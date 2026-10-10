#pragma once

#include <memory>
#include <span>
#include <variant>
#include <vector>

#include <imgui.h>

#include "object_ptr.hpp"


namespace sorcery {
class Texture2D;
class Cubemap;


namespace rendering {
class RenderTarget;
}


namespace mage {
class ImGuiTextureReferences {
public:
  using TextureReference = std::variant<
    ObjectPtr<Texture2D>,
    ObjectPtr<Cubemap>,
    std::shared_ptr<rendering::RenderTarget>
  >;

  auto Reference(Texture2D& tex) -> ImTextureID;
  auto Reference(Cubemap& cubemap) -> ImTextureID;
  auto Reference(std::shared_ptr<rendering::RenderTarget> const& rt) -> ImTextureID;

  [[nodiscard]]
  auto GetReferences() const -> std::span<TextureReference const>;

  auto Reset() -> void;

private:
  std::vector<TextureReference> refs_;
};
}
}
