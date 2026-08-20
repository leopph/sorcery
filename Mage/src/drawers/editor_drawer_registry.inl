#pragma once

#include "editor_drawer_context.hpp"


namespace sorcery::mage {
template<typename T>
auto EditorDrawerRegistry::Draw(T& obj, bool allow_edit, bool& changed) -> void {
  DrawAs(rttr::type::get(obj), obj, allow_edit, changed);
}


template<typename T>
auto EditorDrawerRegistry::DrawAs(T& obj, bool allow_edit, bool& changed) -> void {
  DrawAs(rttr::type::get<T>(), obj, allow_edit, changed);
}


template<typename T>
auto EditorDrawerRegistry::DrawAs(rttr::type const& type, T& obj, bool allow_edit, bool& changed) -> void {
  EditorDrawerContext const ctx{
    .registry = ObserverPtr{this}
  };

  // If the exact passed type has a drawer, use that one
  if (auto const it{drawers_.find(type)}; it != drawers_.end()) {
    it->second->Draw(ctx, obj, allow_edit, changed);
    return;
  }

  // Otherwise go up the inheritance chain and look for parent class drawers
  // RTTR returns base classes in a most-base-to-most-derived order.

  auto const base_classes{type.get_base_classes()};

  for (auto it{base_classes.rbegin()}; it != base_classes.rend(); ++it) {
    if (auto const drawer_it{drawers_.find(*it)}; drawer_it != drawers_.end()) {
      drawer_it->second->Draw(ctx, obj, allow_edit, changed);
      return;
    }
  }

  // If we couldn't find any drawers for the entire inheritance chain, default to reflection-based draw.
  // ReflectionDisplayProperties(obj, allow_edit, changed); TODO
}
}
