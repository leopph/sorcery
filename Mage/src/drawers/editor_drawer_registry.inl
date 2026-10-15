#pragma once

#include "editor_drawer_context.hpp"
#include "../gui_helpers.hpp"


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
    .registry = ObserverPtr{this},
    .tex_refs = tex_refs_
  };

  // If the exact passed type has a drawer, use that one
  if (auto const it = drawers_.find(type); it != drawers_.end()) {
    it->second->Draw(ctx, obj, allow_edit, changed);
    return;
  }

  // Otherwise go up the inheritance chain and look for parent class drawers
  // RTTR returns base classes in a most-base-to-most-derived order.
  // If a parent drawer is found, use that, and fill the unhandled
  // derived class parts with reflection.

  auto const base_classes = type.get_base_classes();

  for (auto it = base_classes.rbegin(); it != base_classes.rend(); ++it) {
    if (auto const drawer_it = drawers_.find(*it); drawer_it != drawers_.end()) {
      drawer_it->second->Draw(ctx, obj, allow_edit, changed);

      DrawReflectedProperties(type, obj, allow_edit, changed,
        [&handled_type = *it](rttr::property const& prop) {
          auto const declaring = prop.get_declaring_type();
          return declaring.is_valid() && declaring.is_derived_from(handled_type);
        });

      return;
    }
  }

  // If we couldn't find any drawers for the entire inheritance chain, default to reflection-based draw.
  DrawReflectedProperties(obj, allow_edit, changed);
}
}
