#pragma once

namespace sorcery::mage {
template<typename T>
auto EditorDrawer<T>::GetTargetType() const -> rttr::type {
  return rttr::type::get<T>();
}


template<typename T>
auto EditorDrawer<T>::Draw(EditorDrawerContext const& ctx, rttr::instance const obj, bool allow_edit,
                           bool& changed) -> void {
  auto* const typed{obj.try_convert<T>()};

  if (!typed) {
    changed = false;
    return;
  }

  Draw(ctx, *typed, allow_edit, changed);
}
}
