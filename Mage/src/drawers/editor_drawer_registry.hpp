#pragma once

#include <memory>
#include <unordered_map>

#include "editor_drawer.h"


namespace sorcery::mage {
class EditorDrawerRegistry {
public:
  template<typename T>
  auto Draw(T& obj, bool allow_edit, bool& changed) -> void;

  template<typename T>
  auto DrawAs(T& obj, bool allow_edit, bool& changed) -> void;

  auto RegisterDrawer(std::unique_ptr<EditorDrawerBase> drawer) -> void;

  EditorDrawerRegistry();

private:
  template<typename T>
  auto DrawAs(rttr::type const& type, T& obj, bool allow_edit, bool& changed) -> void;

  std::unordered_map<rttr::type, std::unique_ptr<EditorDrawerBase>> drawers_;
};
}


#include "editor_drawer_registry.inl"
