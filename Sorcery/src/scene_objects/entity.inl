#pragma once

#include <algorithm>
#include <iterator>


namespace sorcery {
template<std::derived_from<Component> T>
auto Entity::GetComponent() const -> T* {
  if constexpr (std::same_as<Component, T>) {
    return components_.empty()
             ? nullptr
             : components_.front().get();
  } else {
    for (auto const& component : components_) {
      if (auto const castPtr{rttr::rttr_cast<T*>(component.get())}) {
        return castPtr;
      }
    }

    return nullptr;
  }
}


template<std::derived_from<Component> T>
auto Entity::GetComponents(std::vector<T*>& out) const -> std::vector<T*>& {
  if constexpr (std::is_same_v<T, Component>) {
    out.clear();
    std::ranges::transform(components_, std::back_inserter(out), [](auto const& component) {
      return component.get();
    });
  } else {
    out.clear();

    for (auto const& component : components_) {
      if (auto const castPtr{rttr::rttr_cast<T*>(component.get())}) {
        out.emplace_back(castPtr);
      }
    }
  }

  return out;
}


template<std::derived_from<Component> T>
auto Entity::GetComponents() const -> std::vector<T*> {
  std::vector<T*> ret;
  GetComponents(ret);
  return ret;
}
}
