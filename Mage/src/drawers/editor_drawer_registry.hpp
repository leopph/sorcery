#pragma once

#include <memory>
#include <unordered_map>

#include "editor_drawer.h"
#include "observer_ptr.hpp"


namespace sorcery::mage {
class ImGuiTextureReferences;


class EditorDrawerRegistry {
public:
  template<typename T>
  auto Draw(T& obj, bool allow_edit, bool& changed) -> void;

  template<typename T>
  auto DrawAs(T& obj, bool allow_edit, bool& changed) -> void;

  auto RegisterDrawer(std::unique_ptr<EditorDrawerBase> drawer) -> void;

  explicit EditorDrawerRegistry(ImGuiTextureReferences& tex_refs);

private:
  template<typename T>
  auto DrawAs(rttr::type const& type, T& obj, bool allow_edit, bool& changed) -> void;

  ObserverPtr<ImGuiTextureReferences> tex_refs_;
  std::unordered_map<rttr::type, std::unique_ptr<EditorDrawerBase>> drawers_;
};
}


#include "editor_drawer_registry.inl"
